// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include "Agnus.h"
#include "ControlPort.h"
#include "CIA.h"
#include "CPU.h"
#include "Denise.h"
#include "Drive.h"
#include "GdbServer.h"
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
#include "Thread.h"
#include "ZorroManager.h"

/* A complete virtual Amiga. This class is the most prominent one of all. To
 * run the emulator, it is sufficient to create a single object of this type.
 * All subcomponents are created automatically. The public API gives you
 * control over the emulator's behaviour such as running and pausing emulation.
 * Please note that most subcomponents have their own public API. E.g., to
 * query information from Paula, you need to invoke a public method on
 * amiga.paula.
 */
class Amiga : public SuspendableThread {

    /* Result of the latest inspection. In order to update the GUI inspector
     * panels, the emulator schedules events in the inspector slot (SLOT_INS in
     * the secondary table) on a periodic basis. Inside the event handler, the
     * current state is recorded. When the GUI updates the inspector panels, it
     * displays the result of the latest inspection.
     */
    mutable AmigaInfo info = {};

     
    //
    // Sub components
    //
    
public:
    
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
    ControlPort controlPort1 = ControlPort(*this, PORT_1);
    ControlPort controlPort2 = ControlPort(*this, PORT_2);
    SerialPort serialPort = SerialPort(*this);

    // Peripherals
    Keyboard keyboard = Keyboard(*this);
    Drive df0 = Drive(*this, 0);
    Drive df1 = Drive(*this, 1);
    Drive df2 = Drive(*this, 2);
    Drive df3 = Drive(*this, 3);
    
    // Shortcuts to all four drives
    Drive *df[4] = { &df0, &df1, &df2, &df3 };
    
    // Gateway to the GUI
    MsgQueue msgQueue = MsgQueue(*this);

    // Misc
    RetroShell retroShell = RetroShell(*this);
    RemoteManager remoteManager = RemoteManager(*this);
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
    // Snapshot storage
    //
        
private:
    
    class Snapshot *autoSnapshot = nullptr;
    class Snapshot *userSnapshot = nullptr;

    
    //
    // Initializing
    //
    
public:
    
    Amiga();
    ~Amiga();

    
    //
    // Methods from AmigaObject
    //
    
public:

    void prefix() const override;

private:
    
    const char *getDescription() const override { return "Amiga"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
public:
    
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
    void _debugOn() override;
    void _debugOff() override;
    void _inspect() const override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        
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
    // Methods from Thread
    //
    
private:
    
    void execute() override;

    
    //
    // Configuring
    //
    
public:
        
    // Gets a single configuration item
    i64 getConfigItem(Option option) const;
    i64 getConfigItem(Option option, long id) const;
    
    // Sets a single configuration item
    void configure(Option option, i64 value) throws;
    void configure(Option option, long id, i64 value) throws;
    
    // Configures the Amiga with a predefined set of options
    void configure(ConfigScheme scheme);

    // Reverts to factory settings
    void revertToFactorySettings();
    
private:
    
    // Overrides a config option if the corresponding debug option is enabled
    i64 overrideOption(Option option, i64 value);

    
    //
    // Analyzing
    //
    
public:
    
    AmigaInfo getInfo() const { return AmigaComponent::getInfo(info); }
    
    InspectionTarget getInspectionTarget() const;
    void setInspectionTarget(InspectionTarget target, Cycle trigger = 0);
    void removeInspectionTarget() { setInspectionTarget(INSPECTION_NONE); }
        
        
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
    void signalInspect() { setFlag(RL::INSPECT); }
    void signalWarpOn() { setFlag(RL::WARP_ON); }
    void signalWarpOff() { setFlag(RL::WARP_OFF); }
    void signalAutoSnapshot() { setFlag(RL::AUTO_SNAPSHOT); }
    void signalUserSnapshot() { setFlag(RL::USER_SNAPSHOT); }
    
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
    void loadSnapshot(const Snapshot &snapshot) throws;
};
