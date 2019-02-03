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
#include "HardwareComponent.h"
#include "MessageQueue.h"

// Sub components
#include "EventHandler.h"
#include "CPU.h"
#include "CIA.h"
#include "AmigaMemory.h"
#include "DMAController.h"
#include "Denise.h"
#include "Paula.h"
#include "AmigaKeyboard.h"
#include "AmigaControlPort.h"
#include "DiskController.h"
#include "AmigaDrive.h"
#include "AmigaDisk.h"

// File types
#include "BootRom.h"
#include "KickRom.h"
#include "AmigaSnapshot.h"
#include "ADFFile.h"


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
    
    // Specification of the machine we are going to emulate...
    AmigaConfiguration config;
    
    // Boot Rom (needed for A1000 emulation)
    BootRom *bootRom = NULL;
    
    // Kickstart Rom (needed for A500 and A2000 emulation)
    KickRom *kickRom = NULL;
    
    
    //
    // Hardware components
    //
    
    /* The Amiga's master clock
     * This clock runs at 28 MHz and is used to derive all other clock signals.
     */
    uint64_t masterClock = 0; 
    
    // The event handler
    EventHandler eventHandler;
    
    // A Motorola 68000 CPU
    CPU cpu;
    
    // CIA A (odd CIA)
    CIAA ciaA;

    // CIA B (even CIA)
    CIAB ciaB;

    // Memory
    AmigaMemory mem;
    
    // The DMA controller (part of Agnus)
    DMAController dma;

    // Denise (Video)
    Denise denise;

    // Paula (Interrupts, Disk Controller, Audio)
    Paula paula;

    // Disk controller (move to Paula?!)
    DiskController diskController;
    
    // Control ports
    AmigaControlPort controlPort1 = AmigaControlPort(1);
    AmigaControlPort controlPort2 = AmigaControlPort(2);
    
    // Keyboard
    AmigaKeyboard keyboard;

    // Internal drive
    AmigaDrive df0 = AmigaDrive(0);

    // First external drive
    AmigaDrive df1 = AmigaDrive(1);

    
    //
    // Emulator thread
    //
    
    private:
    
    /* A boolean flag for terminating the emulation thread
     * This flag is periodically queried inside the run loop. When it is set to
     * true, the thread terminates.
     */
    bool stop = false;
    
    // The invocation counter for implementing suspend() / resume()
    unsigned suspendCounter = 0;
    
    // The emulator thread
    pthread_t p = NULL;
    
    
    //
    // Emulation speed
    //
    
    /* System timer information
     * Used to match the emulation speed with the speed of a real Amiga.
     */
    mach_timebase_info_data_t tb;
    
    /* Inside restartTimer(), the current time and the master clock cylce
     * are recorded in these variables. They are used in sychronizeTiming()
     * to determine how long the thread has to sleep.
     */
    uint64_t clockBase = 0;
    uint64_t timeBase = 0;
    
    /* Inside sychronizeTiming(), the current time and master clock cycle
     * are stored in these variables. They are used in combination with
     * clockBase and timeBase to determine how long the thread has to sleep.
     */
    // 
     
    /* Wake-up time of the synchronization timer in nanoseconds.
     * This value is recomputed each time the emulator thread is put to sleep.
     * DEPRECATED
     */
    uint64_t nanoTargetTime = 0;
    
    
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
    vector<AmigaSnapshot *> autoSnapshots;
    
    // Storage for user-taken snapshots
    vector<AmigaSnapshot *> userSnapshots;

    
    //
    // Constructing and destructing
    //
    
public:
    
    Amiga();
    ~Amiga();

    //
    // Configuring the emulated machine
    //
    
public:
    
    // Returns the currently set configuration.
    AmigaConfiguration getConfig() { return config; }
    
    // Chooses the emulated Amiga model.
    bool configureModel(AmigaModel model);
    bool configureLayout(long value);

    // Chooses the amount of memory to emulate.
    bool configureChipMemory(long size);
    bool configureSlowMemory(long size);
    bool configureFastMemory(long size);
    
    // Chooses if a real-time clock should be emulated.
    bool configureRealTimeClock(bool value);
    
    // Chooses if a drive is connected or not.
    bool configureDrive(unsigned driveNr, bool connected);
    bool configureDrive(unsigned driveNr, DriveType type);

    
    //
    // Methods from HardwareComponent
    //

private:

    void _powerOn() override;
    void _powerOff() override;
    void _run() override;
    void _pause() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    void _setWarp(bool value) override;
    
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
    // Handling Roms
    //

    // Returns true if a Boot Rom is present.
    bool hasBootRom() { return bootRom != NULL; }

    // Deletes the current Boot Rom.
    void deleteBootRom();
    
    // Adds the Boot Rom to the emulator.
    bool loadBootRom(BootRom *rom);
    bool loadBootRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadBootRomFromFile(const char *path);

    // Returns a fingerprint for the current Boot Rom
    uint64_t bootRomFingerprint() { return bootRom ? bootRom->fingerprint() : 0; }
    
    // Returns true if a Kickstart Rom is present.
    bool hasKickRom() { return kickRom != NULL; }

    // Deletes the current Kickstart Rom.
    void deleteKickRom();
    
    // Adds the Kickstart Rom to the emulator.
    bool loadKickRom(KickRom *rom);
    bool loadKickRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadKickRomFromFile(const char *path);

    // Returns a fingerprint for the current Kickstart Rom
    uint64_t kickRomFingerprint() { return kickRom ? kickRom->fingerprint() : 0; }

    
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
    // bool getAlwaysWarp() { return alwaysWarp; }
    // void setAlwaysWarp(bool value);
    
    // Getter and setter for 'warpLoad'
    // bool getWarpLoad() { return warpLoad; }
    // void setWarpLoad(bool value) { warpLoad = value; }

    /* Updates variable 'warp' and returns the new value.
     * Side effects:
     *   The function sends a notification message if the value changes.
     */
    // bool getWarp();
    
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
    
    // Returns the current time in nanoseconds.
    uint64_t time_in_nanos() { return abs_to_nanos(mach_absolute_time()); }
    
    /* Returns the delay between two frames in nanoseconds.
     * As long as we only emulate PAL machines, the frame rate is 50 Hz
     * and this function returns a constant.
     */
    uint64_t frameDelay() { return uint64_t(1000000000) / 50; }
    
public:
    
    /* Puts the emulator thread to sleep.
     * This function makes the emulator thread wait until nanoTargetTime has
     * been reached. It also assigns a new target value to nanoTargetTime.
     */
    void synchronizeTiming();
    
    
    //
    // Handling snapshots
    //
    
public:
    
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
    void loadFromSnapshotUnsafe(AmigaSnapshot *snapshot);
    void loadFromSnapshotSafe(AmigaSnapshot *snapshot);
    
    // Restores a certain snapshot from the snapshot storage
    bool restoreSnapshot(vector<AmigaSnapshot *> &storage, unsigned nr);
    bool restoreAutoSnapshot(unsigned nr) { return restoreSnapshot(autoSnapshots, nr); }
    bool restoreUserSnapshot(unsigned nr) { return restoreSnapshot(userSnapshots, nr); }
    
    // Restores the latest snapshot from the snapshot storage
    bool restoreLatestAutoSnapshot() { return restoreAutoSnapshot(0); }
    bool restoreLatestUserSnapshot() { return restoreUserSnapshot(0); }
    
    // Returns the number of stored snapshots
    size_t numSnapshots(vector<AmigaSnapshot *> &storage);
    size_t numAutoSnapshots() { return numSnapshots(autoSnapshots); }
    size_t numUserSnapshots() { return numSnapshots(userSnapshots); }
    
    // Returns an snapshot from the snapshot storage
    AmigaSnapshot *getSnapshot(vector<AmigaSnapshot *> &storage, unsigned nr);
    AmigaSnapshot *autoSnapshot(unsigned nr) { return getSnapshot(autoSnapshots, nr); }
    AmigaSnapshot *userSnapshot(unsigned nr) { return getSnapshot(userSnapshots, nr); }
    
    /* Takes a snapshot and inserts it into the snapshot storage
     * The new snapshot is inserted at position 0 and all others are moved one
     * position up. If the buffer is full, the oldest snapshot is deleted. Make
     * sure to call the 'Safe' version outside the emulator thread.
     */
    void takeSnapshot(vector<AmigaSnapshot *> &storage);
    void takeAutoSnapshot() { takeSnapshot(autoSnapshots); }
    void takeUserSnapshot() { takeSnapshot(userSnapshots); }
    void takeAutoSnapshotSafe() { suspend(); takeSnapshot(autoSnapshots); resume(); }
    void takeUserSnapshotSafe() { suspend(); takeSnapshot(userSnapshots); resume(); }
    
    // Deletes a snapshot from the snapshot storage
    void deleteSnapshot(vector<AmigaSnapshot *> &storage, unsigned nr);
    void deleteAutoSnapshot(unsigned nr) { deleteSnapshot(autoSnapshots, nr); }
    void deleteUserSnapshot(unsigned nr) { deleteSnapshot(userSnapshots, nr); }
    
};

#endif
