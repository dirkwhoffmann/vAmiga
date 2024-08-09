// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include "Emulator.h"
#include "Option.h"
#include "Snapshot.h"
#include "ADFFile.h"
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
        &remoteManager,
        &retroShell,
        &debugger,
        &osDebugger,
        &regressionTester
    };
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying emulator instance\n");
}

void
Amiga::prefix(isize level, isize line) const
{
    if (level) {

        if (level >= 2) {

            if (objid == 1) fprintf(stderr, "[Run-ahead] ");
            fprintf(stderr, "%s:%ld", objectName(), line);
        }
        if (level >= 3) {

            fprintf(stderr, " [%lld] (%3ld,%3ld)",
                    agnus.pos.frame, agnus.pos.v, agnus.pos.h);
        }
        if (level >= 4) {

            fprintf(stderr, " %06X ", cpu.getPC0());
            if (agnus.copper.servicing) {
                fprintf(stderr, " [%06X]", agnus.copper.getCopPC0());
            }
            fprintf(stderr, " %2X ", cpu.getIPL());
        }
        if (level >= 5) {

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
    if (hard) msgQueue.put(MSG_RESET);
}

i64
Amiga::getOption(Option option) const
{
    switch (option) {

        case OPT_AMIGA_VIDEO_FORMAT:    return config.type;
        case OPT_AMIGA_WARP_BOOT:       return config.warpBoot;
        case OPT_AMIGA_WARP_MODE:       return config.warpMode;
        case OPT_AMIGA_VSYNC:           return config.vsync;
        case OPT_AMIGA_SPEED_BOOST:     return config.timeLapse;
        case OPT_AMIGA_SNAPSHOTS:       return config.snapshots;
        case OPT_AMIGA_SNAPSHOT_DELAY:  return config.snapshotDelay;
        case OPT_AMIGA_RUN_AHEAD:       return config.runAhead;

        default:
            fatalError;
    }
}

void
Amiga::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_AMIGA_VIDEO_FORMAT:

            if (!VideoFormatEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, VideoFormatEnum::keyList());
            }
            return;

        case OPT_AMIGA_WARP_BOOT:

            return;

        case OPT_AMIGA_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, WarpModeEnum::keyList());
            }
            return;

        case OPT_AMIGA_VSYNC:

            return;

        case OPT_AMIGA_SPEED_BOOST:

            if (value < 50 || value > 200) {
                throw Error(ERROR_OPT_INV_ARG, "50...200");
            }
            return;

        case OPT_AMIGA_SNAPSHOTS:

            return;

        case OPT_AMIGA_SNAPSHOT_DELAY:

            if (value < 10 || value > 3600) {
                throw Error(ERROR_OPT_INV_ARG, "10...3600");
            }
            return;

        case OPT_AMIGA_RUN_AHEAD:

            if (value < 0 || value > 12) {
                throw Error(ERROR_OPT_INV_ARG, "0...12");
            }
            return;

        default:
            throw Error(ERROR_OPT_UNSUPPORTED);
    }
}

void
Amiga::setOption(Option option, i64 value)
{
    switch (option) {

        case OPT_AMIGA_VIDEO_FORMAT:

            if (value != config.type) {

                config.type = VideoFormat(value);
                agnus.setVideoFormat(config.type);
            }
            return;

        case OPT_AMIGA_WARP_BOOT:

            config.warpBoot = isize(value);
            return;

        case OPT_AMIGA_WARP_MODE:

            config.warpMode = WarpMode(value);
            return;

        case OPT_AMIGA_VSYNC:

            config.vsync = bool(value);
            return;

        case OPT_AMIGA_SPEED_BOOST:

            config.timeLapse = isize(value);
            return;

        case OPT_AMIGA_SNAPSHOTS:

            config.snapshots = bool(value);
            scheduleNextSnpEvent();
            return;

        case OPT_AMIGA_SNAPSHOT_DELAY:

            config.snapshotDelay = isize(value);
            scheduleNextSnpEvent();
            return;

        case OPT_AMIGA_RUN_AHEAD:

            config.runAhead = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Amiga::exportConfig(const fs::path &path, bool diff) const
{
    auto fs = std::ofstream(path, std::ofstream::binary);

    if (!fs.is_open()) {
        throw Error(ERROR_FILE_CANT_WRITE);
    }

    exportConfig(fs, diff);
}

void
Amiga::exportConfig(std::ostream &stream, bool diff) const
{
    stream << "# vAmiga " << Amiga::build() << "\n";
    stream << "\n";
    stream << "amiga power off\n";
    stream << "\n";
    CoreComponent::exportConfig(stream, diff);
    stream << "amiga power on\n";
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
Amiga::overrideOption(Option option, i64 value)
{
    static std::map<Option,i64> overrides = OVERRIDES;

    if (overrides.find(option) != overrides.end()) {

        msg("Overriding option: %s = %lld\n", OptionEnum::key(option), value);
        return overrides[option];
    }

    return value;
}

i64
Amiga::get(Option opt, isize objid) const
{
    debug(CNF_DEBUG, "get(%s, %ld)\n", OptionEnum::key(opt), objid);

    auto target = routeOption(opt, objid);
    if (target == nullptr) throw Error(ERROR_OPT_INV_ID);
    return target->getOption(opt);
}

void
Amiga::check(Option opt, i64 value, const std::vector<isize> objids)
{
    value = overrideOption(opt, value);

    if (objids.empty()) {

        for (isize objid = 0;; objid++) {

            auto target = routeOption(opt, objid);
            if (target == nullptr) break;

            debug(CNF_DEBUG, "check(%s, %lld, %ld)\n", OptionEnum::key(opt), value, objid);
            target->checkOption(opt, value);
        }
    }
    for (auto &objid : objids) {

        debug(CNF_DEBUG, "check(%s, %lld, %ld)\n", OptionEnum::key(opt), value, objid);

        auto target = routeOption(opt, objid);
        if (target == nullptr) throw Error(ERROR_OPT_INV_ID);

        target->checkOption(opt, value);
    }
}

void
Amiga::set(Option opt, i64 value, const std::vector<isize> objids)
{
    value = overrideOption(opt, value);

    if (objids.empty()) {

        for (isize objid = 0;; objid++) {

            auto target = routeOption(opt, objid);
            if (target == nullptr) break;

            debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptionEnum::key(opt), value, objid);
            target->setOption(opt, value);
        }
    }
    for (auto &objid : objids) {

        debug(CNF_DEBUG, "set(%s, %lld, %ld)\n", OptionEnum::key(opt), value, objid);

        auto target = routeOption(opt, objid);
        if (target == nullptr) throw Error(ERROR_OPT_INV_ID);

        target->setOption(opt, value);
    }
}

void
Amiga::set(Option opt, const string &value, const std::vector<isize> objids)
{
    set(opt, OptionParser::parse(opt, value), objids);
}

void
Amiga::set(const string &opt, const string &value, const std::vector<isize> objids)
{
    set(Option(util::parseEnum<OptionEnum>(opt)), value, objids);
}

void
Amiga::set(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);

    {   SUSPENDED

        switch(scheme) {

            case CONFIG_A1000_OCS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_OCS_OLD);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_MEM_CHIP_RAM, 512);
                set(OPT_MEM_SLOW_RAM, 512);
                break;

            case CONFIG_A500_OCS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_OCS);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_MEM_CHIP_RAM, 512);
                set(OPT_MEM_SLOW_RAM, 512);
                break;

            case CONFIG_A500_ECS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
                set(OPT_DENISE_REVISION, DENISE_OCS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_MEM_CHIP_RAM, 512);
                set(OPT_MEM_SLOW_RAM, 512);
                break;

            case CONFIG_A500_PLUS_1MB:

                set(OPT_CPU_REVISION, CPU_68000);
                set(OPT_AGNUS_REVISION, AGNUS_ECS_2MB);
                set(OPT_DENISE_REVISION, DENISE_ECS);
                set(OPT_AMIGA_VIDEO_FORMAT, PAL);
                set(OPT_MEM_CHIP_RAM, 512);
                set(OPT_MEM_SLOW_RAM, 512);
                break;

            default:
                fatalError;
        }
    }
}

Configurable *
Amiga::routeOption(Option opt, isize objid)
{
    return CoreComponent::routeOption(opt, objid);
}

const Configurable *
Amiga::routeOption(Option opt, isize objid) const
{
    auto result = const_cast<Amiga *>(this)->routeOption(opt, objid);
    return const_cast<const Configurable *>(result);
}

i64
Amiga::overrideOption(Option opt, i64 value) const
{
    static std::map<Option,i64> overrides = OVERRIDES;

    if (overrides.find(opt) != overrides.end()) {

        msg("Overriding option: %s = %lld\n", OptionEnum::key(opt), value);
        return overrides[opt];
    }

    return value;
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

        case PAL:   return 50.0;
        case NTSC:  return 60.0;

        default:
            fatalError;
    }
}

i64
Amiga::nativeMasterClockFrequency() const
{
    switch (config.type) {

        case PAL:   return CLK_FREQUENCY_PAL;
        case NTSC:  return CLK_FREQUENCY_NTSC;

        default:
            fatalError;
    }
}

double
Amiga::refreshRate() const
{
    if (config.vsync) {

        return double(host.getOption(OPT_HOST_REFRESH_RATE));

    } else {

        return nativeRefreshRate() * config.timeLapse / 100.0;
    }
}

i64
Amiga::masterClockFrequency() const
{
    return nativeMasterClockFrequency() * config.timeLapse / 100;
}

void
Amiga::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::State) {

        os << tab("Power");
        os << bol(isPoweredOn()) << std::endl;
        os << tab("Running");
        os << bol(isRunning()) << std::endl;
        os << tab("Suspended");
        os << bol(isSuspended()) << std::endl;
        os << std::endl;

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
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "_powerOn\n");

    // Perform a reset
    hardReset();

    // Start from a snapshot if requested
    if (string(INITIAL_SNAPSHOT) != "") {

        Snapshot snapshot(INITIAL_SNAPSHOT);
        loadSnapshot(snapshot);
    }

    // Set initial breakpoints
    for (auto &bp : std::vector <u32> (INITIAL_BREAKPOINTS)) {

        cpu.debugger.breakpoints.setAt(bp);
        // track = true; // TODO: FIXME
    }

    msgQueue.put(MSG_POWER, 1);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff\n");

    // Perform a reset
    hardReset();

    msgQueue.put(MSG_POWER, 0);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run\n");

    // Enable or disable CPU debugging
    // track ? cpu.debugger.enableLogging() : cpu.debugger.disableLogging(); // TODO: FIXME

    msgQueue.put(MSG_RUN);
}

void
Amiga::_pause()
{
    debug(RUN_DEBUG, "_pause\n");

    remoteManager.gdbServer.breakpointReached();

    msgQueue.put(MSG_PAUSE);
}

void
Amiga::_halt()
{
    debug(RUN_DEBUG, "_halt\n");

    msgQueue.put(MSG_SHUTDOWN);
}

void
Amiga::_warpOn()
{
    debug(RUN_DEBUG, "_warpOn\n");

    msgQueue.put(MSG_WARP, 1);
}

void
Amiga::_warpOff()
{
    debug(RUN_DEBUG, "_warpOff\n");

    msgQueue.put(MSG_WARP, 0);
}

void
Amiga::_trackOn()
{
    debug(RUN_DEBUG, "_trackOn\n");

    msgQueue.put(MSG_TRACK, 1);
}

void
Amiga::_trackOff()
{
    debug(RUN_DEBUG, "_trackOff\n");

    msgQueue.put(MSG_TRACK, 0);
}

void 
Amiga::update()
{
    if (retroShell.isDirty) {

        retroShell.isDirty = false;
        msgQueue.put(MSG_RSH_UPDATE);
    }
}

void
Amiga::computeFrame()
{
    while (1) {

        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {

            // Did we reach a soft breakpoint?
            if (flags & RL::SOFTSTOP_REACHED) {
                clearFlag(RL::SOFTSTOP_REACHED);
                msgQueue.put(MSG_STEP);
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {
                clearFlag(RL::BREAKPOINT_REACHED);
                auto addr = cpu.debugger.breakpoints.hit->addr;
                msgQueue.put(MSG_BREAKPOINT_REACHED, CpuMsg { addr, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {
                clearFlag(RL::WATCHPOINT_REACHED);
                auto addr = cpu.debugger.watchpoints.hit->addr;
                msgQueue.put(MSG_WATCHPOINT_REACHED, CpuMsg { addr, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a catchpoint?
            if (flags & RL::CATCHPOINT_REACHED) {
                clearFlag(RL::CATCHPOINT_REACHED);
                auto vector = u8(cpu.debugger.catchpoints.hit->addr);
                msgQueue.put(MSG_CATCHPOINT_REACHED, CpuMsg { cpu.getPC0(), vector });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a software trap?
            if (flags & RL::SWTRAP_REACHED) {
                clearFlag(RL::SWTRAP_REACHED);
                msgQueue.put(MSG_SWTRAP_REACHED, CpuMsg { cpu.getPC0(), 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a beam trap?
            if (flags & RL::BEAMTRAP_REACHED) {
                clearFlag(RL::BEAMTRAP_REACHED);
                msgQueue.put(MSG_BEAMTRAP_REACHED, CpuMsg { 0, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a Copper breakpoint?
            if (flags & RL::COPPERBP_REACHED) {
                clearFlag(RL::COPPERBP_REACHED);
                auto addr = u8(agnus.copper.debugger.breakpoints.hit()->addr);
                msgQueue.put(MSG_COPPERBP_REACHED, CpuMsg { addr, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a Copper watchpoint?
            if (flags & RL::COPPERWP_REACHED) {
                clearFlag(RL::COPPERWP_REACHED);
                auto addr = u8(agnus.copper.debugger.watchpoints.hit()->addr);
                msgQueue.put(MSG_COPPERWP_REACHED, CpuMsg { addr, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Are we requested to terminate the run loop?
            if (flags & RL::STOP) {
                clearFlag(RL::STOP);
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Are we requested to synchronize the thread?
            if (flags & RL::SYNC_THREAD) {
                clearFlag(RL::SYNC_THREAD);
                break;
            }
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
    SYNCHRONIZED

    flags |= flag;
}

void
Amiga::clearFlag(u32 flag)
{
    SYNCHRONIZED

    flags &= ~flag;
}

MediaFile *
Amiga::takeSnapshot()
{
    {   SUSPENDED

        return new Snapshot(*this);
    }
}

void
Amiga::serviceSnpEvent(EventID eventId)
{
    // Check for the main instance (ignore the run-ahead instance)
    if (objid == 0) {

        // Take snapshot and hand it over to GUI
        msgQueue.put(MSG_SNAPSHOT_TAKEN, SnapshotMsg { .snapshot = new Snapshot(*this) } );
    }

    // Schedule the next event
    scheduleNextSnpEvent();
}

void
Amiga::scheduleNextSnpEvent()
{
    auto snapshots = emulator.get(OPT_AMIGA_SNAPSHOTS);
    auto delay = emulator.get(OPT_AMIGA_SNAPSHOT_DELAY);

    if (snapshots) {
        agnus.scheduleRel<SLOT_SNP>(SEC(double(delay)), SNP_TAKE);
    } else {
        agnus.cancel<SLOT_SNP>();
    }
}

/*
void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        takeAutoSnapshot();

    } else {

        // Schedule the snapshot to be taken
        signalAutoSnapshot();
    }
}

void
Amiga::requestUserSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        takeUserSnapshot();

    } else {

        // Schedule the snapshot to be taken
        signalUserSnapshot();
    }
}

Snapshot *
Amiga::latestAutoSnapshot()
{
    Snapshot *result = autoSnapshot;
    autoSnapshot = nullptr;
    return result;
}

Snapshot *
Amiga::latestUserSnapshot()
{
    Snapshot *result = userSnapshot;
    userSnapshot = nullptr;
    return result;
}
*/

void 
Amiga::loadSnapshot(const MediaFile &file)
{
    try {

        const Snapshot &snapshot = dynamic_cast<const Snapshot &>(file);
        loadSnapshot(snapshot);

    } catch (...) {

        throw Error(ERROR_FILE_TYPE_MISMATCH);
    }
}

void
Amiga::loadSnapshot(const Snapshot &snapshot)
{
    {   SUSPENDED

        try {

            // Restore the saved state
            load(snapshot.getData());

        } catch (Error &error) {

            /* If we reach this point, the emulator has been put into an
             * inconsistent state due to corrupted snapshot data. We cannot
             * continue emulation, because it would likely crash the
             * application. Because we cannot revert to the old state either,
             * we perform a hard reset to eliminate the inconsistency.
             */
            hardReset();
            throw error;
        }
    }

    // Inform the GUI
    msgQueue.put(MSG_SNAPSHOT_RESTORED);
    msgQueue.put(MSG_VIDEO_FORMAT, agnus.isPAL() ? PAL : NTSC);
}

/*
void
Amiga::takeAutoSnapshot()
{
    if (autoSnapshot) {

        warn("Old auto-snapshot still present. Ignoring request.\n");
        return;
    }

    autoSnapshot = new Snapshot(*this);
    msgQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
}

void
Amiga::takeUserSnapshot()
{
    if (userSnapshot) {

        warn("Old user-snapshot still present. Ignoring request.\n");
        return;
    }

    userSnapshot = new Snapshot(*this);
    msgQueue.put(MSG_USER_SNAPSHOT_TAKEN);
}
*/

void
Amiga::processCommand(const Cmd &cmd)
{
    switch (cmd.type) {

        case CMD_ALARM_ABS:

            setAlarmAbs(cmd.alarm.cycle, cmd.alarm.value);
            break;

        case CMD_ALARM_REL:

            setAlarmRel(cmd.alarm.cycle, cmd.alarm.value);
            break;

        case CMD_INSPECTION_TARGET:

            setAutoInspectionMask(cmd.value);
            break;

        default:
            fatalError;
    }
}

void
Amiga::eolHandler()
{
    // Get the maximum number of rasterlines
    auto lines = agnus.isPAL() ? VPOS_CNT_PAL : VPOS_CNT_NTSC;

    // Check if we need to sync the thread
    if (agnus.pos.v % (lines + 1) == 0) {

        setFlag(RL::SYNC_THREAD);
    }
}

void
Amiga::setAlarmAbs(Cycle trigger, i64 payload)
{
    {   SUSPENDED

        alarms.push_back(Alarm { trigger, payload });
        scheduleNextAlarm();
    }
}

void
Amiga::setAlarmRel(Cycle trigger, i64 payload)
{
    {   SUSPENDED

        alarms.push_back(Alarm { agnus.clock + trigger, payload });
        scheduleNextAlarm();
    }
}

void
Amiga::serviceAlarmEvent()
{
    for (auto it = alarms.begin(); it != alarms.end(); ) {

        if (it->trigger <= agnus.clock) {
            msgQueue.put(MSG_ALARM, it->payload);
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

int
Amiga::getDebugVariable(DebugFlag flag)
{
    switch (flag) {

        case FLAG_XFILES:           return XFILES;

        case FLAG_CNF_DEBUG:        return CNF_DEBUG;
        case FLAG_OBJ_DEBUG:        return OBJ_DEBUG;
        case FLAG_DEF_DEBUG:        return DEF_DEBUG;
        case FLAG_MIMIC_UAE:        return MIMIC_UAE;

        case FLAG_RUN_DEBUG:        return RUN_DEBUG;
        case FLAG_TIM_DEBUG:        return TIM_DEBUG;
        case FLAG_WARP_DEBUG:       return WARP_DEBUG;
        case FLAG_CMD_DEBUG:        return CMD_DEBUG;
        case FLAG_QUEUE_DEBUG:      return QUEUE_DEBUG;
        case FLAG_SNP_DEBUG:        return SNP_DEBUG;

        case FLAG_RUA_DEBUG:        return RUA_DEBUG;
        case FLAG_RUA_CHECKSUM:     return RUA_CHECKSUM;
        case FLAG_RUA_ON_STEROIDS:  return RUA_ON_STEROIDS;

        case FLAG_CPU_DEBUG:        return CPU_DEBUG;
        case FLAG_CST_DEBUG:        return CST_DEBUG;

        case FLAG_OCSREG_DEBUG:     return OCSREG_DEBUG;
        case FLAG_ECSREG_DEBUG:     return ECSREG_DEBUG;
        case FLAG_INVREG_DEBUG:     return INVREG_DEBUG;
        case FLAG_MEM_DEBUG:        return MEM_DEBUG;

        case FLAG_DMA_DEBUG:        return DMA_DEBUG;
        case FLAG_DDF_DEBUG:        return DDF_DEBUG;
        case FLAG_SEQ_DEBUG:        return SEQ_DEBUG;
        case FLAG_SEQ_ON_STEROIDS:  return SEQ_ON_STEROIDS;
        case FLAG_NTSC_DEBUG:       return NTSC_DEBUG;

        case FLAG_COP_CHECKSUM:     return COP_CHECKSUM;
        case FLAG_COPREG_DEBUG:     return COPREG_DEBUG;
        case FLAG_COP_DEBUG:        return COP_DEBUG;

        case FLAG_BLT_CHECKSUM:     return BLT_CHECKSUM;
        case FLAG_BLTREG_DEBUG:     return BLTREG_DEBUG;
        case FLAG_BLT_REG_GUARD:    return BLT_REG_GUARD;
        case FLAG_BLT_MEM_GUARD:    return BLT_MEM_GUARD;
        case FLAG_BLT_DEBUG:        return BLT_DEBUG;
        case FLAG_BLTTIM_DEBUG:     return BLTTIM_DEBUG;
        case FLAG_SLOW_BLT_DEBUG:   return SLOW_BLT_DEBUG;
        case FLAG_OLD_LINE_BLIT:    return OLD_LINE_BLIT;

        case FLAG_BPLREG_DEBUG:     return BPLREG_DEBUG;
        case FLAG_BPLDAT_DEBUG:     return BPLDAT_DEBUG;
        case FLAG_BPLMOD_DEBUG:     return BPLMOD_DEBUG;
        case FLAG_SPRREG_DEBUG:     return SPRREG_DEBUG;
        case FLAG_COLREG_DEBUG:     return COLREG_DEBUG;
        case FLAG_CLXREG_DEBUG:     return CLXREG_DEBUG;
        case FLAG_BPL_ON_STEROIDS:  return BPL_ON_STEROIDS;
        case FLAG_DIW_DEBUG:        return DIW_DEBUG;
        case FLAG_SPR_DEBUG:        return SPR_DEBUG;
        case FLAG_CLX_DEBUG:        return CLX_DEBUG;
        case FLAG_BORDER_DEBUG:     return BORDER_DEBUG;
        case FLAG_LINE_DEBUG:       return LINE_DEBUG;

        case FLAG_INTREG_DEBUG:     return INTREG_DEBUG;
        case FLAG_INT_DEBUG:        return INT_DEBUG;

        case FLAG_CIAREG_DEBUG:     return CIAREG_DEBUG;
        case FLAG_CIASER_DEBUG:     return CIASER_DEBUG;
        case FLAG_CIA_DEBUG:        return CIA_DEBUG;
        case FLAG_TOD_DEBUG:        return TOD_DEBUG;

        case FLAG_ALIGN_HEAD:       return ALIGN_HEAD;
        case FLAG_DSK_CHECKSUM:     return DSK_CHECKSUM;
        case FLAG_DSKREG_DEBUG:     return DSKREG_DEBUG;
        case FLAG_DSK_DEBUG:        return DSK_DEBUG;
        case FLAG_MFM_DEBUG:        return MFM_DEBUG;
        case FLAG_FS_DEBUG:         return FS_DEBUG;

        case FLAG_HDR_ACCEPT_ALL:   return HDR_ACCEPT_ALL;
        case FLAG_HDR_FS_LOAD_ALL:  return HDR_FS_LOAD_ALL;
        case FLAG_WT_DEBUG:         return WT_DEBUG;

        case FLAG_AUDREG_DEBUG:     return AUDREG_DEBUG;
        case FLAG_AUD_DEBUG:        return AUD_DEBUG;
        case FLAG_AUDBUF_DEBUG:     return AUDBUF_DEBUG;
        case FLAG_AUDVOL_DEBUG:     return AUDVOL_DEBUG;
        case FLAG_DISABLE_AUDIRQ:   return DISABLE_AUDIRQ;

        case FLAG_POSREG_DEBUG:     return POSREG_DEBUG;
        case FLAG_JOYREG_DEBUG:     return JOYREG_DEBUG;
        case FLAG_POTREG_DEBUG:     return POTREG_DEBUG;
        case FLAG_PRT_DEBUG:        return PRT_DEBUG;
        case FLAG_SER_DEBUG:        return SER_DEBUG;
        case FLAG_POT_DEBUG:        return POT_DEBUG;
        case FLAG_HOLD_MOUSE_L:     return HOLD_MOUSE_L;
        case FLAG_HOLD_MOUSE_M:     return HOLD_MOUSE_M;
        case FLAG_HOLD_MOUSE_R:     return HOLD_MOUSE_R;

        case FLAG_ZOR_DEBUG:        return ZOR_DEBUG;
        case FLAG_ACF_DEBUG:        return ACF_DEBUG;
        case FLAG_FAS_DEBUG:        return FAS_DEBUG;
        case FLAG_HDR_DEBUG:        return HDR_DEBUG;
        case FLAG_DBD_DEBUG:        return DBD_DEBUG;

        case FLAG_ADF_DEBUG:        return ADF_DEBUG;
        case FLAG_DMS_DEBUG:        return DMS_DEBUG;
        case FLAG_IMG_DEBUG:        return IMG_DEBUG;

        case FLAG_RTC_DEBUG:        return RTC_DEBUG;
        case FLAG_KBD_DEBUG:        return KBD_DEBUG;

        case FLAG_REC_DEBUG:        return REC_DEBUG;
        case FLAG_SCK_DEBUG:        return SCK_DEBUG;
        case FLAG_SRV_DEBUG:        return SRV_DEBUG;
        case FLAG_GDB_DEBUG:        return GDB_DEBUG;

        default:
            throw Error(ERROR_OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }
}

void
Amiga::setDebugVariable(DebugFlag flag, int val)
{
#ifdef RELEASEBUILD

    throw Error(ERROR_OPT_UNSUPPORTED, "Debug variables can only be altered in debug builds.");

#else

    switch (flag) {

        case FLAG_XFILES:           XFILES          = val; break;

        case FLAG_CNF_DEBUG:        CNF_DEBUG       = val; break;
        case FLAG_OBJ_DEBUG:        OBJ_DEBUG       = val; break;
        case FLAG_DEF_DEBUG:        DEF_DEBUG       = val; break;
        case FLAG_MIMIC_UAE:        MIMIC_UAE       = val; break;

        case FLAG_RUN_DEBUG:        RUN_DEBUG       = val; break;
        case FLAG_TIM_DEBUG:        TIM_DEBUG       = val; break;
        case FLAG_WARP_DEBUG:       WARP_DEBUG      = val; break;
        case FLAG_CMD_DEBUG:        CMD_DEBUG       = val; break;
        case FLAG_QUEUE_DEBUG:      QUEUE_DEBUG     = val; break;
        case FLAG_SNP_DEBUG:        SNP_DEBUG       = val; break;

        case FLAG_RUA_DEBUG:        RUA_DEBUG       = val; break;
        case FLAG_RUA_CHECKSUM:     RUA_CHECKSUM    = val; break;
        case FLAG_RUA_ON_STEROIDS:  RUA_ON_STEROIDS = val; break;

        case FLAG_CPU_DEBUG:        CPU_DEBUG       = val; break;
        case FLAG_CST_DEBUG:        CST_DEBUG       = val; break;

        case FLAG_OCSREG_DEBUG:     OCSREG_DEBUG    = val; break;
        case FLAG_ECSREG_DEBUG:     ECSREG_DEBUG    = val; break;
        case FLAG_INVREG_DEBUG:     INVREG_DEBUG    = val; break;
        case FLAG_MEM_DEBUG:        MEM_DEBUG       = val; break;

        case FLAG_DMA_DEBUG:        DMA_DEBUG       = val; break;
        case FLAG_DDF_DEBUG:        DDF_DEBUG       = val; break;
        case FLAG_SEQ_DEBUG:        SEQ_DEBUG       = val; break;
        case FLAG_SEQ_ON_STEROIDS:  SEQ_ON_STEROIDS = val; break;
        case FLAG_NTSC_DEBUG:       NTSC_DEBUG      = val; break;

        case FLAG_COP_CHECKSUM:     COP_CHECKSUM    = val; break;
        case FLAG_COPREG_DEBUG:     COPREG_DEBUG    = val; break;
        case FLAG_COP_DEBUG:        COP_DEBUG       = val; break;

        case FLAG_BLT_CHECKSUM:     BLT_CHECKSUM    = val; break;
        case FLAG_BLTREG_DEBUG:     BLTREG_DEBUG    = val; break;
        case FLAG_BLT_REG_GUARD:    BLT_REG_GUARD   = val; break;
        case FLAG_BLT_MEM_GUARD:    BLT_MEM_GUARD   = val; break;
        case FLAG_BLT_DEBUG:        BLT_DEBUG       = val; break;
        case FLAG_BLTTIM_DEBUG:     BLTTIM_DEBUG    = val; break;
        case FLAG_SLOW_BLT_DEBUG:   SLOW_BLT_DEBUG  = val; break;
        case FLAG_OLD_LINE_BLIT:    OLD_LINE_BLIT   = val; break;

        case FLAG_BPLREG_DEBUG:     BPLREG_DEBUG    = val; break;
        case FLAG_BPLDAT_DEBUG:     BPLDAT_DEBUG    = val; break;
        case FLAG_BPLMOD_DEBUG:     BPLMOD_DEBUG    = val; break;
        case FLAG_SPRREG_DEBUG:     SPRREG_DEBUG    = val; break;
        case FLAG_COLREG_DEBUG:     COLREG_DEBUG    = val; break;
        case FLAG_CLXREG_DEBUG:     CLXREG_DEBUG    = val; break;
        case FLAG_BPL_ON_STEROIDS:  BPL_ON_STEROIDS = val; break;
        case FLAG_DIW_DEBUG:        DIW_DEBUG       = val; break;
        case FLAG_SPR_DEBUG:        SPR_DEBUG       = val; break;
        case FLAG_CLX_DEBUG:        CLX_DEBUG       = val; break;
        case FLAG_BORDER_DEBUG:     BORDER_DEBUG    = val; break;
        case FLAG_LINE_DEBUG:       LINE_DEBUG      = val; break;

        case FLAG_INTREG_DEBUG:     INTREG_DEBUG    = val; break;
        case FLAG_INT_DEBUG:        INT_DEBUG       = val; break;

        case FLAG_CIAREG_DEBUG:     CIAREG_DEBUG    = val; break;
        case FLAG_CIASER_DEBUG:     CIASER_DEBUG    = val; break;
        case FLAG_CIA_DEBUG:        CIA_DEBUG       = val; break;
        case FLAG_TOD_DEBUG:        TOD_DEBUG       = val; break;

        case FLAG_ALIGN_HEAD:       ALIGN_HEAD      = val; break;
        case FLAG_DSK_CHECKSUM:     DSK_CHECKSUM    = val; break;
        case FLAG_DSKREG_DEBUG:     DSKREG_DEBUG    = val; break;
        case FLAG_DSK_DEBUG:        DSK_DEBUG       = val; break;
        case FLAG_MFM_DEBUG:        MFM_DEBUG       = val; break;
        case FLAG_FS_DEBUG:         FS_DEBUG        = val; break;

        case FLAG_HDR_ACCEPT_ALL:   HDR_ACCEPT_ALL  = val; break;
        case FLAG_HDR_FS_LOAD_ALL:  HDR_FS_LOAD_ALL = val; break;
        case FLAG_WT_DEBUG:         WT_DEBUG        = val; break;

        case FLAG_AUDREG_DEBUG:     AUDREG_DEBUG    = val; break;
        case FLAG_AUD_DEBUG:        AUD_DEBUG       = val; break;
        case FLAG_AUDBUF_DEBUG:     AUDBUF_DEBUG    = val; break;
        case FLAG_AUDVOL_DEBUG:     AUDVOL_DEBUG    = val; break;
        case FLAG_DISABLE_AUDIRQ:   DISABLE_AUDIRQ  = val; break;

        case FLAG_POSREG_DEBUG:     POSREG_DEBUG    = val; break;
        case FLAG_JOYREG_DEBUG:     JOYREG_DEBUG    = val; break;
        case FLAG_POTREG_DEBUG:     POTREG_DEBUG    = val; break;
        case FLAG_PRT_DEBUG:        PRT_DEBUG       = val; break;
        case FLAG_SER_DEBUG:        SER_DEBUG       = val; break;
        case FLAG_POT_DEBUG:        POT_DEBUG       = val; break;
        case FLAG_HOLD_MOUSE_L:     HOLD_MOUSE_L    = val; break;
        case FLAG_HOLD_MOUSE_M:     HOLD_MOUSE_M    = val; break;
        case FLAG_HOLD_MOUSE_R:     HOLD_MOUSE_R    = val; break;

        case FLAG_ZOR_DEBUG:        ZOR_DEBUG       = val; break;
        case FLAG_ACF_DEBUG:        ACF_DEBUG       = val; break;
        case FLAG_FAS_DEBUG:        FAS_DEBUG       = val; break;
        case FLAG_HDR_DEBUG:        HDR_DEBUG       = val; break;
        case FLAG_DBD_DEBUG:        DBD_DEBUG       = val; break;

        case FLAG_ADF_DEBUG:        ADF_DEBUG       = val; break;
        case FLAG_DMS_DEBUG:        DMS_DEBUG       = val; break;
        case FLAG_IMG_DEBUG:        IMG_DEBUG       = val; break;

        case FLAG_RTC_DEBUG:        RTC_DEBUG       = val; break;
        case FLAG_KBD_DEBUG:        KBD_DEBUG       = val; break;

        case FLAG_REC_DEBUG:        REC_DEBUG       = val; break;
        case FLAG_SCK_DEBUG:        SCK_DEBUG       = val; break;
        case FLAG_SRV_DEBUG:        SRV_DEBUG       = val; break;
        case FLAG_GDB_DEBUG:        GDB_DEBUG       = val; break;

        default:
            throw Error(ERROR_OPT_UNSUPPORTED,
                        "Unhandled debug variable: " + string(DebugFlagEnum::key(flag)));
    }
#endif
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
