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

            if (objid == 1) fprintf(stderr, "[Run-ahead] ");
            fprintf(stderr, "%s:%ld", component, line);
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
        case OPT_AMIGA_SPEED_BOOST:     return config.speedBoost;
        case OPT_AMIGA_RUN_AHEAD:       return config.runAhead;
        case OPT_AMIGA_SNAP_AUTO:       return config.snapshots;
        case OPT_AMIGA_SNAP_DELAY:      return config.snapshotDelay;
        case OPT_AMIGA_SNAP_COMPRESS:   return config.compressSnapshots;

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
                throw Error(VAERROR_OPT_INV_ARG, VideoFormatEnum::keyList());
            }
            return;

        case OPT_AMIGA_WARP_BOOT:

            return;

        case OPT_AMIGA_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, WarpModeEnum::keyList());
            }
            return;

        case OPT_AMIGA_VSYNC:

            return;

        case OPT_AMIGA_SPEED_BOOST:

            if (value < 50 || value > 200) {
                throw Error(VAERROR_OPT_INV_ARG, "50...200");
            }
            return;

        case OPT_AMIGA_RUN_AHEAD:

            if (value < 0 || value > 12) {
                throw Error(VAERROR_OPT_INV_ARG, "0...12");
            }
            return;

        case OPT_AMIGA_SNAP_AUTO:

            return;

        case OPT_AMIGA_SNAP_DELAY:

            if (value < 10 || value > 3600) {
                throw Error(VAERROR_OPT_INV_ARG, "10...3600");
            }
            return;

        case OPT_AMIGA_SNAP_COMPRESS:

            return;
            
        default:
            throw Error(VAERROR_OPT_UNSUPPORTED);
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

            config.speedBoost = isize(value);
            return;

        case OPT_AMIGA_RUN_AHEAD:

            config.runAhead = isize(value);
            return;

        case OPT_AMIGA_SNAP_AUTO:

            config.snapshots = bool(value);
            scheduleNextSnpEvent();
            return;

        case OPT_AMIGA_SNAP_DELAY:

            config.snapshotDelay = isize(value);
            scheduleNextSnpEvent();
            return;

        case OPT_AMIGA_SNAP_COMPRESS:

            config.compressSnapshots = bool(value);
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
        throw Error(VAERROR_FILE_CANT_WRITE);
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
Amiga::get(Option opt, isize objid) const
{
    debug(CNF_DEBUG, "get(%s, %ld)\n", OptionEnum::key(opt), objid);

    auto target = routeOption(opt, objid);
    if (target == nullptr) throw Error(VAERROR_OPT_INV_ID);
    return target->getOption(opt);
}

void
Amiga::check(Option opt, i64 value, const std::vector<isize> objids)
{
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
        if (target == nullptr) throw Error(VAERROR_OPT_INV_ID);

        target->checkOption(opt, value);
    }
}

void
Amiga::set(Option opt, i64 value, const std::vector<isize> objids)
{
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
        if (target == nullptr) throw Error(VAERROR_OPT_INV_ID);

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

    hardReset();
    msgQueue.put(MSG_POWER, 1);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff\n");

    hardReset();
    msgQueue.put(MSG_POWER, 0);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run\n");

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
Amiga::update(CmdQueue &queue)
{
    Cmd cmd;
    bool cmdConfig = false;

    auto dfn = [&]() -> FloppyDrive& { return *df[cmd.value]; };
    auto cp = [&]() -> ControlPort& { return cmd.value ? controlPort2 : controlPort1; };

    // Process all commands
    while (queue.poll(cmd)) {

        switch (cmd.type) {

            case CMD_CONFIG:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { cmd.config.id });
                break;

            case CMD_CONFIG_ALL:

                cmdConfig = true;
                set(cmd.config.option, cmd.config.value, { });
                break;

            case CMD_ALARM_ABS:
            case CMD_ALARM_REL:
            case CMD_INSPECTION_TARGET:

                processCommand(cmd);
                break;

            case CMD_GUARD_SET_AT:
            case CMD_GUARD_MOVE_NR:
            case CMD_GUARD_IGNORE_NR:
            case CMD_GUARD_REMOVE_NR:
            case CMD_GUARD_REMOVE_AT:
            case CMD_GUARD_REMOVE_ALL:
            case CMD_GUARD_ENABLE_NR:
            case CMD_GUARD_ENABLE_AT:
            case CMD_GUARD_ENABLE_ALL:
            case CMD_GUARD_DISABLE_NR:
            case CMD_GUARD_DISABLE_AT:
            case CMD_GUARD_DISABLE_ALL:

                cpu.processCommand(cmd);
                break;

            case CMD_KEY_PRESS:
            case CMD_KEY_RELEASE:
            case CMD_KEY_RELEASE_ALL:
            case CMD_KEY_TOGGLE:

                keyboard.processCommand(cmd);
                break;

            case CMD_MOUSE_MOVE_ABS:
            case CMD_MOUSE_MOVE_REL:

                cp().processCommand(cmd); break;
                break;

            case CMD_MOUSE_EVENT:
            case CMD_JOY_EVENT:

                cp().processCommand(cmd); break;
                break;

            case CMD_DSK_TOGGLE_WP:
            case CMD_DSK_MODIFIED:
            case CMD_DSK_UNMODIFIED:

                dfn().processCommand(cmd);
                break;

                
            case CMD_RSH_EXECUTE:

                retroShell.exec();
                break;

            case CMD_FOCUS:

                cmd.value ? focus() : unfocus();
                break;

            default:
                fatal("Unhandled command: %s\n", CmdTypeEnum::key(cmd.type));
        }
    }

    // Inform the GUI about a changed machine configuration
    if (cmdConfig) { msgQueue.put(MSG_CONFIG); }

    // Inform the GUI about new RetroShell content
    if (retroShell.isDirty) { retroShell.isDirty = false; msgQueue.put(MSG_RSH_UPDATE); }
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
    Snapshot *result;
    
    // Take the snapshot
    { SUSPENDED result = new Snapshot(*this); }
    
    // Compress the snapshot if requested
    if (config.compressSnapshots) result->compress();
    
    return result;
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
    auto snapshots = emulator.get(OPT_AMIGA_SNAP_AUTO);
    auto delay = emulator.get(OPT_AMIGA_SNAP_DELAY);

    if (snapshots) {
        agnus.scheduleRel<SLOT_SNP>(SEC(double(delay)), SNP_TAKE);
    } else {
        agnus.cancel<SLOT_SNP>();
    }
}

void 
Amiga::loadSnapshot(const MediaFile &file)
{
    try {

        const Snapshot &snapshot = dynamic_cast<const Snapshot &>(file);
        loadSnapshot(snapshot);

    } catch (...) {

        throw Error(VAERROR_FILE_TYPE_MISMATCH);
    }
}

void
Amiga::loadSnapshot(const Snapshot &snap)
{
    // Make a copy so we can modify the snapshot
    Snapshot snapshot(snap);

    // Uncompress the snapshot
    snapshot.uncompress();
    
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
