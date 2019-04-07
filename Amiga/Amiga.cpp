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
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &cpu,
        &ciaA,
        &ciaB,
        &mem,
        &agnus,
        &denise,
        &paula,
        &controlPort1,
        &controlPort2,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &model,         sizeof(model),         PERSISTANT },
        { &realTimeClock, sizeof(realTimeClock), PERSISTANT },
    
        { &masterClock,   sizeof(masterClock),   0 },
        { &clockBase,     sizeof(clockBase),     0 },
    });
    
    // Install a reference to the top-level object in each subcomponent
    setAmiga(this);
    
    // Initialize the mach timer info
    mach_timebase_info(&tb);
}

Amiga::~Amiga()
{
    debug("Destroying Amiga[%p]\n", this);
    powerOff();
    
    if (this == activeAmiga) {
        debug("Stop being the active emulator instance\n");
        activeAmiga = NULL;
    }
}

void
Amiga::makeActiveInstance()
{
    // Return immediately if this emulator instance is the active instance
    if (activeAmiga == this) return;
    
    /* Pause the currently active emulator instance (if any)
     * Because we're going to use the CPU core, we need to save the active
     * instance's CPU context. It will be restored when the other instance
     * becomes the active again (by calling this function).
     */
    if (activeAmiga != NULL) {
        activeAmiga->pause();
        activeAmiga->cpu.recordContext();
    }
    
    // Restore the previously recorded CPU state (if any)
    cpu.restoreContext();
    
    // Bind the CPU core to this emulator instance
    activeAmiga = this;
}

void
Amiga::setDebugMode(bool enable)
{
    if ((debugMode = enable)) {
        
        debug("Enabling debug mode\n");
        setControlFlags(RL_ENABLE_TRACING | RL_ENABLE_BREAKPOINTS);

    } else {

        debug("Disabling debug mode\n");
        clearControlFlags(RL_ENABLE_TRACING | RL_ENABLE_BREAKPOINTS);
    }
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    inspectionTarget = id;
    handler->scheduleSecRel(INSPECTOR_SLOT, 0, inspectionTarget);
    resume();
}

void
Amiga::clearInspectionTarget()
{
    setInspectionTarget(INS_NONE);
}

AmigaInfo
Amiga::getInfo()
{
    AmigaInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

AmigaConfiguration
Amiga::getConfig()
{
    AmigaConfiguration config;
    
    config.model = model;
    config.realTimeClock = realTimeClock;
    config.layout = keyboard.layout;
    
    config.df0.connected = paula.diskController.isConnected(0);
    config.df0.type = df0.getType();
    
    config.df1.connected = paula.diskController.isConnected(1);
    config.df1.type = df1.getType();

    config.df2.connected = paula.diskController.isConnected(2);
    config.df2.type = df2.getType();

    config.df3.connected = paula.diskController.isConnected(3);
    config.df3.type = df3.getType();

    return config;
}

AmigaMemConfiguration
Amiga::getMemConfig()
{
    AmigaMemConfiguration config;
    
    assert(mem.chipRamSize % 1024 == 0);
    assert(mem.slowRamSize % 1024 == 0);
    assert(mem.fastRamSize % 1024 == 0);
    
    config.chipRamSize = mem.chipRamSize / 1024;
    config.slowRamSize = mem.slowRamSize / 1024;
    config.fastRamSize = mem.fastRamSize / 1024;
    
    return config;
}

bool
Amiga::configureModel(AmigaModel m)
{
    if (!isAmigaModel(m)) {
        
        warn("Invalid Amiga model: %d\n", m);
        warn("       Valid values: %d, %d, %d\n", A500, A1000, A2000);
        return false;
    }
    
    if (model != m) {
        
        model = m;
        putMessage(MSG_CONFIG);
    }
    
    return true;
}

bool
Amiga::configureLayout(long layout)
{
    if (keyboard.layout != layout) {
        
        keyboard.layout = layout;
        putMessage(MSG_CONFIG);
    }
    
    return true;
}

bool
Amiga::configureChipMemory(long size)
{
    if (size != 256 && size != 512) {
        
        warn("Invalid Chip Ram size: %d\n", size);
        warn("         Valid values: 256KB, 512KB\n");
        return false;
    }
    
    mem.allocateChipRam(KB(size));
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureSlowMemory(long size)
{
    if ((size % 256) != 0 || size > 512) {
        
        warn("Invalid Slow Ram size: %d\n", size);
        warn("         Valid values: 0KB, 256KB, 512KB\n");
        return false;
    }
    
    mem.allocateSlowRam(KB(size));
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureFastMemory(long size)
{
    if ((size % 64) != 0 || size > 8192) {
        
        warn("Invalid Fast Ram size: %d\n", size);
        warn("         Valid values: 0KB, 64KB, 128KB, ..., 8192KB (8MB)\n");
        return false;
    }
    
    mem.allocateFastRam(KB(size));
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureRealTimeClock(bool value)
{
    if (realTimeClock != value) {
        
        realTimeClock = value;
        mem.updateMemSrcTable();
        putMessage(MSG_CONFIG);
    }
    
    return true;
}

bool
Amiga::configureDrive(unsigned df, bool connected)
{
    if (df >= 4) {
        warn("Invalid drive number (%d). Ignoring request.\n", df);
        return false;
    }

    if (df == 0 && !connected) {
        warn("Df0 cannot be disconnected. Ignoring request.\n");
        connected = true;
    }
    
    paula.diskController.setConnected(df, connected);
    putMessage(MSG_CONFIG);
    return true;
}

bool
Amiga::configureDrive(unsigned driveNr, DriveType type)
{
    if (!isDriveType(type)) {
        
        warn("Invalid drive type: %d\n", type);
        return false;
    }
    
    switch (driveNr) {
        
        case 0:
        if (df0.getType() != type) {
            df0.setType(type);
            putMessage(MSG_CONFIG);
        }
        return true;
        
        case 1:
        if (df1.getType() != type) {
            df1.setType(type);
            putMessage(MSG_CONFIG);
        }
        return true;
        
        case 2:
        if (df2.getType() != type) {
            df2.setType(type);
            putMessage(MSG_CONFIG);
        }
        return true;
        
        case 3:
        if (df3.getType() != type) {
            df3.setType(type);
            putMessage(MSG_CONFIG);
        }
        return true;
    }
    
    warn("Invalid drive number (%d). Ignoring.\n", driveNr);
    return false;
}

void
Amiga::_powerOn()
{
    debug(1, "Power on\n");
    
    masterClock = 0;
    
    // Make this emulator instance the active one
    makeActiveInstance();
    
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_pulse_reset();
    
    // For debugging, we start in debug mode and set a breakpoint
    // debugMode = true;
    
    // cpu.bpManager.setBreakpointAt(0xFC570E); // Blitter
    // cpu.bpManager.setBreakpointAt(0xFE8A6E); // All Blitter stuff done
    
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
    debug(1, "Run\n");
    
    // Check for missing Roms.
    if (!readyToPowerUp()) {
        putMessage(MSG_ROM_MISSING);
        return;
    }
    
    // Make this Amiga the active emulator instance.
    makeActiveInstance();
    
    // Start the emulator thread.
    pthread_create(&p, NULL, threadMain, (void *)this);
    
    // Inform the GUI.
    amiga->putMessage(MSG_RUN);
}

void
Amiga::_pause()
{
    debug(1, "Pause (p = %p)\n", p);
    
    // Cancel the emulator thread if it still running
    if (p) signalStop();
    
    // Wait until the thread has terminated
    pthread_join(p, NULL);
    
    // Update the recorded debug information
    inspect();

    // Inform the GUI
    amiga->putMessage(MSG_PAUSE);
}

void
Amiga::_reset()
{
    msg("Reset\n");
    
    amiga->putMessage(MSG_RESET);
    ping();
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
    
    info.masterClock = masterClock;
    info.dmaClock = agnus.clock;
    info.ciaAClock = ciaA.clock;
    info.ciaBClock = ciaB.clock;
    info.frame = agnus.frame;
    info.vpos = agnus.vpos;
    info.hpos = agnus.hpos;
    
    pthread_mutex_unlock(&lock);
}

void
Amiga::_dump()
{
    AmigaConfiguration config = getConfig();
    
    dumpClock();
    plainmsg("    poweredOn: %s\n", isPoweredOn() ? "yes" : "no");
    plainmsg("   poweredOff: %s\n", isPoweredOff() ? "yes" : "no");
    plainmsg("       paused: %s\n", isPaused() ? "yes" : "no");
    plainmsg("      running: %s\n", isRunning() ? "yes" : "no");
    plainmsg("\n");
    plainmsg("Current configuration:\n\n");
    plainmsg("   AmigaModel: %s\n", modelName(config.model));
    plainmsg("realTimeClock: %s\n", config.realTimeClock ? "yes" : "no");
    plainmsg("          df0: %s %s\n",
             config.df0.connected ? "yes" : "no", driveTypeName(config.df0.type));
    plainmsg("          df1: %s %s\n",
             config.df1.connected ? "yes" : "no", driveTypeName(config.df1.type));
    plainmsg("          df2: %s %s\n",
             config.df2.connected ? "yes" : "no", driveTypeName(config.df2.type));
    plainmsg("          df3: %s %s\n",
             config.df3.connected ? "yes" : "no", driveTypeName(config.df3.type));

    plainmsg("\n");
    plainmsg("         warp: %d (%d) (%d)", warp);
    plainmsg("\n");
}

void
Amiga::_setWarp(bool value) {
    
    if (value) {
        
        putMessage(MSG_WARP_ON);
        
    } else {
        
        restartTimer();
        putMessage(MSG_WARP_OFF);
    }
}

void
Amiga::suspend()
{
    debug(2, "Suspending (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0 && !isRunning())
    return;
    
    pause();
    suspendCounter++;
}

void
Amiga::resume()
{
    debug(2, "Resuming (%d)...\n", suspendCounter);
    
    if (suspendCounter == 0)
    return;
    
    if (--suspendCounter == 0)
    run();
}

bool
Amiga::readyToPowerUp()
{
    AmigaConfiguration config = getConfig();
    
    // Check for Chip Ram
    if (!mem.hasChipRam()) {
        msg("NOT READY YET: Chip Ram is missing.\n");
        return false;
    }
    
    // Check for a Boot Rom (A1000 only)
    if (config.model == A1000 && !mem.hasBootRom()) {
        msg("NOT READY YET: Boot Rom is missing.\n");
        return false;
    }
    
    // Check for a Kickstart Rom (A500, A2000)
    if (config.model != A1000 && !mem.hasKickRom()) {
        msg("NOT READY YET: Kickstart Rom is missing.\n");
        return false;
    }
    
    return true;
}

void
Amiga::setControlFlags(uint32_t flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl |= flags;
    pthread_mutex_unlock(&lock);
}

void
Amiga::clearControlFlags(uint32_t flags)
{
    pthread_mutex_lock(&lock);
    runLoopCtrl &= ~flags;
    pthread_mutex_unlock(&lock);
}

/*
 void
 Amiga::setAlwaysWarp(bool b)
 {
 if (alwaysWarp != b) {
 
 alwaysWarp = b;
 putMessage(b ? MSG_ALWAYS_WARP_ON : MSG_ALWAYS_WARP_OFF);
 }
 }
 
 bool
 Amiga::getWarp()
 {
 bool driveDMA = false; // TODO
 bool newValue = alwaysWarp || (warpLoad && driveDMA);
 
 if (newValue != warp) {
 
 warp = newValue;
 putMessage(warp ? MSG_WARP_ON : MSG_WARP_OFF);
 
 if (warp) {
 // Quickly fade out
 paula.audioUnit.rampDown();
 
 } else {
 // Smoothly fade in
 paula.audioUnit.rampUp();
 paula.audioUnit.alignWritePtr();
 restartTimer();
 }
 }
 
 return warp;
 }
 */

void
Amiga::restartTimer()
{
    timeBase = time_in_nanos();
    clockBase = masterClock;
    
    uint64_t kernelNow = mach_absolute_time();
    uint64_t nanoNow = abs_to_nanos(kernelNow);
    
    // DEPRECATED
    nanoTargetTime = nanoNow + frameDelay();
}

void
Amiga::synchronizeTiming()
{
    uint64_t now         = time_in_nanos();
    uint64_t clockDelta  = masterClock - clockBase;
    uint64_t elapsedTime = (clockDelta * 1000) / masterClockFrequency;
    uint64_t targetTime  = timeBase + elapsedTime;
    
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
            
            warn("The emulator is way too slow (%lld).\n", now - targetTime);
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
         int64_t jitter = sleepUntil(targetTime, 1500000); // 1.5 usec early wakeup
         if (jitter > 1000000000) { // 1 sec
         warn("Jitter is too high (%lld).\n", jitter);
         // restartTimer();
         }
         */
    }
}

bool
Amiga::snapshotIsDue()
{
    unsigned fps = 50; // PAL frames per second
    
    if (!getTakeAutoSnapshots() || amiga->getSnapshotInterval() <= 0)
    return false;
    
    return agnus.frame % (fps * amiga->getSnapshotInterval()) == 0;
}

void
Amiga::loadFromSnapshotUnsafe(AmigaSnapshot *snapshot)
{
    uint8_t *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        loadFromBuffer(&ptr);
        keyboard.releaseAllKeys(); // Avoid constantly pressed keys
        ping();
    }
}

void
Amiga::loadFromSnapshotSafe(AmigaSnapshot *snapshot)
{
    debug(2, "Amiga::loadFromSnapshotSafe\n");
    
    suspend();
    loadFromSnapshotUnsafe(snapshot);
    resume();
}

bool
Amiga::restoreSnapshot(vector<AmigaSnapshot *> &storage, unsigned nr)
{
    AmigaSnapshot *snapshot = getSnapshot(storage, nr);
    
    if (snapshot) {
        loadFromSnapshotSafe(snapshot);
        return true;
    }
    
    return false;
}

size_t
Amiga::numSnapshots(vector<AmigaSnapshot *> &storage)
{
    return storage.size();
}

AmigaSnapshot *
Amiga::getSnapshot(vector<AmigaSnapshot *> &storage, unsigned nr)
{
    return nr < storage.size() ? storage.at(nr) : NULL;
    
}

void
Amiga::takeSnapshot(vector<AmigaSnapshot *> &storage)
{
    // Delete oldest snapshot if capacity limit has been reached
    if (storage.size() >= MAX_SNAPSHOTS) {
        deleteSnapshot(storage, MAX_SNAPSHOTS - 1);
    }
    
    AmigaSnapshot *snapshot = AmigaSnapshot::makeWithAmiga(this);
    storage.insert(storage.begin(), snapshot);
    putMessage(MSG_SNAPSHOT_TAKEN);
}

void
Amiga::deleteSnapshot(vector<AmigaSnapshot *> &storage, unsigned index)
{
    AmigaSnapshot *snapshot = getSnapshot(storage, index);
    
    if (snapshot) {
        delete snapshot;
        storage.erase(storage.begin() + index);
    }
}


//
// The run loop
//

void
Amiga::threadWillStart()
{
    debug(2, "Emulator thread started\n");
}

void
Amiga::threadDidTerminate()
{
    debug(2, "Emulator thread terminated\n");
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
    
    cpu.bpManager.setSoftBreakpointAt(UINT32_MAX);
    run();
}

void
Amiga::stepOver()
{
    if (isRunning())
    return;
    
    debug("Setting bp at %X\n", cpu.getNextPC());
    cpu.bpManager.setSoftBreakpointAt(cpu.getNextPC());
    run();
}

void
Amiga::runLoop()
{
    // Prepare to run
    amiga->restartTimer();
    
    // Enable or disable debugging features
    debugMode ? setControlFlags(RL_DEBUG) : clearControlFlags(RL_DEBUG);
    handler->scheduleSecRel(INSPECTOR_SLOT, 0, inspectionTarget);
    
    // Enter the loop
    do {
        
        // Emulate CPU instruction
        CPUCycle cpuCycles = cpu.executeNextInstruction();
        
        // Advance the masterclock
        masterClock += CPU_CYCLES(cpuCycles);
        
        // Emulate DMA (Agnus is responsible for that)
        agnus.executeUntil(masterClock);
        
        // Check if special action needs to be taken ...
        if (runLoopCtrl) {
            
            // Are we requested to take a snapshot?
            if (runLoopCtrl & RL_SNAPSHOT) {
                takeAutoSnapshot();
                clearControlFlags(RL_SNAPSHOT);
            }
            
            // Are we requested to update the debugger info structs?
            if (runLoopCtrl & RL_INSPECT) {
                inspect();
                clearControlFlags(RL_INSPECT);
            }
            
            // Are we requested to record the execution?
            if (runLoopCtrl & RL_ENABLE_TRACING) {
                cpu.recordInstruction();
            }
            
            // Are we requestes to check for breakpoints?
            if (runLoopCtrl & RL_ENABLE_BREAKPOINTS) {
                if (cpu.bpManager.shouldStop()) {
                    putMessage(MSG_BREAKPOINT_REACHED);
                    break;
                }
            }
            
            // Are we requests to terminate the run loop?
            if (runLoopCtrl & RL_STOP) {
                clearControlFlags(RL_STOP);
                break;
            }
        }
        
    } while (1);
}

void
Amiga::dumpClock()
{
    plainmsg("               Master cycles     CPU cycles    DMA cycles    CIA cycles\n");
    plainmsg(" Master clock: %13lld  %13lld %13lld %13lld\n",
             masterClock,
             AS_CPU_CYCLES(masterClock),
             AS_DMA_CYCLES(masterClock),
             AS_CIA_CYCLES(masterClock));
    plainmsg("    DMA clock: %13lld  %13lld %13lld %13lld\n",
             amiga->agnus.clock,
             AS_CPU_CYCLES(amiga->agnus.clock),
             AS_DMA_CYCLES(amiga->agnus.clock),
             AS_CIA_CYCLES(amiga->agnus.clock));
    plainmsg("  Frame clock: %13lld  %13lld %13lld %13lld\n",
             amiga->agnus.latchedClock,
             AS_CPU_CYCLES(amiga->agnus.latchedClock),
             AS_DMA_CYCLES(amiga->agnus.latchedClock),
             AS_CIA_CYCLES(amiga->agnus.latchedClock));
    plainmsg("  CIA A clock: %13lld  %13lld %13lld %13lld\n",
             amiga->ciaA.clock,
             AS_CPU_CYCLES(amiga->ciaA.clock),
             AS_DMA_CYCLES(amiga->ciaA.clock),
             AS_CIA_CYCLES(amiga->ciaA.clock));
    plainmsg("  CIA B clock: %13lld  %13lld %13lld %13lld\n",
             amiga->ciaB.clock,
             AS_CPU_CYCLES(amiga->ciaB.clock),
             AS_DMA_CYCLES(amiga->ciaB.clock),
             AS_CIA_CYCLES(amiga->ciaB.clock));
    plainmsg("  Color clock: (%d,%d) hex: ($%X,$%X) Frame: %lld\n",
             agnus.vpos, agnus.hpos, agnus.vpos, agnus.hpos, agnus.frame);
    plainmsg("\n");
}
