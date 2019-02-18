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
    
    /* Set up a meaningful initial configuration.
     * This configuration is unlikely to be used though. Under normal
     * circumstances, the GUI overwrites it with it's own settings before
     * powering on.
     */
    config.model         = A500;
    config.realTimeClock = false;
    config.df0.type      = A1010_ORIG;
    config.df0.connected = true;
    config.df1.type      = A1010_ORIG;
    config.df1.connected = false;
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &cpu,
        &ciaA,
        &ciaB,
        &mem,
        &dma,
        &denise,
        &paula,
        &controlPort1,
        &controlPort2,
        &keyboard,
        &diskController,
        &df0,
        &df1,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &config.model, sizeof(config.model), 0 },
        { &config.layout, sizeof(config.layout), 0 },
    });
    
    /*
         { }
         AmigaModel model;
         long layout;
         long chipRamSize; // size in KB
         long slowRamSize; // size in KB
         long fastRamSize; // size in KB
         bool realTimeClock;
         DriveConfiguration df0;
         DriveConfiguration df1;
         */
         
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
    activeAmiga = this;
    
    // TODO: Add code to pull and put the context of the Musashi CPU in and out.
}

void
Amiga::enableDebugging()
{
    debug("Enabling debug mode\n");
    debugMode = true;
}

void
Amiga::disableDebugging()
{
    debug("Disabling debug mode\n");
    debugMode = false;
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
Amiga::configureModel(AmigaModel model)
{
    if (!isAmigaModel(model)) {
        
        warn("Invalid Amiga model: %d\n", model);
        warn("       Valid values: %d, %d, %d\n", A500, A1000, A2000);
        return false;
    }
    
    if (config.model != model) {

        config.model = model;
        putMessage(MSG_CONFIG);
    }
    
    return true;
}

bool
Amiga::configureLayout(long layout)
{
    if (config.layout != layout) {
        
        config.layout = layout;
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
    return true;
}

bool
Amiga::configureRealTimeClock(bool value)
{
    if (config.realTimeClock != value) {
        
        config.realTimeClock = value;
        mem.updateMemSrcTable();
    }
    
    return true;
}

bool
Amiga::configureDrive(unsigned driveNr, bool connected)
{
    switch (driveNr) {
            
        case 0:
            if (config.df0.connected != connected) {
                
                config.df0.connected = connected;
                df0.setConnected(connected);
                putMessage(MSG_CONFIG);
            }
            return true;
            
        case 1:
            if (config.df1.connected != connected) {
                
                config.df1.connected = connected;
                df1.setConnected(connected);
                putMessage(MSG_CONFIG);
            }
            return true;
    }
    
    warn("Invalid drive number (%d). Ignoring.\n", driveNr);
    return false;
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
            if (config.df0.type != type) {
                
                config.df0.type = type;
                putMessage(MSG_CONFIG);
            }
            return true;
            
        case 1:
            if (config.df1.type != type) {
                
                config.df1.type = type;
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
    
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_pulse_reset();
    
    // For debugging, we start in debug mode and set a breakpoint
    debugMode = true;

    // cpu.bpManager.setBreakpointAt(0xFC00DE);
    // cpu.bpManager.setBreakpointAt(0xFC015E);
    // cpu.bpManager.setBreakpointAt(0xFC0208);
    // cpu.bpManager.setBreakpointAt(0xFC0222);
    // cpu.bpManager.setBreakpointAt(0xFC02A8);
    // cpu.bpManager.setBreakpointAt(0xFC02B0);
    // cpu.bpManager.setBreakpointAt(0xFC0380);
    // cpu.bpManager.setBreakpointAt(0xFC039C);
    // cpu.bpManager.setBreakpointAt(0xFC03C2);
    // FC1A38
    // FC1F60
    // cpu.bpManager.setBreakpointAt(0xFC03C6);
    // FC03E2
    // FC041E
    // FC0434
    // FC045E
    // FC04A2
    cpu.bpManager.setBreakpointAt(0xFC04BE); // "Historic moment"
    
    
    

    
    putMessage(MSG_POWER_ON);
}

void
Amiga::_powerOff()
{
    debug(1, "Power off\n");
    
    putMessage(MSG_POWER_OFF);
}

void
Amiga::_run()
{
    debug(1, "Run\n");
    
    // Check for missing Roms
    if (!readyToPowerUp()) {
        putMessage(MSG_ROM_MISSING);
        return;
    }
    
    // Start the emulator thread
    pthread_create(&p, NULL, threadMain, (void *)this);
    
    amiga->putMessage(MSG_RUN);
}

void
Amiga::_pause()
{
    debug(1, "Pause\n");
    
    // Cancel the emulator thread
    stop = true;
    
    // Wait until the thread has terminated
    pthread_join(p, NULL);
    
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
Amiga::_dump()
{
    plainmsg(" Master clock: %lld\n", masterClock);
    plainmsg("in CPU cycles: %lld\n", AS_CPU_CYCLES(masterClock));
    plainmsg("in DMA cycles: %lld\n", AS_DMA_CYCLES(masterClock));
    plainmsg("in CIA cycles: %lld\n", AS_CIA_CYCLES(masterClock));
    plainmsg("\n");
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


//
// Handling Roms
//


/*
bool
Amiga::loadBootRomFromBuffer(const uint8_t *buffer, size_t length)
{
    assert(buffer != NULL);
    
    BootRom *rom = BootRom::makeWithBuffer(buffer, length);
    
    if (!rom) {
        msg("Failed to read Boot Rom from buffer at %p\n", buffer);
        return false;
    }
    
    return loadBootRom(rom);
}

bool
Amiga::loadBootRomFromFile(const char *path)
{
    assert(path != NULL);
    
    BootRom *rom = BootRom::makeWithFile(path);
    
    if (!rom) {
        msg("Failed to read Boot Rom from file %s\n", path);
        return false;
    }
    
    return loadBootRom(rom);
}
*/

/*
bool
Amiga::loadKickRomFromBuffer(const uint8_t *buffer, size_t length)
{
    assert(buffer != NULL);
    
    KickRom *rom = KickRom::makeWithBuffer(buffer, length);
    
    if (!rom) {
        msg("Failed to read Kick Rom from buffer at %p\n", buffer);
        return false;
    }
    
    return loadKickRom(rom);
}

bool
Amiga::loadKickRomFromFile(const char *path)
{
    assert(path != NULL);
    
    KickRom *rom = KickRom::makeWithFile(path);
    
    if (!rom) {
        msg("Failed to read Kick Rom from file %s\n", path);
        return false;
    }
    
    return loadKickRom(rom);
}
*/

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
    debug("Amiga::runLoop\n");
    
    // Prepare to run
    amiga->restartTimer();
    
    //
    // TODO: Emulate the Amiga here ...
    //
    
    // THE FOLLOWING CODE IS FOR VISUAL PROTOTYPING ONLY
    
    // Execute at least one instruction
    stop = false;
    do {
        
        // Emulate the CPU
        CPUCycle cpuCycles = cpu.executeNextInstruction();
        
        // Advance the masterclock
        masterClock += CPU_CYCLES(cpuCycles);
        // debug("CPU has executed %lld cycles newClock = %lld\n", CPU_CYCLES(cpuCycles), masterClock);

        // Emulate the DMA controller
        dma.executeUntil(masterClock);
        
        if (debugMode) {

            // Check if a breakpoint has been reached
            if (cpu.bpManager.shouldStop()) {
                stop = true;
                putMessage(MSG_BREAKPOINT_REACHED);
                debug("MSG_BREAKPOINT_REACHED at %X\n", cpu.getPC());
            }
        }
        
        // stop = true;
    } while (!stop);
}
