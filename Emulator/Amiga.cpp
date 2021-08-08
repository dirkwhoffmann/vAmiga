// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include "Snapshot.h"
#include "ADFFile.h"

// Perform some consistency checks
static_assert(sizeof(i8) == 1,  "i8 size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8) == 1,  "u8 size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");


//
// Amiga Class
//

Amiga::Amiga()
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
     * - Memory mus preceed the CPU, because it contains the CPU reset vector.
     */

    subComponents = std::vector<AmigaComponent *> {

        &oscillator,
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
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
        &msgQueue
    };

    // Set up the initial state
    initialize();
    hardReset();
        
    // Print some debug information
    if (SNP_DEBUG) {
        
        msg("             Agnus : %zu bytes\n", sizeof(Agnus));
        msg("       AudioFilter : %zu bytes\n", sizeof(AudioFilter));
        // msg("       AudioStream : %zu bytes\n", sizeof(AudioStream));
        msg("               CIA : %zu bytes\n", sizeof(CIA));
        msg("       ControlPort : %zu bytes\n", sizeof(ControlPort));
        msg("               CPU : %zu bytes\n", sizeof(CPU));
        msg("            Denise : %zu bytes\n", sizeof(Denise));
        msg("             Drive : %zu bytes\n", sizeof(Drive));
        msg("          Keyboard : %zu bytes\n", sizeof(Keyboard));
        msg("            Memory : %zu bytes\n", sizeof(Memory));
        msg("moira::Breakpoints : %zu bytes\n", sizeof(moira::Breakpoints));
        msg("moira::Watchpoints : %zu bytes\n", sizeof(moira::Watchpoints));
        msg("   moira::Debugger : %zu bytes\n", sizeof(moira::Debugger));
        msg("      moira::Moira : %zu bytes\n", sizeof(moira::Moira));
        msg("             Muxer : %zu bytes\n", sizeof(Muxer));
        msg("        Oscillator : %zu bytes\n", sizeof(Oscillator));
        msg("             Paula : %zu bytes\n", sizeof(Paula));
        msg("       PixelEngine : %zu bytes\n", sizeof(PixelEngine));
        msg("               RTC : %zu bytes\n", sizeof(RTC));
        msg("           Sampler : %zu bytes\n", sizeof(Sampler));
        msg("    ScreenRecorder : %zu bytes\n", sizeof(Recorder));
        msg("        SerialPort : %zu bytes\n", sizeof(SerialPort));
        msg("            Volume : %zu bytes\n", sizeof(Volume));
        msg("             Zorro : %zu bytes\n", sizeof(ZorroManager));
        msg("\n");
    }    
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying Amiga[%p]\n", this);
}

void
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            agnus.frame.nr, agnus.pos.v, agnus.pos.h);

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
        fprintf(stderr, "[%06X] ", agnus.copper.getCopPC());
    }
}

void
Amiga::reset(bool hard)
{
    if (hard) suspend();
    
    // If a disk change is in progress, finish it
    paula.diskController.serviceDiskChangeEvent();
    
    // Execute the standard reset routine
    AmigaComponent::reset(hard);
    
    if (hard) resume();

    // Inform the GUI
    if (hard) msgQueue.put(MSG_RESET);
}

void
Amiga::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Clear all runloop flags
    flags = 0;
}

i64
Amiga::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_AGNUS_REVISION:
        case OPT_SLOW_RAM_MIRROR:
            return agnus.getConfigItem(option);
            
        case OPT_DENISE_REVISION:
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:
            return denise.getConfigItem(option);
            
        case OPT_PALETTE:
        case OPT_BRIGHTNESS:
        case OPT_CONTRAST:
        case OPT_SATURATION:
            return denise.pixelEngine.getConfigItem(option);
            
        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_MODE:
        case OPT_DMA_DEBUG_OPACITY:
            return agnus.dmaDebugger.getConfigItem(option);
            
        case OPT_RTC_MODEL:
            return rtc.getConfigItem(option);

        case OPT_CHIP_RAM:
        case OPT_SLOW_RAM:
        case OPT_FAST_RAM:
        case OPT_EXT_START:
        case OPT_SLOW_RAM_DELAY:
        case OPT_BANKMAP:
        case OPT_UNMAPPING_TYPE:
        case OPT_RAM_INIT_PATTERN:
            return mem.getConfigItem(option);
            
        case OPT_SAMPLING_METHOD:
        case OPT_FILTER_TYPE:
        case OPT_FILTER_ALWAYS_ON:
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
            return paula.muxer.getConfigItem(option);

        case OPT_BLITTER_ACCURACY:
            return agnus.blitter.getConfigItem(option);

        case OPT_DRIVE_SPEED:
        case OPT_LOCK_DSKSYNC:
        case OPT_AUTO_DSKSYNC:
            return paula.diskController.getConfigItem(option);
            
        case OPT_SERIAL_DEVICE:
            return serialPort.getConfigItem(option);

        case OPT_CIA_REVISION: 
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:
            return ciaA.getConfigItem(option);

        case OPT_ACCURATE_KEYBOARD:
            return keyboard.getConfigItem(option);

        default: assert(false); return 0;
    }
}

i64
Amiga::getConfigItem(Option option, long id) const
{
    switch (option) {
            
        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_COLOR:
            return agnus.dmaDebugger.getConfigItem(option, id);

        case OPT_AUDPAN:
        case OPT_AUDVOL:
            return paula.muxer.getConfigItem(option, id);

        case OPT_DRIVE_CONNECT:
            return paula.diskController.getConfigItem(option, id);
            
        case OPT_DRIVE_TYPE:
        case OPT_EMULATE_MECHANICS:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:
            return df[id]->getConfigItem(option);
            
        case OPT_DEFAULT_FILESYSTEM:
        case OPT_DEFAULT_BOOTBLOCK:
            return df[id]->getConfigItem(option);
            
        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:
            if (id == PORT_1) return controlPort1.mouse.getConfigItem(option);
            if (id == PORT_2) return controlPort2.mouse.getConfigItem(option);
            assert(false);
            
        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:
            if (id == PORT_1) return controlPort1.joystick.getConfigItem(option);
            if (id == PORT_2) return controlPort2.joystick.getConfigItem(option);
            assert(false);

        default: assert(false);
    }
    
    return 0;
}

void
Amiga::configure(Option option, i64 value)
{
    _configure(option, value);
    msgQueue.put(MSG_CONFIG);
}

void
Amiga::_configure(Option option, i64 value)
{
    debug(CNF_DEBUG, "configure(%lld, %lld)\n", option, value);

    // Check if this option has been locked for debugging
    static std::map<Option,i64> overrides = OVERRIDES;
    if (overrides.find(option) != overrides.end()) {

        msg("Overriding option: %s = %lld\n", OptionEnum::key(option), value);
        value = overrides[option];
    }
    
    // Propagate configuration request to all components
    AmigaComponent::configure(option, value);
}

void
Amiga::configure(Option option, long id, i64 value)
{
    _configure(option, id, value);
    msgQueue.put(MSG_CONFIG);
}

void
Amiga::_configure(Option option, long id, i64 value)
{
    debug(CNF_DEBUG, "configure(%lld, %ld, %lld)\n", option, id, value);
    
    // Propagate configuration request to all components
    AmigaComponent::configure(option, id, value);
}

void
Amiga::configure(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);
    debug(CNF_DEBUG, "Using ConfigScheme %s", ConfigSchemeEnum::key(scheme));
    
    // Switch the Amiga off
    powerOff();

    // Revert to the initial state
    initialize();
    
    // Apply the selected scheme
    switch(scheme) {
            
        case CONFIG_A500_OCS_1MB:
            
            configure(OPT_CHIP_RAM, 512);
            configure(OPT_SLOW_RAM, 512);
            configure(OPT_AGNUS_REVISION, AGNUS_OCS);
            break;
            
        case CONFIG_A500_ECS_1MB:
            
            configure(OPT_CHIP_RAM, 512);
            configure(OPT_SLOW_RAM, 512);
            configure(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
            break;
            
        default:
            assert(false);
    }    
}

EventID
Amiga::getInspectionTarget() const
{
    return agnus.slot[SLOT_INS].id;
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    agnus.scheduleRel<SLOT_INS>(0, id);
    agnus.serviceINSEvent();
    resume();
}

void
Amiga::setInspectionTarget(EventID id, Cycle trigger)
{
    suspend();
    agnus.scheduleRel<SLOT_INS>(trigger, id);
    resume();
}

void
Amiga::removeInspectionTarget()
{
    suspend();
    agnus.cancel<SLOT_INS>();
    resume();
}

void
Amiga::_inspect()
{
    synchronized {
        
        info.cpuClock = cpu.getMasterClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaA.clock;
        info.ciaBClock = ciaB.clock;
        info.frame = agnus.frame.nr;
        info.vpos = agnus.pos.v;
        info.hpos = agnus.pos.h;
    }
}

void
Amiga::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
    
        if (CNF_DEBUG) {
            
            df0.dump(dump::Config);
            paula.dump(dump::Config);
            paula.muxer.dump(dump::Config);
            ciaA.dump(dump::Config);
            denise.dump(dump::Config);
        }
    }
    
    if (category & dump::State) {
        
        os << tab("Power");
        os << bol(isPoweredOn()) << std::endl;
        os << tab("Running");
        os << bol(isRunning()) << std::endl;
        os << tab("Warp");
        os << bol(warp) << std::endl;
    }
}

bool
Amiga::readyToPowerOn()
{
    debug(RUN_DEBUG, "readyToPowerOn()\n");
    
    try { isReady(); return true; } catch (...) { return false; }
}

void
Amiga::threadPowerOff()
{
    debug(RUN_DEBUG, "threadPowerOff()\n");
    
    // Power off all subcomponents
    AmigaComponent::powerOff();

    // Update the recorded debug information
    inspect();
    
    // Inform the GUI
    msgQueue.put(MSG_POWER_OFF);
}

void
Amiga::threadPowerOn()
{
    debug(RUN_DEBUG, "threadPowerOn()\n");
    
    // Perform a reset
    hardReset();
            
    // Power on all subcomponents
    AmigaComponent::powerOn();
    
    // Update the recorded debug information
    inspect();

#ifdef DF0_DISK
    DiskFile *df0file = AmigaFile::make <ADFFile> (DF0_DISK);
    if (df0file) {
        Disk *disk = Disk::makeWithFile(df0file);
        df0.ejectDisk();
        df0.insertDisk(disk);
        df0.setWriteProtection(false);
    }
#endif
    
#ifdef DF1_DISK
    DiskFile *df1file = DiskFile::makeWithFile(DF1_DISK);
    if (df1file) {
        Disk *disk = Disk::makeWithFile(df1file);
        df1.ejectDisk();
        df1.insertDisk(disk);
        df1.setWriteProtection(false);
    }
#endif
    
#ifdef INITIAL_BREAKPOINT
    debugMode = true;
    cpu.debugger.breakpoints.addAt(INITIAL_BREAKPOINT);
#endif
    
    // Inform the GUI
    msgQueue.put(MSG_POWER_ON);
}

void
Amiga::threadRun()
{
    debug(RUN_DEBUG, "threadRun()\n");
    
    // Launch all subcomponents
    AmigaComponent::run();
    
    // Enable or disable CPU debugging
    debugMode ? cpu.debugger.enableLogging() : cpu.debugger.disableLogging();

    // Inform the GUI
    msgQueue.put(MSG_RUN);
}

void
Amiga::threadPause()
{
    debug(RUN_DEBUG, "threadPause()\n");
        
    // Enter pause mode
    AmigaComponent::pause();
    
    // Update the recorded debug information
    inspect();
    
    // Inform the GUI
    msgQueue.put(MSG_PAUSE);
}

void
Amiga::threadHalt()
{
    debug(RUN_DEBUG, "threadHalt()\n");

    // Inform the GUI
    msgQueue.put(MSG_HALT);
}

void
Amiga::threadWarpOff()
{
    debug(WARP_DEBUG, "threadWarpOff()\n");
    AmigaComponent::warpOff();
    
    // Inform the GUI
    msgQueue.put(MSG_WARP_OFF);
}

void
Amiga::threadWarpOn()
{
    debug(WARP_DEBUG, "threadWarpOn()\n");
    AmigaComponent::warpOn();

    // Inform the GUI
    msgQueue.put(MSG_WARP_ON);
}

void
Amiga::threadExecute()
{
    // debug(RUN_DEBUG, "threadExecute()\n");
    
    while(1) {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {
            
            // Are we requested to take a snapshot?
            if (flags & RL::AUTO_SNAPSHOT) {
                clearControlFlag(RL::AUTO_SNAPSHOT);
                autoSnapshot = Snapshot::makeWithAmiga(this);
                msgQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
            }
            
            if (flags & RL::USER_SNAPSHOT) {
                clearControlFlag(RL::USER_SNAPSHOT);
                userSnapshot = Snapshot::makeWithAmiga(this);
                msgQueue.put(MSG_USER_SNAPSHOT_TAKEN);
            }

            // Are we requested to update the debugger info structs?
            if (flags & RL::INSPECT) {
                clearControlFlag(RL::INSPECT);
                inspect();
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {
                clearControlFlag(RL::BREAKPOINT_REACHED);
                inspect();
                msgQueue.put(MSG_BREAKPOINT_REACHED);
                newState = EXEC_PAUSED;
                break;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {
                clearControlFlag(RL::WATCHPOINT_REACHED);
                inspect();
                msgQueue.put(MSG_WATCHPOINT_REACHED);
                newState = EXEC_PAUSED;
                break;
            }

            // Are we requested to terminate the run loop?
            if (flags & RL::STOP) {
                clearControlFlag(RL::STOP);
                newState = EXEC_PAUSED;
                break;
            }

            // Are we requested to enter or exit warp mode?
            if (flags & RL::WARP_ON) {
                clearControlFlag(RL::WARP_ON);
                AmigaComponent::warpOn();
            }

            if (flags & RL::WARP_OFF) {
                clearControlFlag(RL::WARP_OFF);
                AmigaComponent::warpOff();
            }
            
            // Are we requested to synchronize the thread?
            if (flags & RL::SYNC_THREAD) {
                clearControlFlag(RL::SYNC_THREAD);
                break;
            }
        }
    }
}

void
Amiga::debugOn()
{
    assert(!isEmulatorThread());

    suspend();
    AmigaComponent::debugOn();
    resume();
}

void
Amiga::debugOff()
{
    assert(!isEmulatorThread());
    
    suspend();
    AmigaComponent::debugOff();
    resume();
}

void
Amiga::isReady()
{
    if (!mem.hasRom()) {
        msg("isReady: No Boot Rom or Kickstart Rom found\n");
        throw VAError(ERROR_ROM_MISSING);
    }

    if (!mem.hasChipRam()) {
        msg("isReady: No Chip Ram found\n");
        throw VAError(ERROR_CHIP_RAM_MISSING);
    }
    
    if (mem.hasArosRom()) {

        if (!mem.hasExt()) {
            msg("isReady: Aros requires an extension Rom\n");
            throw VAError(ERROR_AROS_NO_EXTROM);
        }

        if (mem.ramSize() < MB(1)) {
            msg("isReady: Aros requires at least 1 MB of memory\n");
            throw VAError(ERROR_AROS_RAM_LIMIT);
        }
    }

    if (mem.chipRamSize() > KB(agnus.chipRamLimit())) {
        msg("isReady: Chip Ram exceeds Agnus limit\n");
        throw VAError(ERROR_CHIP_RAM_LIMIT);
    }
}

void
Amiga::suspend()
{
    debug(RUN_DEBUG, "Suspending (%zu)...\n", suspendCounter);
    
    if (suspendCounter || isRunning()) {
        pause();
        suspendCounter++;
    }
}

void
Amiga::resume()
{
    debug(RUN_DEBUG, "Resuming (%zu)...\n", suspendCounter);
    
    if (suspendCounter && --suspendCounter == 0) {
        run();
    }
}

void
Amiga::setControlFlag(u32 flag)
{
    synchronized { flags |= flag; }
}

void
Amiga::clearControlFlag(u32 flag)
{
    synchronized { flags &= ~flag; }
}

void
Amiga::stopAndGo()
{
    isRunning() ? pause() : run();
}

void
Amiga::stepInto()
{
    if (isRunning()) return;

    cpu.debugger.stepInto();
    run();
}

void
Amiga::stepOver()
{
    if (isRunning()) return;
    
    cpu.debugger.stepOver();
    run();
}

void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        autoSnapshot = Snapshot::makeWithAmiga(this);
        msgQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
        
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
        userSnapshot = Snapshot::makeWithAmiga(this);
        msgQueue.put(MSG_USER_SNAPSHOT_TAKEN);
        
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

void
Amiga::loadFromSnapshotUnsafe(Snapshot *snapshot)
{
    u8 *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        load(ptr);
        msgQueue.put(MSG_SNAPSHOT_RESTORED);
    }
}

void
Amiga::loadFromSnapshotSafe(Snapshot *snapshot)
{
    trace(SNP_DEBUG, "loadFromSnapshotSafe\n");
    
    suspend();
    loadFromSnapshotUnsafe(snapshot);
    resume();
}
