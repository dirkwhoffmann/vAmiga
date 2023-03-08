// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Thread.h"
#include "Chrono.h"
#include <iostream>

namespace vamiga {

Thread::Thread()
{

}

Thread::~Thread()
{
    // Wait until the thread has terminated
    join();
}

template <> void
Thread::execute<Thread::ThreadMode::Periodic>()
{
    loadClock.go();
    execute();
    loadClock.stop();
}

template <> void
Thread::execute<Thread::ThreadMode::Pulsed>()
{
    loadClock.go();
    execute();
    loadClock.stop();
    
}

template <> void
Thread::sleep<Thread::ThreadMode::Periodic>()
{
    auto now = util::Time::now();

    // Only proceed if we're not running in warp mode
    if (warp) return;

    // Check if we're running too slow...
    if (now > targetTime) {
        
        // Check if we're completely out of sync...
        if ((now - targetTime).asMilliseconds() > 200) {

            warn("Emulation is way too slow: %f\n",(now - targetTime).asSeconds());

            // Restart the sync timer
            targetTime = util::Time::now();
        }
    }
    
    // Check if we're running too fast...
    if (now < targetTime) {
        
        // Check if we're completely out of sync...
        if ((targetTime - now).asMilliseconds() > 200) {
            
            warn("Emulation is way too slow: %f\n",(targetTime - now).asSeconds());

            // Restart the sync timer
            targetTime = util::Time::now();
        }
    }

    // Sleep for a while
    targetTime += util::Time(i64(1000000000.0 / refreshRate()));
    targetTime.sleepUntil();
}

template <> void
Thread::sleep<Thread::ThreadMode::Pulsed>()
{
    // Set a timeout to prevent the thread from stalling
    auto timeout = util::Time(i64(2000000000.0 / refreshRate()));

    // Wait for the next pulse
    if (!warp) waitForWakeUp(timeout);
}

void
Thread::main()
{
    debug(RUN_DEBUG, "main()\n");
    
    while (++loopCounter) {

        if (isRunning()) {

            switch (getThreadMode()) {

                case ThreadMode::Periodic: execute<ThreadMode::Periodic>(); break;
                case ThreadMode::Pulsed: execute<ThreadMode::Pulsed>(); break;
            }
        }

        if (!warp || !isRunning()) {
            
            switch (getThreadMode()) {

                case ThreadMode::Periodic: sleep<ThreadMode::Periodic>(); break;
                case ThreadMode::Pulsed: sleep<ThreadMode::Pulsed>(); break;
            }
        }
        
        // Are we requested to enter or exit warp mode?
        if (warpChangeRequest.test()) {

            switchWarp(newWarp);
            warpChangeRequest.clear();
            warpChangeRequest.notify_one();
        }

        // Are we requested to enter or exit debug mode?
        if (trackChangeRequest.test()) {

            switchDebug(newTrack);
            trackChangeRequest.clear();
            trackChangeRequest.notify_one();
        }

        // Are we requested to change state?
        if (stateChangeRequest.test()) {

            switchState(newState);
            stateChangeRequest.clear();
            stateChangeRequest.notify_one();
        }

        // Compute the CPU load once in a while
        if (loopCounter % 32 == 0) {
            
            auto used  = loadClock.getElapsedTime().asSeconds();
            auto total = nonstopClock.getElapsedTime().asSeconds();
            
            cpuLoad = used / total;
            
            loadClock.restart();
            loadClock.stop();
            nonstopClock.restart();
        }
    }
}

void
Thread::switchState(ExecutionState newState)
{
    assert(isEmulatorThread());

    while (newState != state) {

        if (state == EXEC_OFF && newState == EXEC_PAUSED) {

            CoreComponent::powerOn();
            state = EXEC_PAUSED;

        } else if (state == EXEC_OFF && newState == EXEC_RUNNING) {

            CoreComponent::powerOn();
            state = EXEC_PAUSED;

        } else if (state == EXEC_PAUSED && newState == EXEC_OFF) {

            CoreComponent::powerOff();
            state = EXEC_OFF;

        } else if (state == EXEC_PAUSED && newState == EXEC_RUNNING) {

            CoreComponent::run();
            state = EXEC_RUNNING;

        } else if (state == EXEC_RUNNING && newState == EXEC_OFF) {

            state = EXEC_PAUSED;
            CoreComponent::pause();

        } else if (state == EXEC_RUNNING && newState == EXEC_PAUSED) {

            state = EXEC_PAUSED;
            CoreComponent::pause();

        } else if (state == EXEC_RUNNING && newState == EXEC_SUSPENDED) {

            state = EXEC_SUSPENDED;

        } else if (state == EXEC_SUSPENDED && newState == EXEC_RUNNING) {

            state = EXEC_RUNNING;

        } else if (newState == EXEC_HALTED) {

            CoreComponent::halt();
            state = EXEC_HALTED;
            return;

        } else {

            // Invalid state transition
            fatalError;
        }

        debug(RUN_DEBUG, "Changed state to %s\n", ExecutionStateEnum::key(state));
    }
}

void
Thread::switchWarp(u8 newState)
{
    assert(isEmulatorThread());

    if (bool(newState) != bool(warp)) {
        CoreComponent::warpOnOff(newState);
    }
    warp = newState;
}

void
Thread::switchDebug(u8 newState)
{
    assert(isEmulatorThread());

    if (bool(newState) != bool(track)) {
        CoreComponent::trackOnOff(newState);
    }
    track = newState;
}

void
Thread::powerOn()
{
    debug(RUN_DEBUG, "powerOn()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (isPoweredOff()) {
        
        // Request a state change and wait until the new state has been reached
        changeStateTo(EXEC_PAUSED);
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
        changeStateTo(EXEC_OFF);
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
        changeStateTo(EXEC_RUNNING);
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
        changeStateTo(EXEC_PAUSED);
    }
}

void
Thread::halt()
{
    assert(!isEmulatorThread());
    
    changeStateTo(EXEC_HALTED);
    join();
}

void
Thread::warpOn(isize source)
{
    assert(source >= 0 && source < 8);

    changeWarpTo(warp | (u8)(1 << source));
}

void
Thread::warpOff(isize source)
{
    assert(source >= 0 && source < 8);
    
    changeWarpTo(warp & ~(u8)(1 << source));
}

void
Thread::trackOn(isize source)
{
    assert(source >= 0 && source < 8);
    
    changeDebugTo(track | (u8)(1 << source));
}

void
Thread::trackOff(isize source)
{
    changeDebugTo(track & ~(u8)(1 << source));
}

void
Thread::changeStateTo(ExecutionState requestedState)
{
    assert(!isEmulatorThread());
    assert(stateChangeRequest.test() == false);

    // Assign new state
    newState = requestedState;

    // Request the change
    stateChangeRequest.test_and_set();
    assert(stateChangeRequest.test() == true);

    // Wait until the change has been performed
    stateChangeRequest.wait(true);
    assert(stateChangeRequest.test() == false);
}

void
Thread::changeWarpTo(u8 value)
{
    assert(!isEmulatorThread());
   assert(warpChangeRequest.test() == false);

    // Assign new state
    newWarp = value;

    // Request the change
    warpChangeRequest.test_and_set();
    assert(warpChangeRequest.test() == true);

    // Wait until the change has been performed
    warpChangeRequest.wait(true);
    assert(warpChangeRequest.test() == false);
}

void
Thread::changeDebugTo(u8 value)
{
    assert(!isEmulatorThread());
    assert(trackChangeRequest.test() == false);

    // Assign new state
    newTrack = value;

    // Request the change
    trackChangeRequest.test_and_set();
    assert(trackChangeRequest.test() == true);

    // Wait until the change has been performed
    trackChangeRequest.wait(true);
    assert(trackChangeRequest.test() == false);
}

void
Thread::wakeUp()
{
    if (getThreadMode() == ThreadMode::Pulsed) util::Wakeable::wakeUp();
}

void
Thread::suspend()
{
    debug(RUN_DEBUG, "Suspending (%ld)...\n", suspendCounter);
    
    if (suspendCounter || isRunning()) {

        suspendCounter++;
        assert(state == EXEC_RUNNING || state == EXEC_SUSPENDED);
        changeStateTo(EXEC_SUSPENDED);
    }
}

void
Thread::resume()
{
    debug(RUN_DEBUG, "Resuming (%ld)...\n", suspendCounter);

    if (suspendCounter && --suspendCounter == 0) {
        
        assert(state == EXEC_SUSPENDED);
        changeStateTo(EXEC_RUNNING);
        run();
    }
}

}
