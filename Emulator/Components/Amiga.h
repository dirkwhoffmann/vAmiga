// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include "CoreComponent.h"
#include "Agnus.h"
#include "ControlPort.h"
#include "CIA.h"
#include "CPU.h"
#include "Debugger.h"
#include "Defaults.h"
#include "Denise.h"
#include "FloppyDrive.h"
#include "GdbServer.h"
#include "HardDrive.h"
#include "Host.h"
#include "Keyboard.h"
#include "Memory.h"
#include "MsgQueue.h"
#include "OSDebugger.h"
#include "Paula.h"
#include "RegressionTester.h"
#include "RemoteManager.h"
#include "RetroShell.h"
#include "RshServer.h"
#include "RTC.h"
#include "SerialPort.h"
#include "Snapshot.h"
#include "Thread.h"
#include "ZorroManager.h"

namespace vamiga {

/* A complete virtual Amiga. This class is the most prominent one of all. To
 * run the emulator, it is sufficient to create a single object of this type.
 * All subcomponents are created automatically. The public API gives you
 * control over the emulator's behaviour such as running and pausing emulation.
 * Please note that most subcomponents have their own public API. E.g., to
 * query information from Paula, you need to invoke a public method on
 * amiga.paula.
 */
class Amiga final : public CoreComponent, public Inspectable<AmigaInfo> {

    friend class Emulator;
    
    Descriptions descriptions = {{

        .name           = "Amiga",
        .description    = "Commodore Amiga",
        .shell          = "amiga"
    }};

    ConfigOptions options = {

        OPT_VIDEO_FORMAT,
        OPT_WARP_BOOT,
        OPT_WARP_MODE,
        OPT_VSYNC,
        OPT_TIME_LAPSE
    };
    
    // The current configuration
    AmigaConfig config = {};

    /* Result of the latest inspection. In order to update the GUI inspector
     * panels, the emulator schedules events in the inspector slot (SLOT_INS in
     * the secondary table) on a periodic basis. Inside the event handler, the
     * current state is recorded. When the GUI updates the inspector panels, it
     * displays the result of the latest inspection.
     */
    // mutable AmigaInfo info = {};


    //
    // Sub components
    //

public:

    // User settings
    static Defaults defaults;

    // Information about the host system
    Host host = Host(*this);

    // Core components
    CPU cpu = CPU(*this);
    CIAA ciaA = CIAA(*this);
    CIAB ciaB = CIAB(*this);
    Memory mem = Memory(*this);
    Agnus agnus = Agnus(*this);
    Denise denise = Denise(*this);
    Paula paula = Paula(*this);

    // Logic board
    RTC rtc = RTC(*this);
    ZorroManager zorro = ZorroManager(*this);
    ControlPort controlPort1 = ControlPort(*this, 0);
    ControlPort controlPort2 = ControlPort(*this, 1);
    SerialPort serialPort = SerialPort(*this);

    // Floppy drives
    FloppyDrive df0 = FloppyDrive(*this, 0);
    FloppyDrive df1 = FloppyDrive(*this, 1);
    FloppyDrive df2 = FloppyDrive(*this, 2);
    FloppyDrive df3 = FloppyDrive(*this, 3);

    // Hard drives
    HardDrive hd0 = HardDrive(*this, 0);
    HardDrive hd1 = HardDrive(*this, 1);
    HardDrive hd2 = HardDrive(*this, 2);
    HardDrive hd3 = HardDrive(*this, 3);

    // Zorro boards
    HdController hd0con = HdController(*this, hd0);
    HdController hd1con = HdController(*this, hd1);
    HdController hd2con = HdController(*this, hd2);
    HdController hd3con = HdController(*this, hd3);
    RamExpansion ramExpansion = RamExpansion(*this);
    DiagBoard diagBoard= DiagBoard(*this);

    // Other Peripherals
    Keyboard keyboard = Keyboard(*this);

    // Shortcuts
    FloppyDrive *df[4] = { &df0, &df1, &df2, &df3 };
    HardDrive *hd[4] = { &hd0, &hd1, &hd2, &hd3 };
    HdController *hdcon[4] = { &hd0con, &hd1con, &hd2con, &hd3con };

    // Gateway to the GUI
    MsgQueue msgQueue = MsgQueue(*this);

    // Misc
    RetroShell retroShell = RetroShell(*this);
    RemoteManager remoteManager = RemoteManager(*this);
    Debugger debugger = Debugger(*this);
    OSDebugger osDebugger = OSDebugger(*this);
    RegressionTester regressionTester = RegressionTester(*this);


    //
    // Emulator thread
    //

private:

    /* Run loop flags. This variable is checked at the end of each runloop
     * iteration. Most of the time, the variable is 0 which causes the runloop
     * to repeat. A value greater than 0 means that one or more runloop control
     * flags are set. These flags are flags processed and the loop either
     * repeats or terminates depending on the provided flags.
     */
    RunLoopFlags flags = 0;


    //
    // Storage
    //

private:

    Snapshot *autoSnapshot = nullptr;
    Snapshot *userSnapshot = nullptr;

    typedef struct { Cycle trigger; i64 payload; } Alarm;
    std::vector<Alarm> alarms;


    //
    // Static methods
    //

public:

    // Returns a version string for this release
    static string version();

    // Returns a build number string for this release
    static string build();


    //
    // Initializing
    //

public:

    Amiga(class Emulator& ref, isize id);
    ~Amiga();


    //
    // Methods from CoreObject
    //

public:

    void prefix() const override;

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

    void reset(bool hard);
    void hardReset() { reset(true); }
    void softReset() { reset(false); }

private:

    void _reset(bool hard) override;
    void _powerOn() override;
    void _powerOff() override;
    void _run() override;
    void _pause() override;
    void _halt() override;
    void _warpOn() override;
    void _warpOff() override;
    void _trackOn() override;
    void _trackOff() override;

    template <class T>
    void serialize(T& worker)
    {
        if (util::isResetter(worker)) return;

        worker

        << config.type
        << config.warpMode
        << config.warpBoot
        << config.vsync
        << config.timeLapse;
    }

public:

    isize load(const u8 *buffer) override;
    isize save(u8 *buffer) override;

private:

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM; }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(AmigaInfo &result) const override;


    //
    // Methods from Configurable
    //

public:

    const AmigaConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }

    i64 getOption(Option option) const override;
    void setOption(Option option, i64 value) override;

    // Exports the current configuration to a script file
    void exportConfig(const fs::path &path) const;
    void exportConfig(std::ostream& stream) const;

    // Reverts to factory settings
    void revertToFactorySettings();

private:

    // Overrides a config option if the corresponding debug option is enabled
    i64 overrideOption(Option option, i64 value);


    //
    // Analyzing
    //

public:

    // AmigaInfo getInfo() const { return CoreComponent::getInfo(info); }

    void computeFrame();

    InspectionTarget getInspectionTarget() const;
    void setInspectionTarget(InspectionTarget target, Cycle trigger = 0);
    void removeInspectionTarget() { setInspectionTarget(INSPECTION_NONE); }

    // Returns the native refresh rate of the emulated Amiga (50Hz or 60Hz)
    double nativeRefreshRate() const;

    // Returns the native master clock frequency
    i64 nativeMasterClockFrequency() const;

    // Returns the emulated refresh rate
    double refreshRate() const;

    // Returns the master clock frequency based on the emulated refresh rate
    i64 masterClockFrequency() const;

    
    //
    // Running the emulator
    //

public:

    /* Sets or clears a flag for controlling the run loop. The functions are
     * thread-safe and can be called safely from outside the emulator thread.
     */
    void setFlag(u32 flags);
    void clearFlag(u32 flags);

    // Convenience wrappers
    void signalStop() { setFlag(RL::STOP); }
    void signalAutoSnapshot() { setFlag(RL::AUTO_SNAPSHOT); }
    void signalUserSnapshot() { setFlag(RL::USER_SNAPSHOT); }

#if 0
    // Runs or pauses the emulator
    void stopAndGo();

    /* Executes a single instruction. This function is used for single-stepping
     * through the code inside the debugger. It starts the execution thread and
     * terminates it after the next instruction has been executed.
     */
    void stepInto();

    /* Runs the emulator until the instruction following the current one is
     * reached. This function is used for single-stepping through the code
     * inside the debugger. It sets a soft breakpoint to PC+n where n is the
     * length bytes of the current instruction and starts the emulator thread.
     */
    void stepOver();
#endif


    //
    // Handling snapshots
    //

public:

    // Takes a snapshot
    Snapshot *takeSnapshot();

    // Loads the current state from a snapshot file
    void loadSnapshot(const Snapshot &snapshot) throws;

    // Services a snapshot event
    void serviceSnpEvent(EventID id);

private:

    // Schedules the next snapshot event
    void scheduleNextSnpEvent();


    //
    // Handling snapshots
    //

public:

    /* Requests a snapshot to be taken. Once the snapshot is ready, a message
     * is written into the message queue. The snapshot can then be picked up by
     * calling latestAutoSnapshot() or latestUserSnapshot(), depending on the
     * requested snapshot type.
     */
    void requestAutoSnapshot();
    void requestUserSnapshot();

    /* Returns the most recent snapshot or nullptr if none was taken. If a
     * snapshot was taken, the function hands over the ownership to the caller
     * and deletes the internal pointer.
     */
    Snapshot *latestAutoSnapshot();
    Snapshot *latestUserSnapshot();

    // Loads the current state from a snapshot file
    // void loadSnapshot(const Snapshot &snapshot) throws;

private:

    // Takes a snapshot of a certain kind
    void takeAutoSnapshot();
    void takeUserSnapshot();


    //
    // Managing events
    //

public:

    // End-of-line handler
    void eolHandler();

    /* Alarms are scheduled notifications set by the client (GUI). Once the
     * trigger cycle of an alarm has been reached, the emulator sends a
     * MSG_ALARM to the client.
     */
    void setAlarmAbs(Cycle trigger, i64 payload);
    void setAlarmRel(Cycle trigger, i64 payload);

    // Services an alarm event
    void serviceAlarmEvent();

private:

    // Schedules the next alarm event
    void scheduleNextAlarm();


    //
    // Miscellaneous
    //

public:

    // Modifies an internal debug variable (only available in debug builds)
    static void setDebugVariable(const string &name, int val);
};

}
