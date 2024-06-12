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

Thread::Thread(Emulator& ref) : CoreComponent(ref)
{

}

Thread::~Thread()
{
    // Wait until the thread has terminated
    join();
}

void
Thread::launch()
{
    assert(!isLaunched());

    // Start the thread and enter the main function
    thread = std::thread(&Thread::runLoop, this);

    assert(isLaunched());
}

void
Thread::assertLaunched()
{
    if (!isLaunched()) {

        throw std::runtime_error(string("The emulator thread hasn't been lauchend yet. "
                                        "Missing call to launch()."));
    }
}

void
Thread::resync()
{
    targetTime = util::Time::now();
    baseTime = util::Time::now();
    deltaTime = 0;
    frameCounter = 0;
    missing = 0;
}

void
Thread::executeFrame()
{
    // Only proceed if the emulator is running
    if (!isRunning()) return;

    // Determine the number of overdue frames
    isize missing = warp ? 1 : missingSlices();

    if (std::abs(missing) <= 5) {

        loadClock.go();
        try {

            // Execute all missing frames
            for (isize i = 0; i < missing; i++, frameCounter++) execute();

        } catch (StateChangeException &exc) {

            // Interruption
            switchState((ExecState)exc.data);
        }
        loadClock.stop();

    } else {

        // The emulator got out of sync
        if (missing > 0) {
            warn("Emulation is way too slow (%ld frames behind)\n", missing);
        } else {
            warn("Emulation is way too fast (%ld time slices ahead)\n", -missing);
        }

        resync();
    }

    /* OLD
    if (missing > 0 || warp) {

        trace(TIM_DEBUG, "execute: %lld us\n", execClock.restart().asMicroseconds());

        loadClock.go();

        execute();
        frameCounter++;
        missing--;

        loadClock.stop();
    }
    */
}

void
Thread::sleepFrame()
{
    // Don't sleep if the emulator is running in warp mode
    if (warp && isRunning()) return;

    // Set a timeout to prevent the thread from stalling
    auto timeout = util::Time::milliseconds(50);

    // Wait for the next pulse
    waitForWakeUp(timeout);
}

void
Thread::runLoop()
{
    debug(RUN_DEBUG, "main()\n");

    baseTime = util::Time::now();

    while (state != STATE_HALTED) {

        // Prepare for the next frame
        update();

        // Compute missing frames
        executeFrame();

        // Are we requested to change state?
        if (stateChangeRequest.test()) {

            switchState(newState);
            stateChangeRequest.clear();
            stateChangeRequest.notify_one();
        }

        // Synchronize timing
        sleepFrame();

        // Compute statistics
        computeStats();
    }
}

void
Thread::switchState(ExecState newState)
{
    assert(isEmulatorThread());

    while (newState != state) {

        if (state == STATE_OFF && newState == STATE_PAUSED) {

            CoreComponent::powerOn();
            state = STATE_PAUSED;

        } else if (state == STATE_OFF && newState == STATE_RUNNING) {

            CoreComponent::powerOn();
            state = STATE_PAUSED;

        } else if (state == STATE_PAUSED && newState == STATE_OFF) {

            CoreComponent::powerOff();
            state = STATE_OFF;

        } else if (state == STATE_PAUSED && newState == STATE_RUNNING) {

            CoreComponent::run();
            state = STATE_RUNNING;

        } else if (state == STATE_RUNNING && newState == STATE_OFF) {

            state = STATE_PAUSED;
            CoreComponent::pause();

        } else if (state == STATE_RUNNING && newState == STATE_PAUSED) {

            state = STATE_PAUSED;
            CoreComponent::pause();

        } else if (state == STATE_RUNNING && newState == STATE_SUSPENDED) {

            state = STATE_SUSPENDED;

        } else if (state == STATE_SUSPENDED && newState == STATE_RUNNING) {

            state = STATE_RUNNING;

        } else if (newState == STATE_HALTED) {

            CoreComponent::halt();
            state = STATE_HALTED;

        } else {

            // Invalid state transition
            fatalError;
        }

        debug(RUN_DEBUG, "Changed state to %s\n", ExecStateEnum::key(state));
    }
}

void
Thread::switchWarp(bool state, u8 source)
{
    assert(source >= 0 && source < 8);

    if (!isEmulatorThread()) suspend();

    u8 old = warp;
    state ? SET_BIT(warp, source) : CLR_BIT(warp, source);

    if (bool(old) != bool(warp)) {
        CoreComponent::warpOnOff(warp);
    }

    if (!isEmulatorThread()) resume();
}

void
Thread::switchTrack(bool state, u8 source)
{
    assert(source >= 0 && source < 8);
    assert(isEmulatorThread() || !isRunning());

    u8 old = track;
    state ? SET_BIT(track, source) : CLR_BIT(track, source);

    if (bool(old) != bool(track)) {
        CoreComponent::trackOnOff(track);
    }
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
Thread::powerOn()
{
    debug(RUN_DEBUG, "powerOn()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (isPoweredOff()) {
        
        // Request a state change and wait until the new state has been reached
        changeStateTo(STATE_PAUSED);
    }
}

void
Thread::powerOff()
{
    debug(RUN_DEBUG, "powerOff()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (!isPoweredOff()) {

        // Request a state change and wait until the new state has been reached
        changeStateTo(STATE_OFF);
    }
}

void
Thread::run()
{
    debug(RUN_DEBUG, "run()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());

    if (!isRunning()) {

        // Throw an exception if the emulator is not ready to run
        isReady();
        
        // Request a state change and wait until the new state has been reached
        changeStateTo(STATE_RUNNING);
    }
}

void
Thread::pause()
{
    debug(RUN_DEBUG, "pause()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (isRunning()) {

        // Request a state change and wait until the new state has been reached
        changeStateTo(STATE_PAUSED);
    }
}

void
Thread::halt()
{
    assert(!isEmulatorThread());

    changeStateTo(STATE_HALTED);
    join();
}

void
Thread::warpOn(isize source)
{
    SUSPENDED switchWarp(true, u8(source));
}

void
Thread::warpOff(isize source)
{
    SUSPENDED switchWarp(false, u8(source));
}

void
Thread::trackOn(isize source)
{
    SUSPENDED switchTrack(true, u8(source));
}

void
Thread::trackOff(isize source)
{
    SUSPENDED switchTrack(false, u8(source));
}

void
Thread::changeStateTo(ExecState requestedState)
{
    assertLaunched();

    if (isEmulatorThread()) {

        // Switch immediately
        switchState(requestedState);
        assert(state == requestedState);

    } else {

        // Remember the requested state
        newState = requestedState;

        // Request the change
        assert(stateChangeRequest.test() == false);
        stateChangeRequest.test_and_set();
        assert(stateChangeRequest.test() == true);

        if (!isEmulatorThread()) {

            // Wait until the change has been performed
            stateChangeRequest.wait(true);
            assert(stateChangeRequest.test() == false);
        }
    }
}

void
Thread::wakeUp()
{
    trace(TIM_DEBUG, "wakeup: %lld us\n", wakeupClock.restart().asMicroseconds());
    Wakeable::wakeUp();
}

void
Thread::suspend()
{
    if (!isEmulatorThread()) {

        debug(RUN_DEBUG, "Suspending (%ld)...\n", suspendCounter);

        if (suspendCounter || isRunning()) {

            suspendCounter++;
            assert(state == STATE_RUNNING || state == STATE_SUSPENDED);
            changeStateTo(STATE_SUSPENDED);
        }

    } else {

        debug(RUN_DEBUG, "Skipping suspend (%ld)...\n", suspendCounter);
    }
}

void
Thread::resume()
{
    if (!isEmulatorThread()) {

        debug(RUN_DEBUG, "Resuming (%ld)...\n", suspendCounter);

        if (suspendCounter && --suspendCounter == 0) {

            assert(state == STATE_SUSPENDED);
            changeStateTo(STATE_RUNNING);
            run();
        }
        
    } else {

        debug(RUN_DEBUG, "Skipping resume (%ld)...\n", suspendCounter);
    }
}

}
