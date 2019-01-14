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
     * Together with variable 'power' (inherited from HardwareComponent), this
     * variable defines the operational state of the emulator. From four
     * possible combinations, only three are considered valid:
     *
     *     power     | thread     | State
     *     --------------------------------------
     *     true      | created    | Running
     *     true      | NULL       | Paused
     *     false     | created    | (INVALID)
     *     false     | NULL       | Off
     *
     * According to this table, the semantics of function isPoweredOn() and
     * isPoweredOff() can be stated as follows:
     *
     *    isPoweredOn() == true  <==>  'Running' or 'Paused'
     *   isPoweredOff() == true  <==>  'Off'
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

private:

    /* Initializes the internal state and starts the emulator thread.
     * In detail, this function affects the emulator state as follows:
     *
     *                    | State   | Next    | Actions taken
     *     -------------------------------------------------------------------
     *     powerOn()      | Running | Running | none
     *                    | Paused  | Paused  | none
     *                    | Off     | Running | _powerOn(), create thread
     */
    void _powerOn() override;

    /* Destroys the emulator thread.
     * In detail, this function affects the emulator state as follows:
     *
     *                    | State   | Next    | Actions taken
     *     -------------------------------------------------------------------
     *     powerOff()     | Running | Off     | destroy thread, _powerOff()
     *                    | Paused  | Off     | _powerOff()
     *                    | Off     | Off     | none
     */
    void _powerOff() override;
    
    void _reset() override;
    void _ping() override;
    void _dump() override;

    
    //
    // Controlling the emulation thread
    //
    
public:
    
    /* Returns true if a call to powerUp() will be successful.
     * An Amiga 500 or Amiga 2000 can be powered up any time (if no original
     * Kickstart is present, the emulator falls back to using the free Aros
     * replacement). An Amiga 1000 requires a Boot Rom which is not part of
     * the emulator.
     */
    bool readyToPowerUp();
    
    /* Returns true if the emulator is in 'Running' state
     * According to the table above, 'Running' means that the component is
     * powered on and the emulator thread running.
     */
    bool isRunning() { return p != NULL; }
    
    /* Returns true if the emulator is in 'Paused' state
     * According to the table above, 'Running' means that the component is
     * powered on and the emulator thread has not been created yet or it has
     * been destroyed.
     */
    bool isPaused() { return isPoweredOn() && p == NULL; }
    
    /* Puts a 'Paused' emulator into 'Running' state.
     * In detail, this function affects the emulator state as follows:
     *
     *                    | State   | Next    | Actions taken
     *     -------------------------------------------------------------------
     *     run()          | Running | Running | none
     *                    | Paused  | Running | create thread
     *                    | Off     | Off     | none
     */
    void run();
    
    /* Puts a 'Running' emulator into 'Pause' state.
     * In detail, this function affects the emulator state as follows:
     *
     *                    | State   | Next    | Actions taken
     *     -------------------------------------------------------------------
     *     pause()        | Running | Paused  | destroy thread
     *                    | Paused  | Paused  | none
     *                    | Off     | Off     | none
     */
    void pause();
    
    /* Toggles between 'Running' and 'Pause' state.
     * In detail, this function affects the emulator state as follows:
     *
     *                    | State   | Next    | Actions taken
     *     -------------------------------------------------------------------
     *     runOrPause()   | Running | Paused  | destroy thread
     *                    | Paused  | Running | create thread
     *                    | Off     | Off     | none
     */
    void runOrPause() { isRunning() ? pause() : run(); }
    
    
    /* Pauses the emulation thread temporarily.
     * Because the emulator is running in a separate thread, the GUI has to
     * pause the emulator before changing it's internal state. This is done by
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
    
    /* The thread enter function.
     * This (private) method is invoked when the emulator thread launches. It
     * has to be declared public to make it accessible by the emulator thread.
     */
    void threadWillStart();

    /* The thread exit function.
     * This (private) method is invoked when the emulator thread terminates. It
     * has to be declared public to make it accessible by the emulator thread.
     */
    void threadDidTerminate();
    
    /* The Amiga run loop.
     * This function is one of the most prominent ones. It implements the
     * outermost loop of the emulator and therefore the place where emulation
     * starts. If you want to understand how the emulator works, this function
     * should be your starting point.
     */
    void runLoop();
    
  
    
    //
    // Managing emulation speed
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
