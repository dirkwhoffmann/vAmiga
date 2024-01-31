// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ThreadTypes.h"
#include "CoreComponent.h"
#include "Chrono.h"
#include "Concurrency.h"

namespace vamiga {

/* This class manages the emulator thread that runs alongside the GUI. The
 * thread exists during the emulator's lifetime but, depending on the internal
 * state may not always run the emulator.
 *
 * 1. Thread states:
 *
 * The following states are distinguished:
 *
 *        Off: The emulator is turned off
 *     Paused: The emulator is turned on, but not running
 *    Running: The emulator is turned on and running
 *  Suspended: The emulator is paused for a short period of time
 *     Halted: The emulator is shutting down
 *
 *   ---------  powerOn   ---------    run     ---------  suspend   ---------
 *  |   Off   |--------->| Paused  |--------->| Running |--------->|Suspended|
 *  |         |<---------|         |<---------|         |<---------|         |
 *   ---------  powerOff  ---------   pause    ---------   resume   ---------
 *       ^                                         |
 *       |                   powerOff()            |
 *        -----------------------------------------
 *
 *   isPoweredOff          isPaused             isRunning        isSuspended
 *  |-------------||---------------------||--------------------||------------|
 *                 |---------------------------------------------------------|
 *                                       isPoweredOn
 *
 * State changes are triggered by the following functions:
 *
 * Command    | Current   | Next      | Actions on the delegate
 * ------------------------------------------------------------------------
 * powerOn()  | off       | paused    | _powerOn()
 *            | paused    | paused    | none
 *            | running   | running   | none
 *            | suspended | ---       | Error
 * ------------------------------------------------------------------------
 * powerOff() | off       | off       | none
 *            | paused    | off       | _powerOff()
 *            | running   | off       | _powerOff() + _pause()
 *            | suspended | ---       | Error
 * ------------------------------------------------------------------------
 * run()      | off       | ---       | Error
 *            | paused    | running   | _run()
 *            | running   | running   | none
 *            | suspended | ---       | Error
 * ------------------------------------------------------------------------
 * pause()    | off       | off       | none
 *            | paused    | paused    | none
 *            | running   | paused    | _pause()
 *            | suspended | ---       | Error
 * ------------------------------------------------------------------------
 * suspend()  | off       | ---       | Error
 *            | paused    | ---       | Error
 *            | running   | suspended | none
 *            | suspended | suspended | none
 * ------------------------------------------------------------------------
 * resume()   | off       | ---       | Error
 *            | paused    | ---       | Error
 *            | running   | ---       | Error
 *            | suspended | running   | none
 * ------------------------------------------------------------------------
 * halt()     | --        | halted    | _halt()
 *
 * Inside the main() function, the thread executes an infinite loop that
 * periodically calls function execute(). After each iteration, the thread is
 * put to sleep to synchronize timing.
 *
 * 2. Suspend / Resume:
 *
 * The Thread class provides a suspend-resume mechanism for pausing the thread
 * temporarily. The GUI utilizes this functionality to carry out atomic
 * operations that must not be performed while the emulator runs. To pause the
 * emulator temporarily, critical code sections can be embedded in a
 * suspend/resume block like so:
 *
 *       suspend();
 *       do something with the internal state;
 *       resume();
 *
 * It is safe to nest multiple suspend/resume blocks, but it is essential that
 * each call to suspend() is followed by a call to resume(). As a result, the
 * critical code section must not be exited in the middle, e.g., by throwing an
 * exception. It is therefore recommended to use the exit-safe SUSPENDED macro.
 * It is used in the following way:
 *
 *    {  SUSPENDED
 *
 *       Do something with the internal state;
 *       return or throw an exceptions as you like;
 *    }
 *
 * 3. Synchronization:
 *
 * The Thread class is responsible for timing synchronization. I.e., it has to
 * ensure that the proper number of frames is executed per second. Two
 * synchronization modes are supported:
 *
 * - Periodic:
 *
 *   In periodic mode, the thread puts itself to sleep and utilizes a timer to
 *   schedule a wakeup call. The GUI does not have to take further action. This
 *   method was the default mode used by earlier versions of the emulator.
 *
 * - Pulsed:
 *
 *   In pulsed mode, the thread waits for an external wakeup signal the GUI is
 *   supposed to send after each VSYNC tick. When the signal is received, the
 *   thread calls missingSlices(), which has to be provided by the subclass.
 *   This function tells the thread the number of time slices it needs to
 *   compute.
 *
 * The number of time slices per frame controls the size of a single
 * computation chunk. Per default, the emulator computes an entire frame in a
 * single chunk. That is, it computes a frame, sleeps, computes a frame, sleeps,
 * and so on. A frame can be time-sliced to make the emulator more responsive
 * and let it react faster to external events such as joystick movements. For
 * example, if the number of time slices per frame is set to two, the thread
 * computes the first half of the current frame, sleeps, computes the second
 * half of the current frame, sleeps, and so on. Note that an increased number
 * of time slices increases CPU load and jitter, even in pulsed mode. Jitter
 * may increase because time slices are distributed equally between two wake-up
 * events. Hence, later chunks are computed closer to the next wake-up event
 * and may, therefore, interfere with the VSYNC event of the host computer.
 *
 * 4. Warp mode:
 *
 * Emulation can be sped up by activating warp mode (e.g., during disk
 * accesses). In this mode, timing synchronization is disabled, letting the
 * emulator run as fast as possible. Similar to warp mode, the emulator can
 * enter track mode. This mode is enabled when the user opens the GUI debugger
 * and disabled when the debugger is closed. In track mode, several
 * time-consuming tasks are performed that are usually left out. E.g., the CPU
 * tracks all executed instructions and stores the recorded information in a
 * trace buffer.
 */

class Thread : public CoreComponent, util::Wakeable {
    
protected:
    
    friend class Amiga;
    
    // The thread object
    std::thread thread;
    
    // The current thread state and a change request
    ExecutionState state = EXEC_OFF;
    ExecutionState newState = EXEC_OFF;
    std::atomic_flag stateChangeRequest {};

    // Warp state and track state
    u8 warp = 0;
    u8 track = 0;

    // Counters
    isize suspendCounter = 0;
    isize sliceCounter = 0;

    // Time stamps for calculating wakeup times
    util::Time baseTime;
    util::Time deltaTime;
    util::Time targetTime;

    // Number of time slices that need to be computed
    isize missing = 0;
    
    // Clocks for measuring the CPU load
    util::Clock nonstopClock;
    util::Clock loadClock;

    // The current CPU load in percent
    double cpuLoad = 0.0;

    // Debug clocks
    util::Clock execClock;
    util::Clock wakeupClock;

    
    //
    // Initializing
    //

public:
    
    Thread();
    ~Thread();
    
    const char *getDescription() const override { return "Thread"; }

    
    //
    // Executing
    //

private:
    
    // The code to be executed in each iteration (implemented by the subclass)
    virtual void execute() = 0;

    // Interval between two time slices (provided by the subclass)
    virtual util::Time sliceDelay() const = 0;

    // Number of overdue time slices (used in pulsed sync mode)
    virtual isize missingSlices() const = 0;

    // Rectifies an out-of-sync condition by resetting all counters and clocks
    void resync();

    // Executes a single time slice (if one is pending)
    template <SyncMode M> void execute();

    // Suspends the thread until the next time slice is due
    template <SyncMode M> void sleep();

    // The main entry point (called when the thread is created)
    void main();

public:

    // Returns true if this functions is called from within the emulator thread
    bool isEmulatorThread() { return std::this_thread::get_id() == thread.get_id(); }
    
    // Performs a state change
    void switchState(ExecutionState newState);
    void switchWarp(bool state, u8 source = 0);
    void switchTrack(bool state, u8 source = 0);


    //
    // Analyzing
    //

public:
    
    double getCpuLoad() { return cpuLoad; }
    
    
    //
    // Managing states
    //
    
public:
    
    bool isPoweredOn() const override { return state != EXEC_OFF; }
    bool isPoweredOff() const override { return state == EXEC_OFF; }
    bool isPaused() const override { return state == EXEC_PAUSED; }
    bool isRunning() const override { return state == EXEC_RUNNING; }
    bool isSuspended() const override { return state == EXEC_SUSPENDED; }
    bool isHalted() const override { return state == EXEC_HALTED; }

    void suspend() override;
    void resume() override;

    void powerOn();
    void powerOff();
    void run() throws;
    void pause();
    void halt();

    bool isWarping() const { return warp != 0; }
    void warpOn(isize source = 0);
    void warpOff(isize source = 0);

    bool isTracking() const { return track != 0; }
    void trackOn(isize source = 0);
    void trackOff(isize source = 0);

protected:

    // Initiates a state change
    void changeStateTo(ExecutionState requestedState);

    
    //
    // Synchronizing
    //

public:

    // Provides the current sync mode
    virtual SyncMode getSyncMode() const = 0;

    // Awakes the thread if it runs in pulse mode or adaptive mode
    void wakeUp();

private:
    
    // Wait until the thread has terminated
    void join() { if (thread.joinable()) thread.join(); }
};

struct AutoResume {

    CoreComponent *c;
    AutoResume(CoreComponent *c) : c(c) { c->suspend(); }
    ~AutoResume() { c->resume(); }
};

#define SUSPENDED AutoResume _ar(this);

}
