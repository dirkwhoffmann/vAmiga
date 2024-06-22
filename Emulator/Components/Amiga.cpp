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
#include "Snapshot.h"
#include "ADFFile.h"
#include <algorithm>

namespace vamiga {

// Perform some consistency checks
static_assert(sizeof(i8)  == 1, "i8  size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8)  == 1, "u8  size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");

Defaults Amiga::defaults;

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
    /* The order of subcomponents is important here, because some components
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

        &agnus,
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
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3ld,%3ld) ",
            agnus.pos.frame, agnus.pos.v, agnus.pos.h);

    fprintf(stderr, "%06X ", cpu.getPC0());
    fprintf(stderr, "%2X ", cpu.getIPL());

    u16 dmacon = agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
            (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');

    fprintf(stderr, "%04X %04X ", paula.intena, paula.intreq);

    if (agnus.copper.servicing) {
        fprintf(stderr, "[%06X] ", agnus.copper.getCopPC0());
    }
}

void
Amiga::reset(bool hard)
{
    suspend();

    // Call the pre-reset delegate
    postorderWalk([hard](CoreComponent *c) { c->willReset(hard); });

    // If a disk change is in progress, finish it
    df0.serviceDiskChangeEvent <SLOT_DC0> ();
    df1.serviceDiskChangeEvent <SLOT_DC1> ();
    df2.serviceDiskChangeEvent <SLOT_DC2> ();
    df3.serviceDiskChangeEvent <SLOT_DC3> ();

    // Execute the standard reset routine
    CoreComponent::reset(hard);

    // Call the pre-reset delegate
    postorderWalk([hard](CoreComponent *c) { c->didReset(hard); });

    resume();

    // Inform the GUI
    if (hard) msgQueue.put(MSG_RESET);
}

void
Amiga::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    // Schedule initial events
    scheduleNextSnpEvent();

    // Clear all runloop flags
    flags = 0;
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

        default:
            fatalError;
    }
}

void
Amiga::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_AMIGA_VIDEO_FORMAT:
        case OPT_AMIGA_WARP_BOOT:

            return;

        case OPT_AMIGA_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, WarpModeEnum::keyList());
            }
            return;

        case OPT_AMIGA_VSYNC:
        case OPT_AMIGA_SPEED_BOOST:
        case OPT_AMIGA_SNAPSHOTS:

            return;

        case OPT_AMIGA_SNAPSHOT_DELAY:

            if (value < 10 || value > 3600) {
                throw Error(ERROR_OPT_INV_ARG, "10...3600");
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

            if (!VideoFormatEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, VideoFormatEnum::keyList());
            }

            if (value != config.type) {

                SUSPENDED

                config.type = VideoFormat(value);
                agnus.setVideoFormat(config.type);
            }
            return;

        case OPT_AMIGA_WARP_BOOT:

            config.warpBoot = isize(value);
            return;

        case OPT_AMIGA_WARP_MODE:

            if (!WarpModeEnum::isValid(value)) {
                throw Error(ERROR_OPT_INV_ARG, WarpModeEnum::keyList());
            }

            config.warpMode = WarpMode(value);
            return;

        case OPT_AMIGA_VSYNC:

            config.vsync = bool(value);
            return;

        case OPT_AMIGA_SPEED_BOOST:

            if (value < 50 || value > 200) {
                throw Error(ERROR_OPT_INV_ARG, "50...200");
            }

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

        default:
            fatalError;
    }
}

void
Amiga::exportConfig(const fs::path &path) const
{
    auto fs = std::ofstream(path, std::ofstream::binary);

    if (!fs.is_open()) {
        throw Error(ERROR_FILE_CANT_WRITE);
    }

    exportConfig(fs);
}

void
Amiga::exportConfig(std::ostream &stream) const
{
    stream << "# vAmiga " << Amiga::build() << "\n";
    stream << "\n";
    stream << "amiga power off\n";
    stream << "\n";
    CoreComponent::exportConfig(stream);
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

InspectionTarget
Amiga::getInspectionTarget() const
{
    switch(agnus.id[SLOT_INS]) {

        case EVENT_NONE:  return INSPECTION_NONE;
        case INS_AMIGA:   return INSPECTION_AMIGA;
        case INS_CPU:     return INSPECTION_CPU;
        case INS_MEM:     return INSPECTION_MEM;
        case INS_CIA:     return INSPECTION_CIA;
        case INS_AGNUS:   return INSPECTION_AGNUS;
        case INS_BLITTER: return INSPECTION_BLITTER;
        case INS_COPPER:  return INSPECTION_COPPER;
        case INS_PAULA:   return INSPECTION_PAULA;
        case INS_DENISE:  return INSPECTION_DENISE;
        case INS_PORTS:   return INSPECTION_PORTS;
        case INS_EVENTS:  return INSPECTION_EVENTS;

        default:
            fatalError;
    }
}

void
Amiga::setInspectionTarget(InspectionTarget target, Cycle trigger)
{
    EventID id;

    {   SUSPENDED

        switch(target) {

            case INSPECTION_NONE:    agnus.cancel<SLOT_INS>(); return;

            case INSPECTION_AMIGA:   id = INS_AMIGA; break;
            case INSPECTION_CPU:     id = INS_CPU; break;
            case INSPECTION_MEM:     id = INS_MEM; break;
            case INSPECTION_CIA:     id = INS_CIA; break;
            case INSPECTION_AGNUS:   id = INS_AGNUS; break;
            case INSPECTION_BLITTER: id = INS_BLITTER; break;
            case INSPECTION_COPPER:  id = INS_COPPER; break;
            case INSPECTION_PAULA:   id = INS_PAULA; break;
            case INSPECTION_DENISE:  id = INS_DENISE; break;
            case INSPECTION_PORTS:   id = INS_PORTS; break;
            case INSPECTION_EVENTS:  id = INS_EVENTS; break;

            default:
                fatalError;
        }

        agnus.scheduleRel<SLOT_INS>(trigger, id);
        if (trigger == 0) agnus.serviceINSEvent(id);
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

        return double(emulator.host.getOption(OPT_HOST_REFRESH_RATE));

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

    if (category == Category::Defaults) {

        defaults.dump(category, os);
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

isize
Amiga::load(const u8 *buffer)
{
    auto result = CoreComponent::load(buffer);
    postorderWalk([](CoreComponent *c) { c->_didLoad(); });

    return result;
}

isize
Amiga::save(u8 *buffer)
{
    auto result = CoreComponent::save(buffer);
    postorderWalk([](CoreComponent *c) { c->_didSave(); });

    return result;
}


void
Amiga::computeFrame()
{
    while (1) {

        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {

            // Are we requested to take a snapshot?
            /*
            if (flags & RL::AUTO_SNAPSHOT) {
                clearFlag(RL::AUTO_SNAPSHOT);
                takeAutoSnapshot();
            }

            if (flags & RL::USER_SNAPSHOT) {
                clearFlag(RL::USER_SNAPSHOT);
                takeUserSnapshot();
            }
            */

            // Did we reach a soft breakpoint?
            if (flags & RL::SOFTSTOP_REACHED) {
                clearFlag(RL::SOFTSTOP_REACHED);
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {
                clearFlag(RL::BREAKPOINT_REACHED);
                auto addr = cpu.debugger.breakpoints.hit->addr;
                msgQueue.put(MSG_BREAKPOINT_REACHED, CpuMsg { addr, 0});
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {
                clearFlag(RL::WATCHPOINT_REACHED);
                auto addr = cpu.debugger.watchpoints.hit->addr;
                msgQueue.put(MSG_WATCHPOINT_REACHED, CpuMsg {addr, 0});
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a catchpoint?
            if (flags & RL::CATCHPOINT_REACHED) {
                clearFlag(RL::CATCHPOINT_REACHED);
                auto vector = u8(cpu.debugger.catchpoints.hit->addr);
                msgQueue.put(MSG_CATCHPOINT_REACHED, CpuMsg {cpu.getPC0(), vector});
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a software trap?
            if (flags & RL::SWTRAP_REACHED) {
                clearFlag(RL::SWTRAP_REACHED);
                msgQueue.put(MSG_SWTRAP_REACHED, CpuMsg {cpu.getPC0(), 0});
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a Copper breakpoint?
            if (flags & RL::COPPERBP_REACHED) {
                clearFlag(RL::COPPERBP_REACHED);
                auto addr = u8(agnus.copper.debugger.breakpoints.hit->addr);
                msgQueue.put(MSG_COPPERBP_REACHED, CpuMsg { addr, 0 });
                throw StateChangeException(STATE_PAUSED);
                break;
            }

            // Did we reach a Copper watchpoint?
            if (flags & RL::COPPERWP_REACHED) {
                clearFlag(RL::COPPERWP_REACHED);
                auto addr = u8(agnus.copper.debugger.watchpoints.hit->addr);
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

Snapshot *
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
        autoSnapshot = new Snapshot(*this);
        msgQueue.put(MSG_SNAPSHOT_TAKEN, SnapshotMsg { .snapshot = autoSnapshot } );
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
Amiga::loadSnapshot(const Snapshot &snapshot)
{
    // bool wasPAL, isPAL;

    {   SUSPENDED

        // wasPAL = agnus.isPAL();

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

        // isPAL = agnus.isPAL();
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

void
Amiga::setDebugVariable(const string &name, int val)
{
#ifdef RELEASEBUILD

    throw Error(ERROR_OPT_UNSUPPORTED, "Debug variables can only be altered in debug builds.");

#else

    if      (name == "XFILES")           XFILES          = val;
    else if (name == "CNF_DEBUG")        CNF_DEBUG       = val;
    else if (name == "OBJ_DEBUG")        OBJ_DEBUG       = val;
    else if (name == "DEF_DEBUG")        DEF_DEBUG       = val;
    else if (name == "MIMIC_UAE")        MIMIC_UAE       = val;

    else if (name == "RUN_DEBUG")        RUN_DEBUG       = val;
    else if (name == "TIM_DEBUG")        TIM_DEBUG       = val;
    else if (name == "WARP_DEBUG")       WARP_DEBUG      = val;
    else if (name == "QUEUE_DEBUG")      QUEUE_DEBUG     = val;
    else if (name == "SNP_DEBUG")        SNP_DEBUG       = val;

    else if (name == "CPU_DEBUG")        CPU_DEBUG       = val;
    else if (name == "CST_DEBUG")        CST_DEBUG       = val;

    else if (name == "OCSREG_DEBUG")     OCSREG_DEBUG    = val;
    else if (name == "ECSREG_DEBUG")     ECSREG_DEBUG    = val;
    else if (name == "INVREG_DEBUG")     INVREG_DEBUG    = val;
    else if (name == "MEM_DEBUG")        MEM_DEBUG       = val;

    else if (name == "DMA_DEBUG")        DMA_DEBUG       = val;
    else if (name == "DDF_DEBUG")        DDF_DEBUG       = val;
    else if (name == "SEQ_DEBUG")        SEQ_DEBUG       = val;
    else if (name == "NTSC_DEBUG")       NTSC_DEBUG      = val;

    else if (name == "COP_CHECKSUM")     COP_CHECKSUM    = val;
    else if (name == "COPREG_DEBUG")     COPREG_DEBUG    = val;
    else if (name == "COP_DEBUG")        COP_DEBUG       = val;

    else if (name == "BLT_CHECKSUM")     BLT_CHECKSUM    = val;
    else if (name == "BLTREG_DEBUG")     BLTREG_DEBUG    = val;
    else if (name == "BLT_REG_GUARD")    BLT_REG_GUARD   = val;
    else if (name == "BLT_MEM_GUARD")    BLT_MEM_GUARD   = val;
    else if (name == "BLT_DEBUG")        BLT_DEBUG       = val;
    else if (name == "BLTTIM_DEBUG")     BLTTIM_DEBUG    = val;
    else if (name == "SLOW_BLT_DEBUG")   SLOW_BLT_DEBUG  = val;
    else if (name == "OLD_LINE_BLIT")    OLD_LINE_BLIT   = val;

    else if (name == "BPLREG_DEBUG")     BPLREG_DEBUG    = val;
    else if (name == "BPLDAT_DEBUG")     BPLDAT_DEBUG    = val;
    else if (name == "BPLMOD_DEBUG")     BPLMOD_DEBUG    = val;
    else if (name == "SPRREG_DEBUG")     SPRREG_DEBUG    = val;
    else if (name == "COLREG_DEBUG")     COLREG_DEBUG    = val;
    else if (name == "CLXREG_DEBUG")     CLXREG_DEBUG    = val;
    else if (name == "BPL_DEBUG")        BPL_DEBUG       = val;
    else if (name == "DIW_DEBUG")        DIW_DEBUG       = val;
    else if (name == "SPR_DEBUG")        SPR_DEBUG       = val;
    else if (name == "CLX_DEBUG")        CLX_DEBUG       = val;
    else if (name == "BORDER_DEBUG")     BORDER_DEBUG    = val;

    else if (name == "INTREG_DEBUG")     INTREG_DEBUG    = val;
    else if (name == "INT_DEBUG")        INT_DEBUG       = val;

    else if (name == "CIAREG_DEBUG")     CIAREG_DEBUG    = val;
    else if (name == "CIASER_DEBUG")     CIASER_DEBUG    = val;
    else if (name == "CIA_DEBUG")        CIA_DEBUG       = val;
    else if (name == "TOD_DEBUG")        TOD_DEBUG       = val;

    else if (name == "ALIGN_HEAD")       ALIGN_HEAD      = val;
    else if (name == "DSK_CHECKSUM")     DSK_CHECKSUM    = val;
    else if (name == "DSKREG_DEBUG")     DSKREG_DEBUG    = val;
    else if (name == "DSK_DEBUG")        DSK_DEBUG       = val;
    else if (name == "MFM_DEBUG")        MFM_DEBUG       = val;
    else if (name == "FS_DEBUG")         FS_DEBUG        = val;

    else if (name == "HDR_ACCEPT_ALL")   HDR_ACCEPT_ALL  = val;
    else if (name == "HDR_FS_LOAD_ALL")  HDR_FS_LOAD_ALL = val;
    else if (name == "WT_DEBUG")         WT_DEBUG        = val;

    else if (name == "AUDREG_DEBUG")     AUDREG_DEBUG    = val;
    else if (name == "AUD_DEBUG")        AUD_DEBUG       = val;
    else if (name == "AUDBUF_DEBUG")     AUDBUF_DEBUG    = val;
    else if (name == "DISABLE_AUDIRQ")   DISABLE_AUDIRQ  = val;

    else if (name == "POSREG_DEBUG")     POSREG_DEBUG    = val;
    else if (name == "JOYREG_DEBUG")     JOYREG_DEBUG    = val;
    else if (name == "POTREG_DEBUG")     POTREG_DEBUG    = val;
    else if (name == "PRT_DEBUG")        PRT_DEBUG       = val;
    else if (name == "SER_DEBUG")        SER_DEBUG       = val;
    else if (name == "POT_DEBUG")        POT_DEBUG       = val;
    else if (name == "HOLD_MOUSE_L")     HOLD_MOUSE_L    = val;
    else if (name == "HOLD_MOUSE_M")     HOLD_MOUSE_M    = val;
    else if (name == "HOLD_MOUSE_R")     HOLD_MOUSE_R    = val;

    else if (name == "ZOR_DEBUG")        ZOR_DEBUG       = val;
    else if (name == "ACF_DEBUG")        ACF_DEBUG       = val;
    else if (name == "FAS_DEBUG")        FAS_DEBUG       = val;
    else if (name == "HDR_DEBUG")        HDR_DEBUG       = val;
    else if (name == "DBD_DEBUG")        DBD_DEBUG       = val;

    else if (name == "ADF_DEBUG")        ADF_DEBUG       = val;
    else if (name == "DMS_DEBUG")        DMS_DEBUG       = val;
    else if (name == "IMG_DEBUG")        IMG_DEBUG       = val;

    else if (name == "RTC_DEBUG")        RTC_DEBUG       = val;
    else if (name == "KBD_DEBUG")        KBD_DEBUG       = val;

    else if (name == "REC_DEBUG")        REC_DEBUG       = val;
    else if (name == "SCK_DEBUG")        SCK_DEBUG       = val;
    else if (name == "SRV_DEBUG")        SRV_DEBUG       = val;
    else if (name == "GDB_DEBUG")        GDB_DEBUG       = val;

    else {

        throw Error(ERROR_OPT_UNSUPPORTED, "Unknown debug variable: " + name);
    }

#endif
}

}
