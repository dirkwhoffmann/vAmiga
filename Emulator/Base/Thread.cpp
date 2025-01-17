// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Thread.h"
#include "Chrono.h"
#include <iostream>

namespace vamiga {

void
Thread::launch()
{
    assert(!isLaunched());

    // Start the thread and enter the main function
    thread = std::thread(&Thread::runLoop, this);

    assert(isLaunched());
}

/*
void
Thread::assertLaunched()
{
    if (!isLaunched()) {

        throw std::runtime_error(string("The emulator thread hasn't been lauchend yet. "
                                        "Missing call to launch()."));
    }
}
*/

void
Thread::resync()
{
    resyncs++;
    baseTime = util::Time::now();
    frameCounter = 0;
}

void
Thread::execute()
{
    // Only proceed if the emulator is running
    if (!isRunning()) return;

    // Determine the number of overdue frames
    isize missing = warp ? 1 : missingFrames();

    if (std::abs(missing) <= 5) {

        lock.lock();
        loadClock.go();

        // Execute all missing frames
        for (isize i = 0; i < missing; i++, frameCounter++) {

            // Execute a single frame
            try { computeFrame(); } catch (StateChangeException &exc) {
                
                // Serve a state change request
                switchState((ExecState)exc.data);
            }
        }
        
        loadClock.stop();
        lock.unlock();

    } else {

        // The emulator is out of sync
        if (missing > 0) {
            debug(VID_DEBUG, "Emulation is way too slow (%ld frames behind)\n", missing);
        } else {
            debug(VID_DEBUG, "Emulation is way too fast (%ld time slices ahead)\n", -missing);
        }

        resync();
    }
}

void
Thread::sleep()
{
    // Don't sleep if the emulator is running in warp mode and no suspension is pending
    if (warp && isRunning() && suspensionLock.tryLock()) {
        
        suspensionLock.unlock();
        return;
    }
    
    // Set a timeout to prevent the thread from stalling
    auto timeout = util::Time::milliseconds(50);

    // Wait for the next pulse
    waitForWakeUp(timeout);
}

void
Thread::computeStats()
{
    if (statsCounter++ == 32) {

        auto used  = loadClock.getElapsedTime().asSeconds();
        auto total = nonstopClock.getElapsedTime().asSeconds();

        loadClock.restart();
        loadClock.stop();
        nonstopClock.restart();

        cpuLoad = 0.3 * cpuLoad + 0.7 * used / total;
        fps = 0.3 * fps + 0.7 * statsCounter / total;

        statsCounter = 0;
    }
}

void
Thread::runLoop()
{
    baseTime = util::Time::now();

    while (state != STATE_HALTED) {

        // Prepare for the next frame
        update();

        // Compute missing frames
        execute();

        // Synchronize timing
        sleep();

        // Compute statistics
        computeStats();
    }
}

void
Thread::switchState(ExecState newState)
{
    assert(isEmulatorThread() || isSuspended());
        
    auto invalid = [&]() {

        fatal("Invalid state transition: %s -> %s\n",
              ExecStateEnum::key(state), ExecStateEnum::key(newState));
    };

    debug(RUN_DEBUG,
          "switchState: %s -> %s\n",
          ExecStateEnum::key(state), ExecStateEnum::key(newState));

    if (!isLaunched()) {

        throw std::runtime_error(string("The emulator thread hasn't been lauchend yet. "
                                        "Missing call to launch()."));
    }

    while (state != newState) {

        switch (newState) {

            case STATE_OFF:

                switch (state) {

                    case STATE_PAUSED:      state = STATE_OFF; _powerOff(); break;
                    case STATE_RUNNING:     state = STATE_PAUSED; _pause(); break;

                    default:
                        invalid();
                }
                break;

            case STATE_PAUSED:

                switch (state) {

                    case STATE_OFF:         state = STATE_PAUSED; _powerOn(); break;
                    case STATE_RUNNING:     state = STATE_PAUSED; _pause(); break;

                    default:
                        invalid();
                }
                break;

            case STATE_RUNNING:

                switch (state) {

                    case STATE_OFF:         state = STATE_PAUSED; _powerOn(); break;
                    case STATE_PAUSED:      state = STATE_RUNNING; _run(); break;

                    default:
                        invalid();
                }
                break;

            case STATE_HALTED:

                switch (state) {

                    case STATE_OFF:     state = STATE_HALTED; _halt(); break;
                    case STATE_PAUSED:  state = STATE_OFF; _powerOff(); break;
                    case STATE_RUNNING: state = STATE_PAUSED; _pause(); break;

                    default:
                        invalid();
                }
        }
    }

    debug(RUN_DEBUG, "switchState: %s\n", ExecStateEnum::key(state));
    assert(state == newState);
}

void
Thread::powerOn()
{
    debug(RUN_DEBUG, "powerOn()\n");

    if (isPoweredOff()) {

        switchState(STATE_PAUSED);
    }
}

void
Thread::powerOff()
{
    debug(RUN_DEBUG, "powerOff()\n");

    if (!isPoweredOff()) {

        switchState(STATE_OFF);
    }
}

void
Thread::run()
{    
    debug(RUN_DEBUG, "run()\n");

    if (!isRunning()) {

        // Throw an exception if the emulator is not ready to run
        isReady();

        switchState(STATE_RUNNING);
    }
}

void
Thread::pause()
{
    debug(RUN_DEBUG, "pause()\n");

    if (isRunning()) {

        switchState(STATE_PAUSED);
    }
}

void
Thread::halt()
{
    debug(RUN_DEBUG, "halt()\n");

    if (state != STATE_UNINIT && state != STATE_HALTED) {

        switchState(STATE_HALTED);
    }
}

void
Thread::warpOn(isize source)
{
    assert(source < 7);

    if (!GET_BIT(warp, source)) {

        SUSPENDED

        auto old = warp;
        SET_BIT(warp, source);
        if (!!old != !!warp) _warpOn();
    }
}

void
Thread::warpOff(isize source)
{
    assert(source < 7);

    if (GET_BIT(warp, source)) {

        SUSPENDED

        auto old = warp;
        CLR_BIT(warp, source);
        if (!!old != !!warp) _warpOff();
    }
}

void
Thread::trackOn(isize source)
{
    assert(source < 7);

    if (!GET_BIT(track, source)) {

        SUSPENDED

        auto old = track;
        SET_BIT(track, source);
        if (!!old != !!track) _trackOn();
    }
}

void
Thread::trackOff(isize source)
{
    assert(source < 7);

    if (GET_BIT(track, source)) {

        SUSPENDED

        auto old = track;
        CLR_BIT(track, source);
        if (!!old != !!track) _trackOff();
    }
}

void
Thread::wakeUp()
{
    trace(TIM_DEBUG, "wakeup: %lld us\n", wakeupClock.restart().asMicroseconds());
    Wakeable::wakeUp();
}

void
Thread::suspend() const
{
    debug(RUN_DEBUG, "Suspending (%ld)...\n", suspendCounter);
        
    if (isEmulatorThread()) {
        
        debug(RUN_DEBUG, "suspend() called by the emulator thread\n");

    } else if (suspendCounter++ == 0) {
        
        suspensionLock.lock();
        lock.lock();
    }
}

void
Thread::resume() const
{
    debug(RUN_DEBUG, "Resuming (%ld)...\n", suspendCounter);

    if (isEmulatorThread()) {
        
        debug(RUN_DEBUG, "resume() called by the emulator thread\n");
        
    } else if (suspendCounter <= 0) {
        
        fatal("resume() called with no call to suspend()\n");
        
    } else if (--suspendCounter == 0) {
        
        suspensionLock.unlock();
        lock.unlock();
    }
}

}
