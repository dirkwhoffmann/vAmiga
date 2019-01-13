// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#ifndef _AMIGA_INC
#define _AMIGA_INC

#include "HardwareComponent.h"
#include "MessageQueue.h"

// General
// #include "MessageQueue.h"

// Sub components
// #include "AmigaMemory.h"


/* A complete virtual Amiga
 * This class is the most prominent one of all. To run the emulator, it is
 * sufficient to create a single object of this type. All subcomponents are
 * created automatically. The public API gives you control over the emulator's
 * behaviour such as running and pausing the emulation. Please note that most
 * subcomponents have their own public API. E.g., to query information from
 * Paula, you need to invoke a public method on c64->paula.
 */
class Amiga : public HardwareComponent {
    
public:
    
    //
    // Hardware components
    //
    
    // The Amiga's virtual memory
    // AmigaMemory mem;
    
    
private:
    
    //
    // Emulator thread
    //
    
    // The currently emulated Amiga model
    AmigaModel model = AMIGA_500;
    
    // The invocation counter for implementing suspend() / resume()
    unsigned suspendCounter = 0;
    
    /* The emulator thread
     * The thread is created when the emulator starts and destroyed when the
     * emulator is paused or shut down.
     */
    pthread_t p = NULL;
    
    /* System timer information
     * Used to match the emulation speed with the speed of a real Amiga.
     */
    mach_timebase_info_data_t tb;
    
    /* Wake-up time of the synchronization timer in nanoseconds.
     * This value is recomputed each time the emulator thread is put to sleep.
     */
    uint64_t nanoTargetTime = 0;
    
    // Indicates that we should always run at max speed.
    bool alwaysWarp = false;
    
    // Indicates that we should run at max speed during disk operations.
    bool warpLoad = true;

    /* Indicates if timing synchronization is currently disabled.
     * The variable is updated in getWarp() according to the following logical
     * equation:
     *
     *     warp == (alwaysWarp || (warpLoad && <Disk DMA is active>)
     */
    bool warp = false;
    
 
    //
    // Message queue
    //
    
    /* Our communication channel to the GUI.
     * Used to communicate with the graphical user interface. The GUI registers
     * a listener and a callback function to retrieve messages.
     */
    MessageQueue queue;
    
    
    //
    // Snapshot storage
    //
    
#if 0
    
private:
    
    // Indicates if snapshots should be taken automatically.
    bool takeAutoSnapshots = true;
    
    /* Time in seconds between two auto-saved snapshots.
     * This value only takes effect if takeAutoSnapshots equals true.
     */
    long autoSnapshotInterval = 3;
    
    // Maximum number of stored snapshots
    static const size_t MAX_SNAPSHOTS = 32;
    
    // Storage for auto-taken snapshots
    vector<Snapshot *> autoSnapshots;
    
    // Storage for user-taken snapshots
    vector<Snapshot *> userSnapshots;

#endif
    
    //
    // Constructing and destructing
    //
    
public:
    
    Amiga();
    ~Amiga();

    //
    // Methods from HardwareComponent
    //

public:

    void _powerOn();
    void _powerOff();
    void _reset();
    void _ping();
    void _dump();

    //
    // Controlling the emulation thread
    //
    
    /* Halts the emulation thread temporarily.
     * Because the emulator is running in a separate thread, the GUI has to
     * halt the emulator before changing it's internal state. This is done by
     * embedding the code inside a suspend / resume block:
     *
     *            suspend();
     *            do something with the internal state;
     *            resume();
     *
     *  It it safe to nest multiple suspend() / resume() blocks.
     */
    void suspend();
    void resume();
  
    
    //
    // Configuring the emulator
    //
    
public:
    
    AmigaModel getModel() { return model; }
    void setModel(AmigaModel m);
    
    
    //
    // Accessing the message queue
    //
    
public:
    
    // Registers a listener callback function.
    void addListener(const void *sender, void(*func)(const void *, int, long) ) {
        queue.addListener(sender, func);
    }
    
    // Removes a listener callback function.
    void removeListener(const void *sender) {
        queue.removeListener(sender);
    }
    
    // Reads a notification from message queue.
    // Returns MSG_NONE if the queue is empty.
    Message getMessage() { return queue.getMessage(); }
    
    // Writes a notification message into message queue
    void putMessage(MessageType msg, uint64_t data = 0) { queue.putMessage(msg, data); }
    
    
    //
    // Running the emulator
    //
    
    public:
    
    /* Checks for missing resources.
     * If an Amiga 500 or Amiga 1000 is emulated, this function always returns
     * true. If you emulate an Amiga 1000, it returns false if no Boot Rom
     * is present.
     */
    bool isReadyToGo();
    
    /* The operational state of the emulator is defined by variable 'power'
     * which is defined in AmigaComponent and variable 'p' which represents
     * the emulator thread. From the four possible combinations, only three
     * are valid:
     *
     *     power     | thread     | State
     *     --------------------------------------
     *     true      | created    | Running
     *     true      | NULL       | Paused
     *     false     | created    | (INVALID)
     *     false     | NULL       | Off
     *
     * The state can be queried by various methods with the following meaning:
     *
     * Methods defined in class HardwareComponent:
     *
     *    isPoweredOn() == true  <==>  'Running' or 'Paused'
     *   isPoweredOff() == true  <==>  'Off'
     *
     * Methods defined in class Amiga:
     *
     *      isRunning() == true  <==>  'Running'
     *       isPaused() == true  <==>  'Paused'
     */
    bool isRunning() { return p != NULL; }
    bool isPaused() { return isPoweredOn() && p == NULL; }
    
    /* The emulator state can be controlled by the following functions:
     *
     *     Function    | Current | Next    | Action
     *     -------------------------------------------------------------------
     *     run()       | Running | Running | none
     *                 | Paused  | Running | create thread
     *                 | Off     | Running | powerOn(), create thread
     *     -------------------------------------------------------------------
     *     halt()      | Running | Paused  | destroy thread
     *                 | Paused  | Paused  | none
     *                 | Off     | Off     | none
     *     -------------------------------------------------------------------
     *     powerOn()   | Running | Running | none
     *                 | Paused  | Paused  | none
     *                 | Off     | Running | powerOn(), create thread
     *     -------------------------------------------------------------------
     *     powerOff()  | Running | Off     | destroy thread, powerOff()
     *                 | Paused  | Off     | powerOff()
     *                 | Off     | Off     | none
     *     -------------------------------------------------------------------
     *     coldStart() | Running | Running | powerOff(), powerOn()
     *                 | Paused  | Running | powerOff(), powerOn()
     *                 | Off     | Running | powerOff(), powerOn()
     *
     */
    
    
    

    

    
    // Returns true if the emulator is not running.
    bool isHalted() { return p == NULL; }
    
    /* Creates and launches the emulator thread.
     * This method is usually called after emulation was stopped by a call to
     * halt() or by reaching a breakpoint.
     * Calling this functions on a running emulator has no effect.
     */
    void run();
    
    /* Terminates the emulator thread.
     * The emulator thread is canceled, but the internal state stays intact.
     * Emulation can be continued any time by a call to run().
     * Calling this functions on a running emulator has no effect.
     */
    void halt();

    // Calls run() or halt(), depending on the current state.
    void runOrHalt() { isRunning() ? halt() : run(); }
    
    /* The thread enter function.
     * This (private) method is invoked when the emulator thread launches. It
     * has to be declared public to make it accessible by the emulator thread.
     */
    void threadStarted();

    /* The Amiga run loop.
     * This function is one of the most prominent ones. It implements the
     * outermost loop of the emulator and therefore the place where emulation
     * starts. If you want to understand how the emulator works, this function
     * should be your starting point.
     */
    void runLoop();
    
    /* The thread exit function.
     * This (private) method is invoked when the emulator thread terminates. It
     * has to be declared public to make it accessible by the emulator thread.
     */
    void threadTerminated();

    
    //
    // Managing the execution thread
    //

public:
    
    /* Getter and setter for 'alwaysWarp'
     * Side effects:
     *   setAlwaysWarp sends a notification message if the value changes.
     */
    bool getAlwaysWarp() { return alwaysWarp; }
    void setAlwaysWarp(bool value);
    
    // Getter and setter for 'warpLoad'
    bool getWarpLoad() { return warpLoad; }
    void setWarpLoad(bool value) { warpLoad = value; }

    /* Updates variable 'warp' and returns the new value.
     * Side effects:
     *   The function sends a notification message if the value changes.
     */
    bool getWarp();
    
    /* Restarts the synchronization timer.
     * This function is invoked at launch time to initialize the timer and
     * reinvoked when the synchronization timer got out of sync.
     */
    void restartTimer();
    
private:
    
    // Converts kernel time to nanoseconds.
    uint64_t abs_to_nanos(uint64_t abs) { return abs * tb.numer / tb.denom; }
    
    // Converts nanoseconds to kernel time.
    uint64_t nanos_to_abs(uint64_t nanos) { return nanos * tb.denom / tb.numer; }

    /* Returns the delay between two frames in nanoseconds.
     * As long as we only emulate PAL machines, the frame rate is 50 Hz
     * and this function returns a constant.
     */
    uint64_t frameDelay() { return uint64_t(1000000000) / 50; }

    /* Puts the emulator thread to sleep.
     * This function makes the emulator thread wait until nanoTargetTime has
     * been reached. It also assigns a new target value to nanoTargetTime.
     */
    void synchronizeTiming();
};

#endif
