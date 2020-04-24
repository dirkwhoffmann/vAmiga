// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

//
// Static class variables
//

bool Amiga::debugMode = false;
EventID Amiga::inspectionTarget = INS_NONE;


//
// Emulator thread
//

void
threadTerminated(void* thisAmiga)
{
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread has been canceled
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadDidTerminate();
}

void
*threadMain(void *thisAmiga) {
    
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread is about to start
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadWillStart();
    
    // Configure the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(threadTerminated, thisAmiga);
    
    // Enter the run loop
    amiga->runLoop();
    
    // Clean up and exit
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}


//
// Amiga Class
//

Amiga::Amiga()
{
    setDescription("Amiga");

    /* The order of sub-components is important here, because some components
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

    subComponents = vector<HardwareComponent *> {

        &agnus,
        &rtc,
        &denise,
        &paula,
        &zorro,
        &controlPort1,
        &controlPort2,
        &serialPort,
        &mouse,
        &joystick1,
        &joystick2,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
    };

    // Set up initial state
    initialize();
    reset();

    // Initialize the mach timer info
    mach_timebase_info(&tb);
}

Amiga::~Amiga()
{
    debug("Destroying Amiga[%p]\n", this);
    powerOff();
}

void
Amiga::setDebugMode(bool enable)
{
    if ((debugMode = enable)) {
        
        debug("Enabling debug mode\n");
        cpu.debugger.enableLogging();

    } else {

        debug("Disabling debug mode\n");
        cpu.debugger.disableLogging();
    }
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    inspectionTarget = id;
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    agnus.serviceINSEvent();
    resume();
}

void
Amiga::clearInspectionTarget()
{
    setInspectionTarget(INS_NONE);
}

AmigaConfiguration
Amiga::getConfig()
{
    AmigaConfiguration config;

    config.ciaA = ciaA.getConfig();
    config.ciaB = ciaB.getConfig();
    config.rtc = rtc.getConfig();
    config.audio = paula.audioUnit.getConfig();
    config.mem = mem.getConfig();
    config.agnus = agnus.getConfig();
    config.denise = denise.getConfig();
    config.serialPort = serialPort.getConfig();
    config.blitter = agnus.blitter.getConfig(); 
    config.diskController = paula.diskController.getConfig();
    config.df0 = df0.getConfig();
    config.df1 = df1.getConfig();
    config.df2 = df2.getConfig();
    config.df3 = df3.getConfig();

    // Assure both CIAs are configured equally
    assert(config.ciaA.type == config.ciaB.type);
    assert(config.ciaA.todBug == config.ciaB.todBug);

    return config;
}

AmigaStats
Amiga::getStats()
{
    AmigaStats result;

    pthread_mutex_lock(&lock);

    result = stats;
    clearStats();

    pthread_mutex_unlock(&lock);

    return result;
}

void
Amiga::updateStats()
{
    pthread_mutex_lock(&lock);

    stats.mem = mem.getStats();
    stats.agnus = agnus.getStats();
    stats.frames++;

    pthread_mutex_unlock(&lock);
}

void
Amiga::clearStats()
{
    // debug("Amiga::clearStats\n");

    memset(&stats, 0, sizeof(stats));
    mem.clearStats();
}

bool
Amiga::configure(ConfigOption option, long value)
{
    AmigaConfiguration current = getConfig();
    
    switch (option) {

        case VA_AGNUS_REVISION:

            if (!isAgnusRevision(value)) {
                warn("Invalid Agnus revision: %d\n", value);
                return false;
             }

            if (current.agnus.revision == value) return true;
            agnus.setRevision((AgnusRevision)value);
            break;

        case VA_DENISE_REVISION:

            if (!isDeniseRevision(value)) {
                warn("Invalid Denise revision: %d\n", value);
                return false;
            }

            if (current.denise.revision == value) return true;
            denise.setRevision((DeniseRevision)value);
            break;

        case VA_RT_CLOCK:

            if (!isRTCModel(value)) {
                warn("Invalid RTC model: %d\n", value);
                return false;
            }

            if (current.rtc.model == value) return true;
            rtc.setModel((RTCModel)value);
            mem.updateMemSrcTable();
            break;

        case VA_CHIP_RAM:
            
            if (value != 256 && value != 512 && value != 1024 && value != 2048) {
                warn("Invalid Chip Ram size: %d\n", value);
                warn("         Valid values: 256KB, 512KB, 1024KB, 2048KB\n");
                return false;
            }
            
            mem.allocChip(KB(value));
            break;
    
        case VA_SLOW_RAM:
            
            if ((value % 256) != 0 || value > 512) {
                warn("Invalid Slow Ram size: %d\n", value);
                warn("         Valid values: 0KB, 256KB, 512KB\n");
                return false;
            }
            
            mem.allocSlow(KB(value));
            break;
        
        case VA_FAST_RAM:
            
            if ((value % 64) != 0 || value > 8192) {
                warn("Invalid Fast Ram size: %d\n", value);
                warn("Valid values: 0KB, 64KB, 128KB, ..., 8192KB (8MB)\n");
                return false;
            }
            
            mem.allocFast(KB(value));
            break;

        case VA_EXT_START:

            if (value != 0xE0 && value != 0xF0) {
                warn("Invalid Extended ROM start page: %x\n", value);
                warn("Valid values: 0xE0, 0xF0\n");
                return false;
            }

            mem.setExtStart(value);
            break;

        case VA_DRIVE_SPEED:

            if (!isValidDriveSpeed(value)) {
                warn("Invalid drive speed: %d\n", value);
                return false;
            }

            paula.diskController.setSpeed(value);
            break;

        case VA_EMULATE_SPRITES:

            if (current.denise.emulateSprites == value) return true;
            denise.setEmulateSprites(value);
            break;

        case VA_CLX_SPR_SPR:

            if (current.denise.clxSprSpr == value) return true;
            denise.setClxSprSpr(value);
            break;

        case VA_CLX_SPR_PLF:

            if (current.denise.clxSprPlf == value) return true;
            denise.setClxSprPlf(value);
            break;

        case VA_CLX_PLF_PLF:

            if (current.denise.clxPlfPlf == value) return true;
            denise.setClxPlfPlf(value);
            break;

        case VA_SAMPLING_METHOD:

            if (!isSamplingMethod(value)) {
                warn("Invalid filter activation: %d\n", value);
                return false;
            }
            
            if (current.audio.samplingMethod == value) return true;
            paula.audioUnit.setSamplingMethod((SamplingMethod)value);
            break;

        case VA_FILTER_ACTIVATION:

            if (!isFilterActivation(value)) {
                warn("Invalid filter activation: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILTACT_COUNT - 1);
                return false;
            }

            if (current.audio.filterActivation == value) return true;
            paula.audioUnit.setFilterActivation((FilterActivation)value);
            break;

        case VA_FILTER_TYPE:

            if (!isFilterType(value)) {
                warn("Invalid filter type: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILT_COUNT - 1);
                return false;
            }

            if (current.audio.filterType == value) return true;
            paula.audioUnit.setFilterType((FilterType)value);
            break;

        case VA_BLITTER_ACCURACY:
            
            if (current.blitter.accuracy == value) return true;
            agnus.blitter.setAccuracy(value);
            break;
            
        case VA_FIFO_BUFFERING:

            if (current.diskController.useFifo == value) return true;
            paula.diskController.setUseFifo(value);
            break;

        case VA_SERIAL_DEVICE:

            if (!isSerialPortDevice(value)) {
                warn("Invalid serial port device: %d\n", value);
                return false;
            }

            if (current.serialPort.device == value) return true;
            serialPort.setDevice((SerialPortDevice)value);
            break;

        case VA_TODBUG:

            ciaA.setTodBug(value);
            ciaB.setTodBug(value);
            break;

        default: assert(false);
    }
    
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureDrive(unsigned drive, ConfigOption option, long value)
{
    if (drive >= 4) {
        warn("Invalid drive number: %d\n");
        return false;
    }
    
    DriveConfig current =
    drive == 0 ? getConfig().df0 :
    drive == 1 ? getConfig().df1 :
    drive == 2 ? getConfig().df2 : getConfig().df3;
    
    switch (option) {
            
        case VA_DRIVE_CONNECT:
            
            if (drive == 0 && value == false) {
                warn("Df0 cannot be disconnected. Ignoring.\n");
                return false;
            }

            if (getConfig().diskController.connected[drive] == value) return true;
            paula.diskController.setConnected(drive, value);
            break;
            
        case VA_DRIVE_TYPE:
            
            if (!isDriveType(value)) {
                warn("Invalid drive type: %d\n", value);
                return false;
            }
            
            if (value != DRIVE_35_DD) {
                warn("Unsupported drive type: %s\n", driveTypeName((DriveType)value));
                return false;
            }
            
            if (current.type == value) return true;
            df[drive]->setType((DriveType)value);
            break;

        default: assert(false);
    }
    
    putMessage(MSG_CONFIG);
    return true;
}

long
Amiga::getConfig(ConfigOption option)
{
    switch (option) {

        case VA_AGNUS_REVISION: return agnus.getRevision();
        case VA_DENISE_REVISION: return denise.getRevision();
        case VA_RT_CLOCK: return rtc.getModel();
        case VA_CHIP_RAM: return mem.chipRamSize() / KB(1);
        case VA_SLOW_RAM: return mem.slowRamSize() / KB(1);
        case VA_FAST_RAM: return mem.fastRamSize() / KB(1);
        case VA_EXT_START: return mem.getExtStart();
        case VA_DRIVE_SPEED: return df0.getSpeed();
        case VA_EMULATE_SPRITES: return denise.getEmulateSprites();
        case VA_CLX_SPR_SPR: return denise.getClxSprSpr();
        case VA_CLX_SPR_PLF: return denise.getClxSprPlf();
        case VA_CLX_PLF_PLF: return denise.getClxPlfPlf();
        case VA_SAMPLING_METHOD: return paula.audioUnit.getSamplingMethod();
        case VA_FILTER_ACTIVATION: return paula.audioUnit.getFilterActivation();
        case VA_FILTER_TYPE: return paula.audioUnit.getFilterType();
        case VA_BLITTER_ACCURACY: return agnus.blitter.getAccuracy();
        case VA_FIFO_BUFFERING: return paula.diskController.getUseFifo();
        case VA_SERIAL_DEVICE: return serialPort.getDevice();
        case VA_TODBUG: return ciaA.getTodBug();

        default: assert(false); return 0;
    }
}

long
Amiga::getDriveConfig(unsigned drive, ConfigOption option)
{
    assert(drive < 4);
            
    switch (option) {
            
        case VA_DRIVE_CONNECT: return paula.diskController.isConnected(drive);
        case VA_DRIVE_TYPE: return df[drive]->getType();

        default: assert(false);
    }
    
    return 0;
}

void
Amiga::prefix()
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            agnus.frame.nr, agnus.pos.v, agnus.pos.h);

    fprintf(stderr, "%06X ", cpu.getPC());
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
Amiga::_initialize()
{
}

void
Amiga::powerOn()
{
    if (readyToPowerOn() == ERR_OK) HardwareComponent::powerOn();
}

void
Amiga::powerOff()
{
    HardwareComponent::powerOff();
}

void
Amiga::run()
{
    if (readyToPowerOn() == ERR_OK) HardwareComponent::run();
}

void
Amiga::reset()
{
    suspend();

    assert(!isRunning());
    
    // Ask all components to finish ongoing activities
    finalize();
    
    // Execute the standard reset routine
    HardwareComponent::reset();

    // Discard all previously recorded stastical information
    clearStats();

    // Inform the GUI
    putMessage(MSG_RESET);

    resume();
}

void
Amiga::warpOn()
{
    suspend();
    HardwareComponent::warpOn();
    resume();
}

void
Amiga::warpOff()
{
    suspend();
    HardwareComponent::warpOff();
    resume();
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "Power on\n");

#ifdef BOOT_DISK

    ADFFile *adf = ADFFile::makeWithFile(BOOT_DISK);
    if (adf) {
        Disk *disk = Disk::makeWithFile(adf);
        df0.insertDisk(disk);
    }

#endif

#ifdef INITIAL_BREAKPOINT

    debugMode = true;
    cpu.debugger.breakpoints.addAt(INITIAL_BREAKPOINT);

#endif

    // Clear all runloop flags
    runLoopCtrl = 0;

    // Update the recorded debug information
    inspect();

    putMessage(MSG_POWER_ON);
}

void
Amiga::_powerOff()
{
    debug(1, "Power off\n");
    
    // Update the recorded debug information
    inspect();
    
    putMessage(MSG_POWER_OFF);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "Starting emulation thread (PC = %X)\n", cpu.getPC());

    // Start the emulator thread
    pthread_create(&p, NULL, threadMain, (void *)this);
    
    // Inform the GUI
    putMessage(MSG_RUN);
}

void
Amiga::_pause()
{
    // Cancel the emulator thread if it still running
    if (p) signalStop();
    
    // Wait until the thread has terminated
    pthread_join(p, NULL);
    
    // Update the recorded debug information
    inspect();

    // Inform the GUI
    putMessage(MSG_PAUSE);
}

void
Amiga::_ping()
{
    putMessage(warp ? MSG_WARP_ON : MSG_WARP_OFF);
}

void
Amiga::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.cpuClock = cpu.getMasterClock();
    info.dmaClock = agnus.clock;
    info.ciaAClock = ciaA.clock;
    info.ciaBClock = ciaB.clock;
    info.frame = agnus.frame.nr;
    info.vpos = agnus.pos.v;
    info.hpos = agnus.pos.h;
    
    pthread_mutex_unlock(&lock);
}

void
Amiga::_dump()
{
    AmigaConfiguration config = getConfig();
    DiskControllerConfig dc = config.diskController;

    dumpClock();
    msg("    poweredOn: %s\n", isPoweredOn() ? "yes" : "no");
    msg("   poweredOff: %s\n", isPoweredOff() ? "yes" : "no");
    msg("       paused: %s\n", isPaused() ? "yes" : "no");
    msg("      running: %s\n", isRunning() ? "yes" : "no");
    msg("\n");
    msg("Current configuration:\n\n");
    msg("          df0: %s %s\n",
             dc.connected[0] ? "yes" : "no", driveTypeName(config.df0.type));
    msg("          df1: %s %s\n",
             dc.connected[1] ? "yes" : "no", driveTypeName(config.df1.type));
    msg("          df2: %s %s\n",
             dc.connected[2] ? "yes" : "no", driveTypeName(config.df2.type));
    msg("          df3: %s %s\n",
             dc.connected[3] ? "yes" : "no", driveTypeName(config.df3.type));

    msg("\n");
    msg("         warp: %d", warp);
    msg("\n");
}

void
Amiga::_warpOn()
{
    putMessage(MSG_WARP_ON);
}

void
Amiga::_warpOff()
{
    restartTimer();
    putMessage(MSG_WARP_OFF);
}

void
Amiga::suspend()
{
    debug(RUN_DEBUG, "Suspending (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0 && !isRunning())
    return;
    
    pause();
    suspendCounter++;
}

void
Amiga::resume()
{
    debug(RUN_DEBUG, "Resuming (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0)
    return;
    
    if (--suspendCounter == 0)
    run();
}

ErrorCode
Amiga::readyToPowerOn()
{
    if (!mem.hasRom()) {
        msg("readyToPowerUp: No Boot Rom or Kickstart Rom found\n");
        return ERR_ROM_MISSING;
    }

    if (!mem.hasChipRam()) {
        msg("readyToPowerUp: No Chip Ram found\n");
        return ERR_ROM_MISSING;
    }
    
    if (mem.hasArosRom()) {

        if (!mem.hasExt()) {
            msg("readyToPowerUp: Aros requires an extension Rom\n");
            return ERR_AROS_NO_EXTROM;
        }

        if (mem.ramSize() < MB(1)) {
            msg("readyToPowerUp: Aros requires at least 1 MB of memory\n");
            return ERR_AROS_RAM_LIMIT;
        }
    }

    if (mem.chipRamSize() > KB(agnus.chipRamLimit())) {
        msg("readyToPowerUp: Chip Ram exceeds Agnus limit\n");
        return ERR_CHIP_RAM_LIMIT;
    }

    return ERR_OK;
}

void
Amiga::setControlFlags(u32 flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl |= flags;
    pthread_mutex_unlock(&lock);
}

void
Amiga::clearControlFlags(u32 flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl &= ~flags;
    pthread_mutex_unlock(&lock);
}

void
Amiga::restartTimer()
{
    timeBase = time_in_nanos();
    clockBase = agnus.clock;
}

void
Amiga::synchronizeTiming()
{
    u64 now          = time_in_nanos();
    Cycle clockDelta = agnus.clock - clockBase;
    i64 elapsedTime  = (clockDelta * 1000) / masterClockFrequency;
    i64 targetTime   = timeBase + elapsedTime;
    
    /*
     debug("now         = %lld\n", now);
     debug("clockDelta  = %lld\n", clockDelta);
     debug("elapsedTime = %lld\n", elapsedTime);
     debug("targetTime  = %lld\n", targetTime);
     debug("\n");
     */
    
    // Check if we're running too slow ...
    if (now > targetTime) {
        
        // Check if we're completely out of sync ...
        if (now - targetTime > 200000000) {
            
            // warn("The emulator is way too slow (%lld).\n", now - targetTime);
            restartTimer();
            return;
        }
    }
    
    // Check if we're running too fast ...
    if (now < targetTime) {
        
        // Check if we're completely out of sync ...
        if (targetTime - now > 200000000) {
            
            warn("The emulator is way too fast (%lld).\n", targetTime - now);
            restartTimer();
            return;
        }
        
        // See you soon...
        mach_wait_until(targetTime);
        /*
         i64 jitter = sleepUntil(targetTime, 1500000); // 1.5 usec early wakeup
         if (jitter > 1000000000) { // 1 sec
         warn("Jitter is too high (%lld).\n", jitter);
         // restartTimer();
         }
         */
    }
}

void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        autoSnapshot = Snapshot::makeWithAmiga(this);
        putMessage(MSG_AUTO_SNAPSHOT_TAKEN);
        
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
        putMessage(MSG_USER_SNAPSHOT_TAKEN);
        
    } else {
        
        // Schedule the snapshot to be taken
        signalUserSnapshot();
    }
}

Snapshot *
Amiga::latestAutoSnapshot()
{
    Snapshot *result = autoSnapshot;
    autoSnapshot = NULL;
    return result;
}

Snapshot *
Amiga::latestUserSnapshot()
{
    Snapshot *result = userSnapshot;
    userSnapshot = NULL;
    return result;
}

void
Amiga::loadFromSnapshotUnsafe(Snapshot *snapshot)
{
    u8 *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        load(ptr);
        ping();
    }
}

void
Amiga::loadFromSnapshotSafe(Snapshot *snapshot)
{
    debug(SNP_DEBUG, "loadFromSnapshotSafe\n");
    
    suspend();
    loadFromSnapshotUnsafe(snapshot);
    resume();
}


//
// The run loop
//

void
Amiga::threadWillStart()
{
    debug(RUN_DEBUG, "Emulator thread started\n");
}

void
Amiga::threadDidTerminate()
{
    debug(RUN_DEBUG, "Emulator thread terminated\n");
    p = NULL;
    
    /* Put emulator into pause mode. If we got here by a call to pause(), the
     * following (reentrant) call to pause() has no effect. If we got here
     * because a breakpoint was reached, the following call will perform the
     * state transition.
     */
    pause();
}

void
Amiga::stepInto()
{
    if (isRunning())
    return;

    cpu.debugger.stepInto();
    run();
}

void
Amiga::stepOver()
{
    if (isRunning())
    return;
    
    cpu.debugger.stepOver();
    run();
}

void
Amiga::runLoop()
{
    debug(RUN_DEBUG, "runLoop()\n");

    // Prepare to run
    restartTimer();
    
    // Enable or disable debugging features
    if (debugMode) {
        cpu.debugger.enableLogging();
    } else {
        cpu.debugger.disableLogging();
    }
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    
    // Enter the loop
    do {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (runLoopCtrl) {
            
            // Are we requested to take a snapshot?
            if (runLoopCtrl & RL_AUTO_SNAPSHOT) {
                autoSnapshot = Snapshot::makeWithAmiga(this);
                putMessage(MSG_AUTO_SNAPSHOT_TAKEN);
                clearControlFlags(RL_AUTO_SNAPSHOT);
            }
            if (runLoopCtrl & RL_USER_SNAPSHOT) {
                userSnapshot = Snapshot::makeWithAmiga(this);
                putMessage(MSG_USER_SNAPSHOT_TAKEN);
                clearControlFlags(RL_USER_SNAPSHOT);
            }

            // Are we requested to update the debugger info structs?
            if (runLoopCtrl & RL_INSPECT) {
                inspect();
                clearControlFlags(RL_INSPECT);
            }

            // Did we reach a breakpoint?
            if (runLoopCtrl & RL_BREAKPOINT_REACHED) {
                inspect();
                putMessage(MSG_BREAKPOINT_REACHED);
                debug(RUN_DEBUG, "BREAKPOINT_REACHED\n");
                clearControlFlags(RL_BREAKPOINT_REACHED);
                break;
            }

            // Did we reach a watchpoint?
            if (runLoopCtrl & RL_WATCHPOINT_REACHED) {
                inspect();
                putMessage(MSG_WATCHPOINT_REACHED);
                debug(RUN_DEBUG, "WATCHPOINT_REACHED\n");
                clearControlFlags(RL_WATCHPOINT_REACHED);
                break;
            }

            // Are we requested to terminate the run loop?
            if (runLoopCtrl & RL_STOP) {
                clearControlFlags(RL_STOP);
                debug(RUN_DEBUG, "RL_STOP\n");
                break;
            }
        }
        
    } while (1);
}

void
Amiga::dumpClock()
{
    msg("               Master cycles     CPU cycles    DMA cycles    CIA cycles\n");
    msg("    CPU clock: %13lld  %13lld %13lld %13lld\n",
             cpu.getMasterClock(),
             AS_CPU_CYCLES(cpu.getMasterClock()),
             AS_DMA_CYCLES(cpu.getMasterClock()),
             AS_CIA_CYCLES(cpu.getMasterClock()));
    msg("    DMA clock: %13lld  %13lld %13lld %13lld\n",
             agnus.clock,
             AS_CPU_CYCLES(agnus.clock),
             AS_DMA_CYCLES(agnus.clock),
             AS_CIA_CYCLES(agnus.clock));
    msg("  CIA A clock: %13lld  %13lld %13lld %13lld\n",
             ciaA.clock,
             AS_CPU_CYCLES(ciaA.clock),
             AS_DMA_CYCLES(ciaA.clock),
             AS_CIA_CYCLES(ciaA.clock));
    msg("  CIA B clock: %13lld  %13lld %13lld %13lld\n",
             ciaB.clock,
             AS_CPU_CYCLES(ciaB.clock),
             AS_DMA_CYCLES(ciaB.clock),
             AS_CIA_CYCLES(ciaB.clock));
    msg("  Color clock: (%d,%d) hex: ($%X,$%X) Frame: %lld\n",
             agnus.pos.v, agnus.pos.h, agnus.pos.v, agnus.pos.h, agnus.frame.nr);
    msg("\n");
}
