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
    
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadTerminated();
}

void
*threadMain(void *thisAmiga) {
    
    assert(thisAmiga != NULL);
    
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadStarted();
    
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
    debug(2, "Creating Amiga[%p]\n", this);
    
    p = NULL;
    warp = false;
    alwaysWarp = false;
    warpLoad = false;
    
    // Register sub components
    HardwareComponent *subcomponents[] = {
        
        // &mem,
        NULL };
    
    registerSubcomponents(subcomponents, sizeof(subcomponents));
    setAmiga(this);
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        { &warp,               sizeof(warp),               CLEAR_ON_RESET },
        { NULL,             0,                       0 }};
    
    registerSnapshotItems(items, sizeof(items));
    
    // Initialize the mach timer info
    mach_timebase_info(&tb);
}

Amiga::~Amiga()
{
    debug(1, "Destroying Amiga[%p]\n", this);
    
    // Terminate the emulator thread
    halt();
}

void
Amiga::_powerOn()
{
    msg("Powering on\n");
    
    ping();
}

void
Amiga::_powerOff()
{
    msg("Powering off\n");
    
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
    plainmsg("               Amiga Model : %s\n", modelName(model));
    plainmsg("warp, warpLoad, alwaysWarp : %d %d %d\n", warp, warpLoad, alwaysWarp);
    plainmsg("\n");
}

void
Amiga::suspend()
{
    debug(2, "Suspending...\n");
    
    if (suspendCounter == 0 && isHalted())
        return;
    
    halt();
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

void
Amiga::setModel(AmigaModel model)
{
    if (!isAmigaModel(model)) {
        warn("Unknown Amiga model (%d). Using an A500 instead.\n");
        model = AMIGA_500;
    }
    
    this->model = model;
}

bool
Amiga::isReadyToGo()
{
    return true;
}

void
Amiga::run()
{
    if (isHalted()) {
        
        // Check for missing Roms
        if (!isReadyToGo()) {
            putMessage(MSG_ROM_MISSING);
            return;
        }
        
        // Start the emulator thread
        pthread_create(&p, NULL, threadMain, (void *)this);
    }
}

void
Amiga::halt()
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
    uint64_t kernelNow = mach_absolute_time();
    uint64_t nanoNow = abs_to_nanos(kernelNow);
    
    nanoTargetTime = nanoNow + frameDelay();
}

void
Amiga::synchronizeTiming()
{
    const uint64_t earlyWakeup = 1500000; /* 1.5 milliseconds */
    
    // Get current time in nano seconds
    uint64_t nanoAbsTime = abs_to_nanos(mach_absolute_time());
    
    // Check how long we're supposed to sleep
    int64_t timediff = (int64_t)nanoTargetTime - (int64_t)nanoAbsTime;
    if (timediff > 200000000 || timediff < -200000000 /* 0.2 sec */) {
        
        // The emulator seems to be out of sync, so we better reset the
        // synchronization timer
        
        debug("Synchronization lost (%lld). Restarting timer.\n", timediff);
        restartTimer();
    }
    
    // Convert nanoTargetTime into kernel unit
    int64_t kernelTargetTime = nanos_to_abs(nanoTargetTime);
    
    // Sleep and update target timer
    // debug(2, "%p Sleeping for %lld\n", this, kernelTargetTime - mach_absolute_time());
    int64_t jitter = sleepUntil(kernelTargetTime, earlyWakeup);
    nanoTargetTime += frameDelay();
    
    // debug(2, "Jitter = %d", jitter);
    if (jitter > 1000000000 /* 1 sec */) {
        
        // The emulator did not keep up with the real time clock. Instead of
        // running behind for a long time, we reset the synchronization timer
        
        debug(2, "Jitter exceeds limit (%lld). Restarting timer.\n", jitter);
        restartTimer();
    }
}

//
// The run loop
//

void
Amiga::threadStarted()
{
    debug(2, "Emulator thread started\n");
    amiga->putMessage(MSG_RUN);
}

void
Amiga::runLoop()
{
    // Prepare to run
    amiga->restartTimer();
    
    //
    // TODO: Emulate the Amiga here ...
    //
}

void
Amiga::threadTerminated()
{
    debug(2, "Emulator thread terminated\n");
    p = NULL;
    amiga->putMessage(MSG_HALT);
}
