// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Amiga.h"
#include "Emulator.h"
#include "Option.h"
// #include "Snapshot.h"
// #include "ADFFile.h"
#include "Media.h"
#include "Chrono.h"
#include <algorithm>

namespace vamiga {

string
Amiga::version()
{
    string result;
    
    result = std::to_string(VER_MAJOR) + "." + std::to_string(VER_MINOR);
    if constexpr (VER_SUBMINOR > 0) result += "." + std::to_string(VER_SUBMINOR);
    if constexpr (VER_BETA > 0) result += 'b' + std::to_string(VER_BETA);
    
    return result;
}

string
Amiga::build()
{
    string db = debugBuild ? " [DEBUG BUILD]" : "";
    
    return version() + db + " (" + __DATE__ + " " + __TIME__ + ")";
}

Amiga::Amiga(class Emulator& ref, isize id) : CoreComponent(ref, id)
{
    /* UPDATE: THE ORDER SHOULD NO LONGER BE IMPORTANT. TODO: CHECK THIS!
     *
     * The order of subcomponents is important here, because some components
     * are dependend on others during initialization. I.e.,
     *
     * - The control ports, the serial Controller, the disk controller, and the
     *   disk drives must preceed the CIAs, because the CIA port values depend
     *   on these devices.
     *
     * - The CIAs must preceed memory, because they determine if the lower
     *   memory banks are overlayed by Rom.
     *
     * - Memory must preceed the CPU, because it contains the CPU reset vector.
     */
    
    subComponents = std::vector<CoreComponent *> {
        
        &host,
        &agnus,
        &audioPort,
        &videoPort,
        &rtc,
        &denise,
        &paula,
        &zorro,
        &controlPort1,
        &controlPort2,
        &serialPort,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
        &hd0,
        &hd1,
        &hd2,
        &hd3,
        &hd0con,
        &hd1con,
        &hd2con,
        &hd3con,
        &ramExpansion,
        &diagBoard,
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
        &logicAnalyzer,
        &remoteManager,
        &retroShell,
        &osDebugger,
        &regressionTester
    };
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying emulator instance\n");
}

void
Amiga::prefix(isize level, const char *component, isize line) const
{
    if (level) {
        
        if (level >= 2) {
            
            if (isRunAheadInstance()) fprintf(stderr, "[Run-ahead] ");
            fprintf(stderr, "%s:%ld", component, line);
        }
        if (level >= 3) {
            
            fprintf(stderr, " [%lld]", agnus.pos.frame);
        }
        if (level >= 4) {
            
            fprintf(stderr, " (%3ld,%3ld)", agnus.pos.v, agnus.pos.h);
        }
        if (level >= 5) {
            
            fprintf(stderr, " %06X ", cpu.getPC0());
            if (agnus.copper.servicing) {
                fprintf(stderr, " [%06X]", agnus.copper.getCopPC0());
            }
            fprintf(stderr, " %2X ", cpu.getIPL());
        }
        if (level >= 6) {
            
            u16 dmacon = agnus.dmacon;
            bool dmaen = dmacon & DMAEN;
            fprintf(stderr, " %c%c%c%c%c%c",
                    (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
                    (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
                    (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
                    (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
                    (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
                    (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');
            
            fprintf(stderr, " %04X %04X", paula.intena, paula.intreq);
        }
        fprintf(stderr, " ");
    }
}

void
Amiga::_willReset(bool hard)
{
    // If a disk change is in progress, finish it
    df0.serviceDiskChangeEvent <SLOT_DC0> ();
    df1.serviceDiskChangeEvent <SLOT_DC1> ();
    df2.serviceDiskChangeEvent <SLOT_DC2> ();
    df3.serviceDiskChangeEvent <SLOT_DC3> ();
}

void
Amiga::_didReset(bool hard)
{
    // Schedule initial events
    scheduleNextSnpEvent();
    
    // Clear all runloop flags
    flags = 0;
    
    // Inform the GUI
    if (hard) msgQueue.put(Msg::RESET);
}

i64
Amiga::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::AMIGA_VIDEO_FORMAT:    return (i64)config.type;
        case Opt::AMIGA_WARP_BOOT:       return (i64)config.warpBoot;
        case Opt::AMIGA_WARP_MODE:       return (i64)config.warpMode;
        case Opt::AMIGA_VSYNC:           return (i64)config.vsync;
        case Opt::AMIGA_SPEED_BOOST:     return (i64)config.speedBoost;
        case Opt::AMIGA_RUN_AHEAD:       return (i64)config.runAhead;
        case Opt::AMIGA_SNAP_AUTO:       return (i64)config.autoSnapshots;
        case Opt::AMIGA_SNAP_DELAY:      return (i64)config.snapshotDelay;
        case Opt::AMIGA_SNAP_COMPRESSOR: return (i64)config.snapshotCompressor;
        case Opt::AMIGA_WS_COMPRESSION:  return (i64)config.compressWorkspaces;

        default:
            fatalError;
    }
}

void
Amiga::checkOption(Opt opt, i64 value)
{
    switch (opt) {
            
        case Opt::AMIGA_VIDEO_FORMAT:
            
            if (!TVEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, TVEnum::keyList());
            }
            return;
            
        case Opt::AMIGA_WARP_BOOT:
            
            return;
            
        case Opt::AMIGA_WARP_MODE:
            
            if (!WarpEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, WarpEnum::keyList());
            }
            return;
            
        case Opt::AMIGA_VSYNC:
            
            return;
            
        case Opt::AMIGA_SPEED_BOOST:
            
            if (value < 50 || value > 200) {
                throw CoreError(Fault::OPT_INV_ARG, "50...200");
            }
            return;
            
        case Opt::AMIGA_RUN_AHEAD:
            
            if (value < -7 || value > 7) {
                throw CoreError(Fault::OPT_INV_ARG, "-7...7");
            }
            return;
            
        case Opt::AMIGA_SNAP_AUTO:
            
            return;
            
        case Opt::AMIGA_SNAP_DELAY:
            
            if (value < 10 || value > 3600) {
                throw CoreError(Fault::OPT_INV_ARG, "10...3600");
            }
            return;
            
        case Opt::AMIGA_SNAP_COMPRESSOR:
            
            if (!CompressorEnum::isValid(value)) {
                throw CoreError(Fault::OPT_INV_ARG, CompressorEnum::keyList());
            }
            return;
            
        case Opt::AMIGA_WS_COMPRESSION:

            return;
            
        default:
            throw CoreError(Fault::OPT_UNSUPPORTED);
    }
}

void
Amiga::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::AMIGA_VIDEO_FORMAT:
            
            if (TV(value) != config.type) {
                
                config.type = TV(value);
                agnus.setVideoFormat(config.type);
            }
            return;
            
        case Opt::AMIGA_WARP_BOOT:
            
            config.warpBoot = isize(value);
            return;
            
        case Opt::AMIGA_WARP_MODE:
            
            config.warpMode = Warp(value);
            return;
            
        case Opt::AMIGA_VSYNC:
            
            config.vsync = bool(value);
            return;
            
        case Opt::AMIGA_SPEED_BOOST:
            
            config.speedBoost = isize(value);
            return;
            
        case Opt::AMIGA_RUN_AHEAD:
            
            config.runAhead = isize(value);
            return;
            
        case Opt::AMIGA_SNAP_AUTO:
            
            config.autoSnapshots = bool(value);
            scheduleNextSnpEvent();
            return;
            
        case Opt::AMIGA_SNAP_DELAY:
            
            config.snapshotDelay = isize(value);
            scheduleNextSnpEvent();
            return;
            
        case Opt::AMIGA_SNAP_COMPRESSOR:
            
            config.snapshotCompressor = Compressor(value);
            return;

        case Opt::AMIGA_WS_COMPRESSION:
            
            config.compressWorkspaces = bool(value);
            return;

        default:
            fatalError;
    }
}

void
Amiga::loadWorkspace(const fs::path &path)
{
    std::stringstream ss;
    
    // Set the search path to the workspace directoy
    host.setSearchPath(path);
    
    // Assemble the setup script
    try {
        
        // Power off the amiga to make it configurable
        ss << "\n";
        ss << "try workspace init";
        ss << "\n";
        
        // Read the config script
        if (fs::exists(path / "config.retrosh")) {
            
            Script script(path / "config.retrosh");
            script.writeToStream(ss);
        }
        
        // Power on the Amiga with the new configuration
        ss << "\n";
        ss << "try workspace activate";
        
    } catch (CoreError &exc) {
        
        printf("Error: %s\n", exc.what());
        throw;
    }

    // Execute the setup script
    retroShell.asyncExecScript(ss);
}

void
Amiga::saveWorkspace(const fs::path &path)
{
    std::stringstream ss, df, hd;

    auto exportADF = [&](FloppyDrive& drive, string name) {
        
        if (!drive.hasDisk()) return;
        
        string file = name + (config.compressWorkspaces ? ".adz" : ".adf");

        try {
            
            if (config.compressWorkspaces) {
                
                ADZFile(ADFFile(drive)).writeToFile(path / file);
            } else {
                ADFFile(drive).writeToFile(path / file);
            }
            drive.markDiskAsUnmodified();
            
            df << "try " << name << " insert " << file << "\n";
            df << "try " << name << (drive.hasProtectedDisk() ? " protect\n" : " unprotect\n");
            
        } catch (...) { }
    };
    
    auto exportHDF = [&](HardDrive& drive, string name) {
        
        if (!drive.hasDisk()) return;
        
        string file = name + (config.compressWorkspaces ? ".hdz" : ".hdf");
        
        try {

            if (config.compressWorkspaces) {
                HDZFile(HDFFile(drive)).writeToFile(path / file);
            } else {
                HDFFile(drive).writeToFile(path / file);
            }
            drive.markDiskAsUnmodified();
            
            hd << "try " << name << " attach " << file << "\n";
            hd << "try " << name << (drive.hasProtectedDisk() ? " protect\n" : " unprotect\n");
            
        } catch (...) { }
    };

    // If a file with the specified name exists, delete it
    if (fs::exists(path) && !fs::is_directory(path)) fs::remove(path);
    
    // Create the directory if necessary
    if (!fs::exists(path)) fs::create_directories(path);
        
    // Remove old files
    for (const auto& entry : fs::directory_iterator(path)) fs::remove_all(entry.path());
        
    // Prepare the config script
    auto now = std::time(nullptr);
    ss << "# Workspace setup (" << std::put_time(std::localtime(&now), "%c") << ")\n";
    ss << "# Generated with vAmiga " << Amiga::build() << "\n";
    ss << "\n";

    // Dump the current config
    exportConfig(ss, false, { Class::Host} );

    // Export ROMs
    ss << "\n# ROMs\n\n";
    if (mem.hasRom()) { mem.saveRom(path / "rom.bin"); ss << "try mem load rom rom.bin\n"; }
    if (mem.hasWom()) { mem.saveWom(path / "wom.bin"); ss << "try mem load wom wom.bin\n"; }
    if (mem.hasExt()) { mem.saveExt(path / "ext.bin"); ss << "try mem load ext ext.bin\n"; }

    // Export floppy disks
    exportADF(df0, "df0");
    exportADF(df1, "df1");
    exportADF(df2, "df2");
    exportADF(df3, "df3");
    
    if (!df.str().empty()) {
        ss << "\n# Floppy disks\n\n";
        ss << df.str();
    }
    
    // Export hard disks
    exportHDF(hd0, "hd0");
    exportHDF(hd1, "hd1");
    exportHDF(hd2, "hd2");
    exportHDF(hd3, "hd3");
    
    if (!hd.str().empty()) {
        ss << "\n# Hard drives\n\n";
        ss << hd.str();
    }

    // Write the script into the workspace bundle
    std::ofstream file(path / "config.retrosh");
    file << ss.str();
    
    // Inform the GUI
    msgQueue.put(Msg::WORKSPACE_SAVED);
}

void
Amiga::initWorkspace()
{
    /* This function is called at the beginning of a workspace script */
    
    // Power off the Amiga to make it configurable
    powerOff();
}

void
Amiga::activateWorkspace()
{
    /* This function is called at the end of a workspace script */
     
    // Power on the Amiga
    powerOn();
    
    // Inform the GUI
    msgQueue.put(Msg::WORKSPACE_LOADED);
}

void
Amiga::revertToFactorySettings()
{
    // Switch the emulator off
    powerOff();

    // Revert to the initial state
    initialize();
}

i64
Amiga::get(Opt opt, isize objid) const
{
    debug(CNF_DEBUG, "get(%s, %ld)\n", OptEnum::key(opt), objid);

    auto target = routeOption(opt, objid);
    if (target == nullptr) throw CoreError(Fault::OPT_INV_ID);
    return target->getOption(opt);
}

void
Amiga::check(Opt opt, i64 value, const std::vector<isize> objids)
{
    if (objids.empty()) {

        for (isize objid = 0;; objid++) {

            auto target = routeOption(opt, objid);
            if (target == nullptr) break;

            debug(CNF_DEBUG, "check(%s, %lld, %ld)\n", OptEnum::key(opt), value, objid);
            target->checkOption(opt, value);
        }
    }
    for (auto &objid : objids) {

        debug(CNF_DEBUG, "check(%s, %lld, %ld)\n", OptEnum::key(opt), value, objid);

        auto target = routeOption(opt, objid);
        if (target == nullptr) throw CoreError(Fault::OPT_INV_ID);

        target->checkOption(opt, value);
    }
}

void
Amiga::set(Opt opt, i64 value, const std::vector<isize> objids)
{
    if (objids.empty()) {

        for (isize objid = 0;; objid++) {

            auto target = routeOption(opt, objid);
            if (target == nullptr) break;

            debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptEnum::key(opt), value, objid);
            target->setOption(opt, value);
        }
    }
    for (auto &objid : objids) {

        debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptEnum::key(opt), value, objid);

        auto target = routeOption(opt, objid);
        if (target == nullptr) throw CoreError(Fault::OPT_INV_ID);

        target->setOption(opt, value);
    }
}

void
Amiga::set(Opt opt, const string &value, const std::vector<isize> objids)
{
    set(opt, OptionParser::parse(opt, value), objids);
}

void
Amiga::set(const string &opt, const string &value, const std::vector<isize> objids)
{
    set(Opt(util::parseEnum<OptEnum>(opt)), value, objids);
}

void
Amiga::set(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);
    
    switch(scheme) {
            
        case ConfigScheme::A1000_OCS_1MB:
            
            set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            set(Opt::AGNUS_REVISION, (i64)AgnusRevision::OCS_OLD);
            set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            set(Opt::MEM_CHIP_RAM, 512);
            set(Opt::MEM_SLOW_RAM, 512);
            break;
            
        case ConfigScheme::A500_OCS_1MB:
            
            set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            set(Opt::AGNUS_REVISION, (i64)AgnusRevision::OCS);
            set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            set(Opt::MEM_CHIP_RAM, 512);
            set(Opt::MEM_SLOW_RAM, 512);
            break;
            
        case ConfigScheme::A500_ECS_1MB:
            
            set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            set(Opt::AGNUS_REVISION, (i64)AgnusRevision::ECS_1MB);
            set(Opt::DENISE_REVISION, (i64)DeniseRev::OCS);
            set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            set(Opt::MEM_CHIP_RAM, 512);
            set(Opt::MEM_SLOW_RAM, 512);
            break;
            
        case ConfigScheme::A500_PLUS_1MB:
            
            set(Opt::CPU_REVISION, (i64)CPURev::CPU_68000);
            set(Opt::AGNUS_REVISION, (i64)AgnusRevision::ECS_2MB);
            set(Opt::DENISE_REVISION, (i64)DeniseRev::ECS);
            set(Opt::AMIGA_VIDEO_FORMAT, (i64)TV::PAL);
            set(Opt::MEM_CHIP_RAM, 512);
            set(Opt::MEM_SLOW_RAM, 512);
            break;
            
        default:
            fatalError;
    }
}

Configurable *
Amiga::routeOption(Opt opt, isize objid)
{
    return CoreComponent::routeOption(opt, objid);
}

const Configurable *
Amiga::routeOption(Opt opt, isize objid) const
{
    auto result = const_cast<Amiga *>(this)->routeOption(opt, objid);
    return const_cast<const Configurable *>(result);
}

u64
Amiga::getAutoInspectionMask() const
{
    return agnus.data[SLOT_INS];
}

void 
Amiga::setAutoInspectionMask(u64 mask)
{
    if (mask) {

        agnus.data[SLOT_INS] = mask;
        agnus.serviceINSEvent();

    } else {

        agnus.data[SLOT_INS] = 0;
        agnus.cancel<SLOT_INS>();
    }
}

double
Amiga::nativeRefreshRate() const
{
    switch (config.type) {

        case TV::PAL:   return 50.0;
        case TV::NTSC:  return 60.0;

        default:
            fatalError;
    }
}

i64
Amiga::nativeMasterClockFrequency() const
{
    switch (config.type) {

        case TV::PAL:   return PAL::CLK_FREQUENCY;
        case TV::NTSC:  return NTSC::CLK_FREQUENCY;

        default:
            fatalError;
    }
}

double
Amiga::refreshRate() const
{
    if (config.vsync) {

        return double(host.getOption(Opt::HOST_REFRESH_RATE));

    } else {

        auto boost = config.speedBoost ? config.speedBoost : 100;
        return nativeRefreshRate() * boost / 100.0;
    }
}

i64
Amiga::masterClockFrequency() const
{
    auto boost = config.speedBoost ? config.speedBoost : 100;
    return nativeMasterClockFrequency() * boost / 100;
}

void
Amiga::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::State) {

        os << tab("Refresh rate");
        os << dec(isize(refreshRate())) << " Fps" << std::endl;
        os << tab("Native master clock");
        os << flt(nativeMasterClockFrequency() / float(1000000.0)) << " MHz" << std::endl;
        os << tab("Emulated master clock");
        os << flt(masterClockFrequency() / float(1000000.0)) << " MHz" << std::endl;
        os << tab("Native refresh rate");
        os << flt(nativeRefreshRate()) << " Fps" << std::endl;
        os << tab("Emulated refresh rate");
        os << flt(refreshRate()) << " Fps" << std::endl;
        os << std::endl;

        os << tab("Frame");
        os << dec(agnus.pos.frame) << std::endl;
        os << tab("CPU progress");
        os << dec(cpu.getMasterClock()) << " Master cycles (";
        os << dec(cpu.getCpuClock()) << " CPU cycles)" << std::endl;
        os << tab("Agnus progress");
        os << dec(agnus.clock) << " Master cycles (";
        os << dec(AS_DMA_CYCLES(agnus.clock)) << " DMA cycles)" << std::endl;
        os << tab("CIA A progress");
        os << dec(ciaA.getClock()) << " Master cycles (";
        os << dec(AS_CIA_CYCLES(ciaA.getClock())) << " CIA cycles)" << std::endl;
        os << tab("CIA B progress");
        os << dec(ciaB.getClock()) << " Master cycles (";
        os << dec(AS_CIA_CYCLES(ciaA.getClock())) << " CIA cycles)" << std::endl;
        os << std::endl;
    }

    if (category == Category::Current) {

        auto dmacon = agnus.dmacon;
        bool dmaen = dmacon & DMAEN;
        auto intreq = paula.intreq;
        auto intena = (paula.intena & 0x8000) ? paula.intena : 0;
        auto fc = cpu.readFC();
        char empty = '.';

        char sr[32];
        (void)cpu.disassembleSR(sr);

        os << std::setfill('0');
        os << "   DMACON  INTREQ / INTENA  STATUS REGISTER  IPL FCP" << std::endl;

        os << "   ";
        os << ((dmacon & BPLEN) ? (dmaen ? 'B' : 'b') : empty);
        os << ((dmacon & COPEN) ? (dmaen ? 'C' : 'c') : empty);
        os << ((dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : empty);
        os << ((dmacon & SPREN) ? (dmaen ? 'S' : 's') : empty);
        os << ((dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : empty);
        os << ((dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : empty);

        os << "  ";
        os << ((intena & 0x4000) ? '1' : '0');
        os << ((intreq & 0x2000) ? ((intena & 0x2000) ? 'E' : 'e') : empty);
        os << ((intreq & 0x1000) ? ((intena & 0x1000) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0800) ? ((intena & 0x0800) ? 'R' : 'r') : empty);
        os << ((intreq & 0x0400) ? ((intena & 0x0400) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0200) ? ((intena & 0x0200) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0100) ? ((intena & 0x0100) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0080) ? ((intena & 0x0080) ? 'A' : 'a') : empty);
        os << ((intreq & 0x0040) ? ((intena & 0x0040) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0020) ? ((intena & 0x0020) ? 'V' : 'v') : empty);
        os << ((intreq & 0x0010) ? ((intena & 0x0010) ? 'C' : 'c') : empty);
        os << ((intreq & 0x0008) ? ((intena & 0x0008) ? 'P' : 'p') : empty);
        os << ((intreq & 0x0004) ? ((intena & 0x0004) ? 'S' : 's') : empty);
        os << ((intreq & 0x0002) ? ((intena & 0x0002) ? 'D' : 'd') : empty);
        os << ((intreq & 0x0001) ? ((intena & 0x0001) ? 'T' : 't') : empty);

        os << "  ";
        os << sr;

        os << " [";
        os << std::right << std::setw(1) << std::dec << isize(cpu.getIPL()) << "]";

        os << " ";
        os << ((fc & 0b100) ? '1' : '0');
        os << ((fc & 0b010) ? '1' : '0');
        os << ((fc & 0b001) ? '1' : '0');
        os << std::endl;
    }
    
    if (category == Category::Trace) {
        
        os << "\n";
        cpu.dumpLogBuffer(os, 8);
        os << "\n";
        dump(Category::Current, os);
        os << "\n";
        cpu.disassembleRange(os, cpu.getPC0(), 8);
        os << "\n";
    }
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "_powerOn\n");

    hardReset();
    msgQueue.put(Msg::POWER, 1);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff\n");

    hardReset();
    msgQueue.put(Msg::POWER, 0);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run\n");

    msgQueue.put(Msg::RUN);
}

void
Amiga::_pause()
{
    debug(RUN_DEBUG, "_pause\n");

    remoteManager.gdbServer.breakpointReached();
    msgQueue.put(Msg::PAUSE);
}

void
Amiga::_halt()
{
    debug(RUN_DEBUG, "_halt\n");

    msgQueue.put(Msg::SHUTDOWN);
}

void
Amiga::_warpOn()
{
    debug(RUN_DEBUG, "_warpOn\n");

    msgQueue.put(Msg::WARP, 1);
}

void
Amiga::_warpOff()
{
    debug(RUN_DEBUG, "_warpOff\n");

    msgQueue.put(Msg::WARP, 0);
}

void
Amiga::_trackOn()
{
    debug(RUN_DEBUG, "_trackOn\n");

    msgQueue.put(Msg::TRACK, 1);
}

void
Amiga::_trackOff()
{
    debug(RUN_DEBUG, "_trackOff\n");

    msgQueue.put(Msg::TRACK, 0);
}

void 
Amiga::update(CmdQueue &queue)
{
    Command cmd;
    bool cmdConfig = false;

    auto dfn = [&]() -> FloppyDrive& { return *df[cmd.value]; };

    // Process all commands
    while (queue.poll(cmd)) {

        switch (cmd.type) {

            case Cmd::CONFIG:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { cmd.config.id });
                break;

            case Cmd::CONFIG_ALL:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { });
                break;

            case Cmd::ALARM_ABS:
            case Cmd::ALARM_REL:
            case Cmd::INSPECTION_TARGET:
            case Cmd::HARD_RESET:
            case Cmd::SOFT_RESET:
            case Cmd::POWER_ON:
            case Cmd::POWER_OFF:
            case Cmd::RUN:
            case Cmd::PAUSE:
            case Cmd::WARP_ON:
            case Cmd::WARP_OFF:
            case Cmd::HALT:

                processCommand(cmd);
                break;

            case Cmd::GUARD_SET_AT:
            case Cmd::GUARD_MOVE_NR:
            case Cmd::GUARD_IGNORE_NR:
            case Cmd::GUARD_REMOVE_NR:
            case Cmd::GUARD_REMOVE_AT:
            case Cmd::GUARD_REMOVE_ALL:
            case Cmd::GUARD_ENABLE_NR:
            case Cmd::GUARD_ENABLE_AT:
            case Cmd::GUARD_ENABLE_ALL:
            case Cmd::GUARD_DISABLE_NR:
            case Cmd::GUARD_DISABLE_AT:
            case Cmd::GUARD_DISABLE_ALL:

                cpu.processCommand(cmd);
                break;

            case Cmd::KEY_PRESS:
            case Cmd::KEY_RELEASE:
            case Cmd::KEY_RELEASE_ALL:
            case Cmd::KEY_TOGGLE:

                keyboard.processCommand(cmd);
                break;

            case Cmd::MOUSE_MOVE_ABS:
            case Cmd::MOUSE_MOVE_REL:
            {
                auto &port = cmd.coord.port ? controlPort2 : controlPort1;
                port.processCommand(cmd); break;
                break;
            }
            case Cmd::MOUSE_BUTTON:
            case Cmd::JOY_EVENT:
            {
                auto &port = cmd.action.port ? controlPort2 : controlPort1;
                port.processCommand(cmd); break;
                break;
            }
            case Cmd::DSK_TOGGLE_WP:
            case Cmd::DSK_MODIFIED:
            case Cmd::DSK_UNMODIFIED:

                dfn().processCommand(cmd);
                break;

                
            case Cmd::RSH_EXECUTE:

                retroShell.exec();
                break;

            case Cmd::FOCUS:

                cmd.value ? focus() : unfocus();
                break;

            default:
                fatal("Unhandled command: %s\n", CmdTypeEnum::key(cmd.type));
        }
    }

    // Inform the GUI about a changed machine configuration
    if (cmdConfig) { msgQueue.put(Msg::CONFIG, isize(cmd.type)); }

    // Inform the GUI about new RetroShell content
    if (retroShell.isDirty) { retroShell.isDirty = false; msgQueue.put(Msg::RSH_UPDATE); }
}

void
Amiga::computeFrame()
{
    while (1) {

        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {

            enum Action { cont, pause, leave } action = cont;
                        
            // Are we requested to synchronize the thread?
            if (flags & RL::SYNC_THREAD) {

                action = leave;
            }

            // Did we reach a soft breakpoint?
            if (flags & RL::SOFTSTOP_REACHED) {

                msgQueue.put(Msg::STEP);
                action = pause;
            }

            // Shall we stop at the end of the current line?
            if (flags & RL::EOL_REACHED) {

                msgQueue.put(Msg::EOL_REACHED);
                action = pause;
            }
  
            // Shall we stop at the end of the current frame?
            if (flags & RL::EOF_REACHED) {

                msgQueue.put(Msg::EOF_REACHED);
                action = pause;
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {

                auto addr = cpu.debugger.breakpoints.hit->addr;
                msgQueue.put(Msg::BREAKPOINT_REACHED, CpuMsg { addr, 0 });
                action = pause;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {

                auto addr = cpu.debugger.watchpoints.hit->addr;
                msgQueue.put(Msg::WATCHPOINT_REACHED, CpuMsg { addr, 0 });
                action = pause;
            }

            // Did we reach a catchpoint?
            if (flags & RL::CATCHPOINT_REACHED) {

                auto vector = u8(cpu.debugger.catchpoints.hit->addr);
                msgQueue.put(Msg::CATCHPOINT_REACHED, CpuMsg { cpu.getPC0(), vector });
                action = pause;
            }

            // Did we reach a software trap?
            if (flags & RL::SWTRAP_REACHED) {

                msgQueue.put(Msg::SWTRAP_REACHED, CpuMsg { cpu.getPC0(), 0 });
                action = pause;
            }

            // Did we reach a beam trap?
            if (flags & RL::BEAMTRAP_REACHED) {

                msgQueue.put(Msg::BEAMTRAP_REACHED, CpuMsg { 0, 0 });
                action = pause;
            }

            // Did we reach a Copper breakpoint?
            if (flags & RL::COPPERBP_REACHED) {

                auto addr = u8(agnus.copper.debugger.breakpoints.hit()->addr);
                msgQueue.put(Msg::COPPERBP_REACHED, CpuMsg { addr, 0 });
                action = pause;
            }

            // Did we reach a Copper watchpoint?
            if (flags & RL::COPPERWP_REACHED) {

                auto addr = u8(agnus.copper.debugger.watchpoints.hit()->addr);
                msgQueue.put(Msg::COPPERWP_REACHED, CpuMsg { addr, 0 });
                action = pause;
            }

            // Are we requested to pause the emulator?
            if (flags & RL::STOP) {

                action = pause;
            }
            
            flags = 0;
            
            if (action == pause) { throw StateChangeException((long)ExecState::PAUSED); }
            if (action == leave) { break; }
        }
    }
}

void
Amiga::fastForward(isize frames)
{
    auto target = agnus.pos.frame + frames;

    // Execute until the target frame has been reached
    while (agnus.pos.frame < target) computeFrame();
}

void
Amiga::cacheInfo(AmigaInfo &result) const
{
    {   SYNCHRONIZED

        info.cpuClock = cpu.getMasterClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaA.getClock();
        info.ciaBClock = ciaB.getClock();
        info.frame = agnus.pos.frame;
        info.vpos = agnus.pos.v;
        info.hpos = agnus.pos.h;
    }
}

void
Amiga::setFlag(u32 flag)
{
    assert(isEmulatorThread());
    flags |= flag;
}

void
Amiga::clearFlag(u32 flag)
{
    assert(isEmulatorThread());
    flags &= ~flag;
}

MediaFile *
Amiga::takeSnapshot()
{
    Snapshot *result;
    
    // Take the snapshot
   result = new Snapshot(*this);
    
    // Compress the snapshot if requested
    result->compress(config.snapshotCompressor);
    
    return result;
}

void
Amiga::serviceSnpEvent(EventID eventId)
{
    // Check for the main instance (ignore the run-ahead instance)
    if (objid == 0) {

        // Take snapshot and hand it over to GUI
        msgQueue.put(Msg::SNAPSHOT_TAKEN, SnapshotMsg { .snapshot = new Snapshot(*this) } );
    }

    // Schedule the next event
    scheduleNextSnpEvent();
}

void
Amiga::scheduleNextSnpEvent()
{
    auto snapshots = emulator.get(Opt::AMIGA_SNAP_AUTO);
    auto delay = emulator.get(Opt::AMIGA_SNAP_DELAY);

    if (snapshots) {
        agnus.scheduleRel<SLOT_SNP>(SEC(double(delay)), SNP_TAKE);
    } else {
        agnus.cancel<SLOT_SNP>();
    }
}

void
Amiga::loadSnapshot(const fs::path &path)
{
    loadSnapshot(Snapshot(path));
}

void
Amiga::loadSnapshot(const MediaFile &file)
{
    const Snapshot &snapshot = dynamic_cast<const Snapshot &>(file);
    loadSnapshot(snapshot);
}

void
Amiga::loadSnapshot(const Snapshot &snap)
{
    // Make a copy so we can modify the snapshot
    Snapshot snapshot(snap);
    
    // Uncompress the snapshot
    snapshot.uncompress();
    
    // Restore the saved state (may throw)
    load(snapshot.getData());
        
    // Inform the GUI
    msgQueue.put(Msg::SNAPSHOT_RESTORED);
    msgQueue.put(Msg::VIDEO_FORMAT, agnus.isPAL() ? (i64)TV::PAL : (i64)TV::NTSC);
}

void
Amiga::saveSnapshot(const fs::path &path)
{
    Snapshot(*this, config.snapshotCompressor).writeToFile(path);
}

void
Amiga::processCommand(const Command &cmd)
{
    switch (cmd.type) {

        case Cmd::ALARM_ABS:

            setAlarmAbs(cmd.alarm.cycle, cmd.alarm.value);
            break;

        case Cmd::ALARM_REL:

            setAlarmRel(cmd.alarm.cycle, cmd.alarm.value);
            break;

        case Cmd::INSPECTION_TARGET:

            setAutoInspectionMask(cmd.value);
            break;

        case Cmd::HARD_RESET:
            
            emulator.hardReset();
            break;
            
        case Cmd::SOFT_RESET:
            
            emulator.softReset();
            break;
            
        case Cmd::POWER_ON:
            
            emulator.powerOn();
            break;
            
        case Cmd::POWER_OFF:
            
            emulator.powerOff();
            break;
            
        case Cmd::RUN:
            
            emulator.run();
            break;
            
        case Cmd::PAUSE:
            
            emulator.pause();
            break;
            
        case Cmd::WARP_ON:
            
            if (cmd.value == 0) {
                throw std::runtime_error("Source 0 is reserved for implementing config.warpMode.");
            }
            emulator.warpOn(cmd.value);
            break;
            
        case Cmd::WARP_OFF:

            if (cmd.value == 0) {
                throw std::runtime_error("Source 0 is reserved for implementing config.warpMode.");
            }
            emulator.warpOff(cmd.value);
            break;

        case Cmd::HALT:

            emulator.halt();
            break;

        default:
            fatalError;
    }
}

void
Amiga::eolHandler()
{

}

void
Amiga::setAlarmAbs(Cycle trigger, i64 payload)
{
    alarms.push_back(Alarm { trigger, payload });
    scheduleNextAlarm();
}

void
Amiga::setAlarmRel(Cycle trigger, i64 payload)
{
    alarms.push_back(Alarm { agnus.clock + trigger, payload });
    scheduleNextAlarm();
}

void
Amiga::serviceAlarmEvent()
{
    for (auto it = alarms.begin(); it != alarms.end(); ) {

        if (it->trigger <= agnus.clock) {
            msgQueue.put(Msg::ALARM, it->payload);
            it = alarms.erase(it);
        } else {
            it++;
        }
    }
    scheduleNextAlarm();
}

void
Amiga::scheduleNextAlarm()
{
    Cycle trigger = INT64_MAX;

    agnus.cancel<SLOT_ALA>();

    for(Alarm alarm : alarms) {

        if (alarm.trigger < trigger) {
            agnus.scheduleAbs<SLOT_ALA>(alarm.trigger, ALA_TRIGGER);
            trigger = alarm.trigger;
        }
    }
}

u32
Amiga::random()
{
    return random(u32(agnus.clock));
}

u32
Amiga::random(u32 seed)
{
    // Parameters for the Linear Congruential Generator (LCG)
    u64 a = 1664525;
    u64 c = 1013904223;
    u64 m = 1LL << 32;

    // Apply the LCG formula
    return u32((a * seed + c) % m);
}

}
