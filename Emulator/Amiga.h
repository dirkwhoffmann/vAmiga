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

// General
#include "AmigaComponent.h"
#include "Serialization.h"
#include "MessageQueue.h"

// Sub components
#include "Agnus.h"
#include "Blitter.h"
#include "ControlPort.h"
#include "CIA.h"
#include "Copper.h"
#include "CPU.h"
#include "Denise.h"
#include "Disk.h"
#include "Drive.h"
#include "Joystick.h"
#include "Keyboard.h"
#include "Memory.h"
#include "Moira.h"
#include "Mouse.h"
#include "RTC.h"
#include "Paula.h"
#include "SerialPort.h"
#include "ZorroManager.h"

// File types
#include "RomFile.h"
#include "ExtFile.h"
#include "Snapshot.h"
#include "ADFFile.h"

/* A complete virtual Amiga
 * This class is the most prominent one of all. To run the emulator, it is
 * sufficient to create a single object of this type. All subcomponents are
 * created automatically. The public API gives you control over the emulator's
 * behaviour such as running and pausing the emulation. Please note that most
 * subcomponents have their own public API. E.g., to query information from
 * Paula, you need to invoke a public method on amiga.paula.
 */
class Amiga : public HardwareComponent {

    // Information shown in the GUI inspector panel
    AmigaInfo info;

    // Information shown in the GUI monitor panel
    AmigaStats stats;

public:

    /* Inspection target
     * To update the GUI periodically, the emulator schedules this event in the
     * inspector slot (INS_SLOT in the secondary table) on a periodic basis.
     * If the event is EVENT_NONE, no action is taken. If an INS_xxx event is
     * scheduled, inspect() is called on a certain Amiga component.
     */
    static EventID inspectionTarget;
    
private:
    
    /* Indicates if the Amiga should be executed in debug mode.
     * Debug mode is enabled when the GUI debugger is opend and disabled when
     * the GUI debugger is closed. In debug mode, several time-consuming tasks
     * are performed that are usually left out. E.g., the CPU checks for
     * breakpoints and records the executed instruction in it's trace buffer.
     */
    static bool debugMode;
    
    
    //
    // Sub components
    //
    
public:
    
    // Core components
    CPU cpu = CPU(*this);
    CIAA ciaA = CIAA(*this);
    CIAB ciaB = CIAB(*this);
    RTC rtc = RTC(*this);
    Memory mem = Memory(*this);
    Agnus agnus = Agnus(*this);
    Denise denise = Denise(*this);
    Paula paula = Paula(*this);
    ZorroManager zorro = ZorroManager(*this);
    
    // Ports
    ControlPort controlPort1 = ControlPort(1, *this);
    ControlPort controlPort2 = ControlPort(2, *this);
    SerialPort serialPort = SerialPort(*this);

    // Peripherals
    Mouse mouse = Mouse(*this);
    Joystick joystick1 = Joystick(1, *this);
    Joystick joystick2 = Joystick(2, *this);
    Keyboard keyboard = Keyboard(*this);
    
    // Floppy drives
    Drive df0 = Drive(0, *this);
    Drive df1 = Drive(1, *this);
    Drive df2 = Drive(2, *this);
    Drive df3 = Drive(3, *this);
    
    // Shortcuts to all four drives
    Drive *df[4] = { &df0, &df1, &df2, &df3 };
    

    //
    // Emulator thread
    //
    
public:
    
    /* Run loop control
     * This variable is checked at the end of each runloop iteration. Most of
     * the time, the variable is 0 which causes the runloop to repeat. A value
     * greater than 0 means that one or more runloop control flags are set.
     * These flags are flags processed and the loop either repeats or
     * terminates, depending on the set flags.
     */
    u32 runLoopCtrl = 0;
    
private:
    
    // The invocation counter for implementing suspend() / resume()
    unsigned suspendCounter = 0;
    
    // The emulator thread
    pthread_t p = NULL;
    
    
    //
    // Emulation speed
    //
    
private:
    
    /* System timer information
     * Used to match the emulation speed with the speed of a real Amiga.
     */
    mach_timebase_info_data_t tb;
    
    /* Inside restartTimer(), the current time and the DMA clock cylce
     * are recorded in these variables. They are used in sychronizeTiming()
     * to determine how long the thread has to sleep.
     */
    Cycle clockBase = 0;
    u64 timeBase = 0;

    
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
    
    
    //
    // Debugging
    //
    
public:
    
    bool debugDMA = false; // REMOVE AFTER DEBUGGING
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Amiga();
    ~Amiga();

    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clockBase;
    }


    //
    // Configuring
    //

    // Returns the currently set configuration.
    AmigaConfiguration getConfig();

    // Changes the configuration
    bool configure(ConfigOption option, long value);
    bool configureDrive(unsigned drive, ConfigOption option, long value);


    //
    // Methods from AmigaObject and HardwareComponent
    //

public:

    void prefix() override;
    void reset() override;

    void powerOn() override;
    void run() override;

private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _run() override;
    void _pause() override;
    void _reset() override { RESET_SNAPSHOT_ITEMS }
    void _ping() override;
    void _inspect() override;
    void _dump() override;
    void _warpOn() override;
    void _warpOff() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

public:

    // Returns the result of the most recent call to inspect()
    AmigaInfo getInfo();

    // Returns the statistical information for the most recently finished frame
    AmigaStats getStats();

    // Updates the statistical information returned by getStats()
    void updateStats();

    // Clears all previously recorded statistical information
    void clearStats();

    //
    // Accessing properties
    //

public:
    
    // Indicates if debug mode is enabled.
    bool getDebugMode() { return debugMode; }
    
    // Enabled or disabled debug mode.
    void setDebugMode(bool enable);
    void enableDebugging() { setDebugMode(true); }
    void disableDebugging() { setDebugMode(false); }
    
    /* Sets the inspection target.
     * If an inspection target is set (INS_xxx events), the emulator schedules
     * periodic calls to inspect().
     */
    void setInspectionTarget(EventID id);

    // Removed the currently set inspection target
    void clearInspectionTarget();
    

    //
    // Controlling the emulation thread
    //
    
public:
    
    /* Returns true if a call to powerUp() will be successful.
     * It returns false, e.g., if no Kickstart Rom or Boot Rom is installed.
     */
    ErrorCode readyToPowerOn();
    
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
    
    /* Sets or clears a run loop control flag
     * The functions are thread-safe and can be called from inside or outside
     * the emulator thread.
     */
    void setControlFlags(u32 flags);
    void clearControlFlags(u32 flags);
    
    // Convenience wrappers for controlling the run loop
    void signalSnapshot() { setControlFlags(RL_SNAPSHOT); }
    void signalInspect() { setControlFlags(RL_INSPECT); }
    void signalStop() { setControlFlags(RL_STOP); }

    
    //
    // Accessing the message queue
    //
    
public:
    
    // Registers a listener callback function.
    //     void addListener(const void *sender, void(*func)(const void *, int, long) ) {
    void addListener(const void *sender, Callback func) {
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
    void putMessage(MessageType msg, u64 data = 0) { queue.putMessage(msg, data); }
    
    
    //
    // Running the emulator
    //
    
public:
    
    // Runs or pauses the emulator
    void stopAndGo() { isRunning() ? pause() : run(); }
    
    /* Executes a single instruction
     * This function is used for single-stepping through the code inside the
     * debugger. It starts the execution thread and terminates it after the
     * next instruction has been executed.
     */
    void stepInto();
    
    /* Executes until the instruction following the current one is reached.
     * This function is used for single-stepping through the code inside the
     * debugger. It sets a soft breakpoint to PC+n where n is the length
     * bytes of the current instruction and starts the execution thread.
     */
    void stepOver();
    
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

    /* Restarts the synchronization timer.
     * This function is invoked at launch time to initialize the timer and
     * reinvoked when the synchronization timer got out of sync.
     */
    void restartTimer();
    
private:
    
    // Converts kernel time to nanoseconds.
    u64 abs_to_nanos(u64 abs) { return abs * tb.numer / tb.denom; }
    
    // Converts nanoseconds to kernel time.
    u64 nanos_to_abs(u64 nanos) { return nanos * tb.denom / tb.numer; }
    
    // Returns the current time in nanoseconds.
    u64 time_in_nanos() { return abs_to_nanos(mach_absolute_time()); }
    
    /* Returns the delay between two frames in nanoseconds.
     * As long as we only emulate PAL machines, the frame rate is 50 Hz
     * and this function returns a constant.
     */
    u64 frameDelay() { return u64(1000000000) / 50; }
    
public:
    
    // Puts the emulator thread to sleep.
    void synchronizeTiming();
    
    
    //
    // Handling snapshots
    //
    
public:
    
    // Returns true if an auto-snapshot should be taken in the current frame.
    bool snapshotIsDue();
    
    // Indicates if the auto-snapshot feature is enabled.
    bool getTakeAutoSnapshots() { return takeAutoSnapshots; }
    
    // Enables or disabled the auto-snapshot feature.
    void setTakeAutoSnapshots(bool enable) { takeAutoSnapshots = enable; }
    
    /* Disables the auto-snapshot feature temporarily.
     * This method is called when the snaphshot browser opens.
     */
    void suspendAutoSnapshots() { autoSnapshotInterval -= (LONG_MAX / 2); }
    
    /* Heal a call to suspendAutoSnapshots()
     * This method is called when the snaphshot browser closes.
     */
    void resumeAutoSnapshots() { autoSnapshotInterval += (LONG_MAX / 2); }
    
    // Returns the time between two auto-snapshots in seconds.
    long getSnapshotInterval() { return autoSnapshotInterval; }
    
    // Sets the time between two auto-snapshots in seconds.
    void setSnapshotInterval(long value) { autoSnapshotInterval = value; }
    
    /* Loads the current state from a snapshot file
     * There is an thread-unsafe and thread-safe version of this function. The
     * first one can be unsed inside the emulator thread or from outside if the
     * emulator is halted. The second one can be called any time.
     */
    void loadFromSnapshotUnsafe(Snapshot *snapshot);
    void loadFromSnapshotSafe(Snapshot *snapshot);
    
    // Restores a certain snapshot from the snapshot storage
    bool restoreSnapshot(vector<Snapshot *> &storage, unsigned nr);
    bool restoreAutoSnapshot(unsigned nr);
    bool restoreUserSnapshot(unsigned nr);
    
    // Restores the latest snapshot from the snapshot storage
    bool restoreLatestAutoSnapshot() { return restoreAutoSnapshot(0); }
    bool restoreLatestUserSnapshot() { return restoreUserSnapshot(0); }
    
    // Returns the number of stored snapshots
    size_t numSnapshots(vector<Snapshot *> &storage);
    size_t numAutoSnapshots() { return numSnapshots(autoSnapshots); }
    size_t numUserSnapshots() { return numSnapshots(userSnapshots); }
    
    // Returns an snapshot from the snapshot storage
    Snapshot *getSnapshot(vector<Snapshot *> &storage, unsigned nr);
    Snapshot *autoSnapshot(unsigned nr) { return getSnapshot(autoSnapshots, nr); }
    Snapshot *userSnapshot(unsigned nr) { return getSnapshot(userSnapshots, nr); }
    
    /* Takes a snapshot and inserts it into the snapshot storage
     * The new snapshot is inserted at position 0 and all others are moved one
     * position up. If the buffer is full, the oldest snapshot is deleted. Make
     * sure to call the 'Safe' version outside the emulator thread.
     */
    void takeSnapshot(vector<Snapshot *> &storage);
    void takeAutoSnapshot();
    void takeUserSnapshot();
    void takeAutoSnapshotSafe() { suspend(); takeAutoSnapshot(); resume(); }
    void takeUserSnapshotSafe() { suspend(); takeUserSnapshot(); resume(); }
    
    // Deletes a snapshot from the snapshot storage
    void deleteSnapshot(vector<Snapshot *> &storage, unsigned nr);
    void deleteAutoSnapshot(unsigned nr) { deleteSnapshot(autoSnapshots, nr); }
    void deleteUserSnapshot(unsigned nr) { deleteSnapshot(userSnapshots, nr); }
    
    
    //
    // Debugging the emulator
    //
    
    void dumpClock();
    
};

#endif
