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
    config.chipRamSize   = 512;
    config.slowRamSize   = 0;
    config.fastRamSize   = 0;
    config.realTimeClock = false;
    config.df0.type      = A1010_ORIG;
    config.df0.connected = true;
    config.df1.type      = A1010_ORIG;
    config.df1.connected = false;

    // Register sub components
    HardwareComponent *subcomponents[] = {
        
        &mem,
        &dma,
        &denise,
        NULL };
    
    registerSubcomponents(subcomponents, sizeof(subcomponents));
    setAmiga(this);
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        { &warp,               sizeof(warp),               CLEAR_ON_RESET },
        { NULL,                0,                          0 }};
    
    registerSnapshotItems(items, sizeof(items));
    
    // Initialize the mach timer info
    mach_timebase_info(&tb);
}

Amiga::~Amiga()
{
    debug(1, "Destroying Amiga[%p]\n", this);
    powerOff();
}

bool
Amiga::configureModel(AmigaModel model)
{
    if (!isAmigaModel(model)) {
        warn("Invalid Amiga model: %d\n", model);
        warn("       Valid values: %d, %d, %d\n", A500, A1000, A2000);
        return false;
    }
    config.model = model;
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
    config.chipRamSize = size;
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
    config.slowRamSize = size;
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
    config.fastRamSize = size;
    return true;
}

bool
Amiga::configureRealTimeClock(bool value)
{
    config.realTimeClock = value;
    return true;
}

bool
Amiga::configureDrive(unsigned driveNr, bool connected)
{
    switch (driveNr) {
        case 0: config.df0.connected = connected; return true;
        case 1: config.df1.connected = connected; return true;
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
        case 0: config.df0.type = type; return true;
        case 1: config.df1.type = type; return true;
    }
    
    warn("Invalid drive number (%d). Ignoring.\n", driveNr);
    return false;
}

void
Amiga::_powerOn()
{
    msg("Powering on\n");
    
    // Reset the master clock
    masterClock = 0;
    
    // Start the emulator
    run();
    
    // Update GUI elements
    ping();
}

void
Amiga::_powerOff()
{
    msg("Powering off\n");
    
    // Stop the emulator
    pause();
    
    // Update GUI elements
    ping();
}

void
Amiga::_reset()
{
    msg("Resetting\n");
    
    ping();
}

void
Amiga::_ping()
{
    putMessage(warp ? MSG_WARP_ON : MSG_WARP_OFF);
    putMessage(alwaysWarp ? MSG_ALWAYS_WARP_ON : MSG_ALWAYS_WARP_OFF);
}

void
Amiga::_dump()
{
    plainmsg("Current configuration:\n\n");
    plainmsg("   AmigaModel: %s\n", modelName(config.model));
    plainmsg("  chipRamSize: %d KB\n", config.chipRamSize);
    plainmsg("  slowRamSize: %d KB\n", config.slowRamSize);
    plainmsg("  fastRamSize: %d KB\n", config.fastRamSize);
    plainmsg("realTimeClock: %s\n", config.realTimeClock ? "yes" : "no");
    plainmsg("          df0: %s\n", config.df0.connected ? "yes" : "no");
    plainmsg("               %s\n", driveTypeName(config.df0.type));
    plainmsg("          df1: %s\n", config.df1.connected ? "yes" : "no");
    plainmsg("               %s\n", driveTypeName(config.df1.type));

    plainmsg("\n");
    plainmsg("    warp mode: %d (%d) (%d)", warp, warpLoad, alwaysWarp);
    plainmsg("\n");
}

void
Amiga::suspend()
{
    debug(2, "Suspending...\n");
    
    if (suspendCounter == 0 && !isRunning())
        return;
    
    pause();
    suspendCounter++;
}

void
Amiga::resume()
{
    debug(2, "Resuming...\n");
    
    if (suspendCounter == 0)
        return;
    
    if (--suspendCounter == 0)
        run();
}

bool
Amiga::readyToPowerUp()
{
    if (config.model == A1000 && bootRom == NULL) {
        debug("NOT READY TO RUN (A1000)\n");
    }
    if (config.model != A1000 && kickRom == NULL) {
        debug("NOT READY TO RUN (A500, A2000)\n");
    }

    return (config.model == A1000) ? bootRom != NULL : kickRom != NULL;
}

void
Amiga::run()
{
    debug("run()\n");
    
    if (isPaused()) {
        
        // Check for missing Roms
        if (!readyToPowerUp()) {
            putMessage(MSG_ROM_MISSING);
            return;
        }
        
        // Start the emulator thread
        pthread_create(&p, NULL, threadMain, (void *)this);
    }
}

void
Amiga::pause()
{
    if (isRunning()) {
        
        // Cancel the emulator thread
        pthread_cancel(p);
        
        // Wait until the thread has terminated
        pthread_join(p, NULL);
    }
}

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
        
        /* Warping has the unavoidable drawback that audio playback gets out of
         * sync. To cope with this issue, we ramp down the volume when entering
         * warp mode and fade in smoothly when warping ends.
         */
        
        if (warp) {
            // Quickly fade out
            // sid.rampDown();
            
        } else {
            // Smoothly fade in
            // sid.rampUp();
            // sid.alignWritePtr();
            restartTimer();
        }
    }
    
    return warp;
}

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

void
Amiga::deleteBootRom()
{
    if (bootRom) delete bootRom;
    bootRom = NULL;
}

bool
Amiga::loadBootRom(BootRom *rom)
{
    deleteBootRom(); // Delete the old Rom (if any)
    bootRom = rom;
    return bootRom != NULL;
}

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

void
Amiga::deleteKickRom()
{
    if (kickRom) delete kickRom;
    kickRom = NULL;
}

bool
Amiga::loadKickRom(KickRom *rom)
{
    deleteKickRom(); // Delete the old Rom (if any)
    kickRom = rom;
    return kickRom != NULL;
}

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


//
// The run loop
//

void
Amiga::threadWillStart()
{
    debug(2, "Emulator thread started\n");
    amiga->putMessage(MSG_RUN);
}

void
Amiga::threadDidTerminate()
{
    debug(2, "Emulator thread terminated\n");
    p = NULL;
    amiga->putMessage(MSG_PAUSE);
}

void
Amiga::runLoop()
{
    // Prepare to run
    amiga->restartTimer();
    
    //
    // TODO: Emulate the Amiga here ...
    //
    
    // THE FOLLOWING CODE IS FOR VISUAL PROTOTYPING ONLY
    while (!stop) {
        
        // Emulate the CPU (fake)
        uint64_t cpuCycles = 16;  // Fake CPU consumes 16 cycles
        
        // Advance the master clock
        masterClock += cpuCycles * 4;
        
        dma.executeUntil(masterClock);
        denise.executeUntil(masterClock);
        
    }
}
