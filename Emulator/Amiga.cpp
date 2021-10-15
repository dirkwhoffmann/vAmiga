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
#include <algorithm>

// Perform some consistency checks
static_assert(sizeof(i8) == 1,  "i8 size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8) == 1,  "u8 size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");

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
     * - Memory must preceed the CPU, because it contains the CPU reset vector.
     */
    
    subComponents = std::vector<AmigaComponent *> {

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
        &retroShell,
        &regressionTester,
        &msgQueue
    };

    // Set up the initial state
    initialize();
    hardReset();
        
    // Print some debug information
    if constexpr (SNP_DEBUG) {
        
        msg("             Agnus : %zu bytes\n", sizeof(Agnus));
        msg("       AudioFilter : %zu bytes\n", sizeof(AudioFilter));
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
    debug(RUN_DEBUG, "Destroying Amiga\n");
}

void
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3zd,%3zd) ",
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

        default:
            fatalError;
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
        case OPT_START_DELAY:
        case OPT_STOP_DELAY:
        case OPT_STEP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:
        case OPT_DEFAULT_FILESYSTEM:
        case OPT_DEFAULT_BOOTBLOCK:
            
            return df[id]->getConfigItem(option);
            
        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:
            
            if (id == PORT_1) return controlPort1.mouse.getConfigItem(option);
            if (id == PORT_2) return controlPort2.mouse.getConfigItem(option);
            fatalError;
            
        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:
            
            if (id == PORT_1) return controlPort1.joystick.getConfigItem(option);
            if (id == PORT_2) return controlPort2.joystick.getConfigItem(option);
            fatalError;

        default:
            fatalError;
    }
    
    return 0;
}

void
Amiga::configure(Option option, i64 value)
{
    debug(CNF_DEBUG, "configure(%lld, %lld)\n", option, value);

    // The following options do not send a message to the GUI
    static std::vector<Option> quiet = {
        
        OPT_HIDDEN_LAYER_ALPHA,
        OPT_BRIGHTNESS,
        OPT_CONTRAST,
        OPT_SATURATION,
        OPT_DRIVE_PAN,
        OPT_STEP_VOLUME,
        OPT_POLL_VOLUME,
        OPT_INSERT_VOLUME,
        OPT_EJECT_VOLUME,
        OPT_AUDVOLL,
        OPT_AUDVOLR,
        OPT_AUDPAN,
        OPT_AUDVOL
    };
    
    // Check if this option has been locked for debugging
    value = overrideOption(option, value);

    switch (option) {

        case OPT_AGNUS_REVISION:
        case OPT_SLOW_RAM_MIRROR:
            
            agnus.setConfigItem(option, value);
            break;
            
        case OPT_DENISE_REVISION:
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:
            
            denise.setConfigItem(option, value);
            break;

        case OPT_PALETTE:
        case OPT_BRIGHTNESS:
        case OPT_CONTRAST:
        case OPT_SATURATION:
            
            denise.pixelEngine.setConfigItem(option, value);
            break;

        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_MODE:
        case OPT_DMA_DEBUG_OPACITY:
            
            agnus.dmaDebugger.setConfigItem(option, value);
            break;

        case OPT_RTC_MODEL:
            
            rtc.setConfigItem(option, value);
            break;

        case OPT_CHIP_RAM:
        case OPT_SLOW_RAM:
        case OPT_FAST_RAM:
        case OPT_EXT_START:
        case OPT_SLOW_RAM_DELAY:
        case OPT_BANKMAP:
        case OPT_UNMAPPING_TYPE:
        case OPT_RAM_INIT_PATTERN:
            
            mem.setConfigItem(option, value);
            break;

        case OPT_DRIVE_TYPE:
        case OPT_EMULATE_MECHANICS:
        case OPT_START_DELAY:
        case OPT_STOP_DELAY:
        case OPT_STEP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:
        case OPT_DEFAULT_FILESYSTEM:
        case OPT_DEFAULT_BOOTBLOCK:
            
            df[0]->setConfigItem(option, value);
            df[1]->setConfigItem(option, value);
            df[2]->setConfigItem(option, value);
            df[3]->setConfigItem(option, value);
            break;
            
        case OPT_SAMPLING_METHOD:
        case OPT_FILTER_TYPE:
        case OPT_FILTER_ALWAYS_ON:
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
            
            paula.muxer.setConfigItem(option, value);
            break;

        case OPT_AUDPAN:
        case OPT_AUDVOL:
            
            paula.muxer.setConfigItem(option, 0, value);
            paula.muxer.setConfigItem(option, 1, value);
            paula.muxer.setConfigItem(option, 2, value);
            paula.muxer.setConfigItem(option, 3, value);
            break;

        case OPT_BLITTER_ACCURACY:
            
            agnus.blitter.setConfigItem(option, value);
            break;

        case OPT_DRIVE_SPEED:
        case OPT_LOCK_DSKSYNC:
        case OPT_AUTO_DSKSYNC:
            
            paula.diskController.setConfigItem(option, value);
            break;

        case OPT_SERIAL_DEVICE:
            
            serialPort.setConfigItem(option, value);
            break;

        case OPT_CIA_REVISION:
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:
            
            ciaA.setConfigItem(option, value);
            ciaB.setConfigItem(option, value);
            break;

        case OPT_ACCURATE_KEYBOARD:
            
            keyboard.setConfigItem(option, value);
            break;

        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:
            
            controlPort1.mouse.setConfigItem(option, value);
            controlPort2.mouse.setConfigItem(option, value);
            break;
            
        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:
            
            controlPort1.joystick.setConfigItem(option, value);
            controlPort2.joystick.setConfigItem(option, value);
            break;

        default:
            fatalError;
    }

    if (std::find(quiet.begin(), quiet.end(), option) == quiet.end()) {
        msgQueue.put(MSG_CONFIG, option);
    }
}

void
Amiga::configure(Option option, long id, i64 value)
{
    debug(CNF_DEBUG, "configure(%lld, %ld, %lld)\n", option, id, value);

    // Check if this option has been locked for debugging
    value = overrideOption(option, value);

    // The following options do not send a message to the GUI
    static std::vector<Option> quiet = {
        
        OPT_DRIVE_PAN,
        OPT_STEP_VOLUME,
        OPT_POLL_VOLUME,
        OPT_INSERT_VOLUME,
        OPT_EJECT_VOLUME,
        OPT_AUDVOLL,
        OPT_AUDVOLR,
        OPT_AUDPAN,
        OPT_AUDVOL,
        OPT_MOUSE_VELOCITY
    };
    
    switch (option) {
            
        case OPT_DMA_DEBUG_ENABLE:
        case OPT_DMA_DEBUG_COLOR:
            
            agnus.dmaDebugger.setConfigItem(option, id, value);
            break;

        case OPT_AUDPAN:
        case OPT_AUDVOL:
            
            paula.muxer.setConfigItem(option, id, value);
            break;

        case OPT_DRIVE_CONNECT:
            
            paula.diskController.setConfigItem(option, id, value);
            break;

        case OPT_DRIVE_TYPE:
        case OPT_EMULATE_MECHANICS:
        case OPT_START_DELAY:
        case OPT_STOP_DELAY:
        case OPT_STEP_DELAY:
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:
        case OPT_DEFAULT_FILESYSTEM:
        case OPT_DEFAULT_BOOTBLOCK:
            
            df[id]->setConfigItem(option, value);
            break;

        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:
            
            if (id == PORT_1) controlPort1.mouse.setConfigItem(option, value);
            if (id == PORT_2) controlPort2.mouse.setConfigItem(option, value);
            break;
            
        case OPT_AUTOFIRE:
        case OPT_AUTOFIRE_BULLETS:
        case OPT_AUTOFIRE_DELAY:
            
            if (id == PORT_1) controlPort1.joystick.setConfigItem(option, value);
            if (id == PORT_2) controlPort2.joystick.setConfigItem(option, value);
            break;

        default:
            fatalError;
    }
    
    if (std::find(quiet.begin(), quiet.end(), option) == quiet.end()) {
        msgQueue.put(MSG_CONFIG, option);
    }
}

void
Amiga::configure(ConfigScheme scheme)
{
    assert_enum(ConfigScheme, scheme);

    suspended {
        
        switch(scheme) {
                
            case CONFIG_A500_OCS_1MB:
                
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                configure(OPT_AGNUS_REVISION, AGNUS_OCS_PLCC);
                break;
                
            case CONFIG_A500_ECS_1MB:
                
                configure(OPT_CHIP_RAM, 512);
                configure(OPT_SLOW_RAM, 512);
                configure(OPT_AGNUS_REVISION, AGNUS_ECS_1MB);
                break;
                
            default:
                fatalError;
        }
    }
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
    switch(agnus.scheduler.id[SLOT_INS]) {
            
        case EVENT_NONE:  return INSPECTION_NONE;
        case INS_AMIGA:   return INSPECTION_AMIGA;
        case INS_CPU:     return INSPECTION_CPU;
        case INS_MEM:     return INSPECTION_MEM;
        case INS_CIA:     return INSPECTION_CIA;
        case INS_AGNUS:   return INSPECTION_AGNUS;
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
    
    suspended {
        
        switch(target) {
                
            case INSPECTION_NONE:    agnus.scheduler.cancel<SLOT_INS>(); return;
                
            case INSPECTION_AMIGA:   id = INS_AMIGA; break;
            case INSPECTION_CPU:     id = INS_CPU; break;
            case INSPECTION_MEM:     id = INS_MEM; break;
            case INSPECTION_CIA:     id = INS_CIA; break;
            case INSPECTION_AGNUS:   id = INS_AGNUS; break;
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

void
Amiga::_inspect() const
{
    synchronized {
        
        info.cpuClock = cpu.getMasterClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaA.getClock();
        info.ciaBClock = ciaB.getClock();
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
    
        if constexpr (CNF_DEBUG) {
            
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
        os << tab("Warp mode");
        os << bol(inWarpMode()) << std::endl;
        os << tab("Debug mode");
        os << bol(inDebugMode()) << std::endl;
    }
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "_powerOn\n");

    // Perform a reset
    hardReset();
                
    // Update the recorded debug information
    inspect();

#ifdef DF0_DISK
    ADFFile df0file(DF0_DISK);
    df0.ejectDisk();
    df0.insertDisk(std::make_unique<Disk>(df0file));
    df0.setWriteProtection(false);
#endif
    
#ifdef DF1_DISK
    ADFFile df1file(DF1_DISK);
    df1.ejectDisk();
    df1.insertDisk(std::make_unique<Disk>(df1file));
    df1.setWriteProtection(false);
#endif
    
#ifdef INITIAL_BREAKPOINT
    debugMode = true;
    cpu.debugger.breakpoints.addAt(INITIAL_BREAKPOINT);
#endif
    
    msgQueue.put(MSG_POWER_ON);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff\n");

    inspect();
    msgQueue.put(MSG_POWER_OFF);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run\n");

    // Enable or disable CPU debugging
    debugMode ? cpu.debugger.enableLogging() : cpu.debugger.disableLogging();

    msgQueue.put(MSG_RUN);
}

void
Amiga::_pause()
{
    debug(RUN_DEBUG, "_pause\n");

    inspect();
    msgQueue.put(MSG_PAUSE);
}

void
Amiga::_halt()
{
    debug(RUN_DEBUG, "_halt\n");

    msgQueue.put(MSG_HALT);
}

void
Amiga::_warpOn()
{
    debug(RUN_DEBUG, "_warpOn\n");

    msgQueue.put(MSG_WARP_ON);
}

void
Amiga::_warpOff()
{
    debug(RUN_DEBUG, "_warpOff\n");

    msgQueue.put(MSG_WARP_OFF);
}

void
Amiga::execute()
{    
    while(1) {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (flags) {
            
            // Are we requested to take a snapshot?
            if (flags & RL::AUTO_SNAPSHOT) {
                clearFlag(RL::AUTO_SNAPSHOT);
                autoSnapshot = new Snapshot(*this);
                msgQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
            }
            
            if (flags & RL::USER_SNAPSHOT) {
                clearFlag(RL::USER_SNAPSHOT);
                userSnapshot = new Snapshot(*this);
                msgQueue.put(MSG_USER_SNAPSHOT_TAKEN);
            }

            // Are we requested to update the debugger info structs?
            if (flags & RL::INSPECT) {
                clearFlag(RL::INSPECT);
                inspect();
            }

            // Did we reach a soft breakpoint?
            if (flags & RL::SOFTSTOP_REACHED) {
                clearFlag(RL::SOFTSTOP_REACHED);
                inspect();
                newState = EXEC_PAUSED;
                break;
            }

            // Did we reach a breakpoint?
            if (flags & RL::BREAKPOINT_REACHED) {
                clearFlag(RL::BREAKPOINT_REACHED);
                inspect();
                msgQueue.put(MSG_BREAKPOINT_REACHED, cpu.debugger.breakpointPC);
                newState = EXEC_PAUSED;
                break;
            }

            // Did we reach a watchpoint?
            if (flags & RL::WATCHPOINT_REACHED) {
                clearFlag(RL::WATCHPOINT_REACHED);
                inspect();
                msgQueue.put(MSG_WATCHPOINT_REACHED, cpu.debugger.watchpointPC);
                newState = EXEC_PAUSED;
                break;
            }

            // Are we requested to terminate the run loop?
            if (flags & RL::STOP) {
                clearFlag(RL::STOP);
                newState = EXEC_PAUSED;
                break;
            }

            // Are we requested to enter or exit warp mode?
            if (flags & RL::WARP_ON) {
                clearFlag(RL::WARP_ON);
                AmigaComponent::warpOn();
            }

            if (flags & RL::WARP_OFF) {
                clearFlag(RL::WARP_OFF);
                AmigaComponent::warpOff();
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
Amiga::setFlag(u32 flag)
{
    synchronized { flags |= flag; }
}

void
Amiga::clearFlag(u32 flag)
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
    
    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Amiga::stepOver()
{
    if (isRunning()) return;
    
    cpu.debugger.stepOver();
    run();
    
    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        autoSnapshot = new Snapshot(*this);
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
        userSnapshot = new Snapshot(*this);
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
Amiga::loadSnapshot(const Snapshot &snapshot)
{
    // Check if this snapshot is compatible with the emulator
    if (snapshot.isTooOld() || FORCE_SNAPSHOT_TOO_OLD) {
        throw VAError(ERROR_SNP_TOO_OLD);
    }
    if (snapshot.isTooNew() || FORCE_SNAPSHOT_TOO_NEW) {
        throw VAError(ERROR_SNP_TOO_NEW);
    }

    suspended {
        
        // Restore the saved state
        load(snapshot.getData());
                
        // Print some debug info if requested
        if constexpr (SNP_DEBUG) dump();
    }
    
    // Inform the GUI
    msgQueue.put(MSG_SNAPSHOT_RESTORED);
}
