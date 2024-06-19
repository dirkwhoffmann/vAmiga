// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmigaTypes.h"
#include "Error.h"
#include <filesystem>

// REMOVE:
#include "Media.h"

namespace vamiga {

namespace moira { class Guards; }

//
// Base class for all APIs
//

class API {
    
public:
    
    class Emulator *emu = nullptr;
    
    API() { }
    API(Emulator *emu) : emu(emu) { }
    
    void suspend();
    void resume();
    
    bool isUserThread() const;
};

//
// Public APIs
//

struct DefaultsAPI : API {

    class Defaults *defaults = nullptr;

    DefaultsAPI(Defaults *defaults) : defaults(defaults) { }
};

struct AmigaAPI : API {

    class Amiga *amiga = nullptr;

    /// @}
    /// @name Handling snapshots
    /// @{

    /** @brief  Takes a snapshot
     *
     *  @return A pointer to the created Snapshot object.
     *
     *  @note   The function transfers the ownership to the caller. It is
     *          his responsibility of the caller to free the object.
     */
    Snapshot *takeSnapshot();

    /** @brief  Loads a snapshot into the emulator.
     *
     *  @param  snapshot    Reference to a snapshot.
     */
    void loadSnapshot(const Snapshot &snapshot);
};

struct AgnusAPI : API {

    class Agnus *agnus = nullptr;
};

struct BlitterAPI : API {

    class Blitter *blitter = nullptr;
};

struct CIAAPI : API {

    class CIA *cia = nullptr;
};

struct CopperAPI : API {

    class Copper *copper = nullptr;
};

struct CPUAPI : API {

    class CPU *cpu = nullptr;
};

struct DebuggerAPI : API {

    class Debugger *debugger = nullptr;
};

struct DeniseAPI : API {

    class Denise *denise = nullptr;
};

struct DiskControllerAPI : API {

    class DiskController *diskController = nullptr;
};

struct DmaDebuggerAPI : API {

    class DmaDebugger *dmaDebugger = nullptr;
};

struct FloppyDriveAPI : API {

    class FloppyDrive *drive = nullptr;
};

struct GuardsAPI : API {

    class moira::Guards *guards = nullptr;
};

struct HardDriveAPI : API {

    class HardDrive *drive = nullptr;
};

struct HostAPI : API {

    class Host *host = nullptr;
};

struct JoystickAPI : API {

    class Joystick *joystick = nullptr;
};

struct KeyboardAPI : API {

    class Keyboard *keyboard = nullptr;
};

struct MemoryAPI : API {

    class Memory *mem = nullptr;
};

struct MouseAPI : API {

    class Mouse *mouse = nullptr;
};

struct PaulaAPI : API {

    class Paula *paula = nullptr;
};

struct RetroShellAPI : API {

    class RetroShell *retroShell = nullptr;
};

struct RtcAPI : API {

    class RTC *rtc = nullptr;
};

struct RecorderAPI : API {

    class Recorder *recorder = nullptr;
};

struct RemoteManagerAPI : API {

    class RemoteManager *remoteManager = nullptr;
};

struct SerialPortAPI : API {

    class SerialPort *serialPort = nullptr;
};

struct ControlPortAPI : API {

    class ControlPort *controlPort = nullptr;

    JoystickAPI joystick;
    MouseAPI mouse;
};

class VAmiga : public API {

public:

    static DefaultsAPI defaults;

    AmigaAPI amiga;
    AgnusAPI agnus;
    BlitterAPI blitter;
    GuardsAPI breakpoints;
    CIAAPI ciaA, ciaB;
    ControlPortAPI controlPort1;
    ControlPortAPI controlPort2;
    CopperAPI copper;
    GuardsAPI copperBreakpoints;
    CPUAPI cpu;
    DebuggerAPI debugger;
    DeniseAPI denise;
    DiskControllerAPI diskController;
    DmaDebuggerAPI dmaDebugger;
    FloppyDriveAPI df0, df1, df2, df3;
    HardDriveAPI hd0,hd1, hd2, hd3;
    HostAPI host;
    KeyboardAPI keyboard;
    MemoryAPI mem;
    PaulaAPI paula;
    RetroShellAPI retroShell;
    RtcAPI rtc;
    RecorderAPI recorder;
    RemoteManagerAPI remoteManager;
    SerialPortAPI serialPort;
    GuardsAPI watchpoints;

    //
    // Static methods
    //
    
public:
    
    /** @brief  Returns a version string for this release.
     */
    static string version();

    /** @brief  Returns a build-number string for this release.
     */
    static string build();

    
    //
    // Initializing
    //
    
public:
    
    VAmiga();
    ~VAmiga();


    /// @}
    /// @name Querying the emulator state
    /// @{

    /** @brief  Returns true iff the emulator if the emulator is powered on.
     */
    bool isPoweredOn();

    /** @brief  Returns true iff the emulator if the emulator is powered off.
     */
    bool isPoweredOff();

    /** @brief  Returns true iff the emulator is in paused state.
     */
    bool isPaused();

    /** @brief  Returns true iff the emulator is running.
     */
    bool isRunning();

    /** @brief  Returns true iff the emulator has been suspended.
     */
    bool isSuspended();

    /** @brief  Returns true iff the emulator has shut down.
     */
    bool isHalted();

    /** @brief  Returns true iff warp mode is active.
     */
    bool isWarping();

    /** @brief  Returns true iff the emulator runs in track mode.
     */
    bool isTracking();

    /** @brief  Checks if the emulator is runnable.
     *  The function checks if the necessary ROMs are installed to lauch the
     *  emulator. On success, the functions returns. Otherwise, an exception
     *  is thrown.
     *
     *  @throw  Error (ERROR_ROM_BASIC_MISSING)
     *  @throw  Error (ERROR_ROM_CHAR_MISSING)
     *  @throw  Error (ERROR_ROM_KERNAL_MISSING)
     *  @throw  Error (ERROR_ROM_CHAR_MISSING)
     *  @throw  Error (ERROR_ROM_MEGA65_MISMATCH)
     */
    void isReady();


    /// @}
    /// @name Controlling the emulator state
    /// @{

    /** @brief  Switches the emulator on
     *
     *  Powering on the emulator changes the interal state to #STATE\_PAUSED,
     *  the same state that is entered when the user hits the pause button.
     *  Calling this function on an already powered-on emulator has no effect.
     *  */
    void powerOn();

    /** @brief  Switches the emulator off
     *
     *  Powering off the emulator changes the interal state of #STATE\_OFF.
     *  Calling this function on an already powered-off emulator has no effect.
     */
    void powerOff();

    /** @brief  Starts emulation
     *
     *  Running the emulator changes the internal state to #STATE\_RUNNING,
     *  which is the normal operation state. Frames are computed at normal
     *  pace, if warp mode is switched off, or as fast as possible, if warp
     *  mode is switched on. If this function is called for a powere-off
     *  emulator, an implicit call to powerOn() will be performed.
     */
    void run();

    /** @brief   Pauses emulation
     *
     * Pausing the emulator changes the interal state from #STATE\_RUNNING
     * to #STATE\_PAUSED after completing the curent frame. The emulator
     * enteres a frozes state where no more frames are computed.
     */
    void pause();

    /** @brief   Terminates the emulator thread
     *
     *  Halting the emulator changes the internal state to #STATE\_HALTED.
     *  This state is part of the shutdown procedure and never entered during
     *  normal operation.
     */
    void halt();

    /** @brief   Suspends the emulator thread
     *
     *  See the \ref vc64::Suspendable "Suspendable" class for a detailes
     *  description of the suspend-resume machanism.
     */
    void suspend();

    /** @brief   Suspends the emulator thread
     *
     *  See the \ref vc64::Suspendable "Suspendable" class for a detailes
     *  description of the suspend-resume machanism.
     */
    void resume();

    /** @brief  Enables warp mode.
     */
    void warpOn(isize source = 0);

    /** @brief  Disables warp mode.
     */
    void warpOff(isize source = 0);

    /** @brief  Enables track mode.
     */
    void trackOn(isize source = 0);

    /** @brief  Disables track mode.
     */
    void trackOff(isize source = 0);

    
    /// @}
    /// @name Single-stepping
    /// @{

    /** @brief  Steps a single instruction
     *
     *  If the emulator is paused, calling this functions steps the CPU for
     *  a single instruction. Otherwise, calling this function
     *  has no effect. The CPU debugger utilizes this function to implement single
     *  stepping.
     */
    void stepInto();

    /** @brief  Steps over the current instruction
     *
     *  If the emulator is paused, calling this functions runs the emulator
     *  until the instruction following the current
     *  instruction has been reached. Otherwise, calling this function
     *  has no effect. The CPU debugger utilizes this function to implement single
     *  stepping.
     *
     *  stepOver() acts like stepInto() except for branching instructions.
     *  For instance, if the current instruction is a `JSR` instruction, the
     *  subroutine will be executed in a single chunk.
     *
     *  @note If the current instruction is a branch instruction which never
     *  returns, the emulator will remain in running state. Calling this
     *  function will then be equivalent to a calling run(), with the exception
     *  of a small runtime overhead since the emulator will check after the
     *  each instruction if the program counter has reached it's target
     *  location.
     */
    void stepOver();


    /// @}
    /// @name Synchronizing the emulator thread
    /// @{

    /** @brief  Sends a wakeup signal to the emulator thread.
     *
     *  To compute frames at the proper pace, the emulator core expects the GUI
     *  to send a wakeup signal on each VSYNC pulse. Once this signal is
     *  received, the emulator thread starts computing all missing frames. To
     *  minimize jitter, the wakeup signal should be sent right after the
     *  current texture has been handed over to the GPU.
     */
    void wakeUp();


    /// @}
    /// @name Configuring the emulator
    /// @{

    /** @brief  Launches the emulator thread.
     *
     *  This function must be called in the initialization procedure of the
     *  emulator. It launches the emulator thread and registers a callback
     *  function to the message queue. The callback function is a message
     *  receiver processing all incoming messages from the emulator.
     *
     *  @param  listener    An arbitrary pointer which will be passed to the
     *  registered callback function as first argument. You can use the argument
     *  to pass the address of an object into the callback function. Inside the
     *  callback you can utilize the pointer to invoke a message processing
     *  function on that object.
     *  @param  func        The callback function.
     */
    void launch(const void *listener, Callback *func);

    /** @brief  Queries a configuration option.
     *
     *  This is the main function to query a configuration option.
     *
     *  @param option   The option to query
     *
     *  @note Some options require an additional parameter to uniquely
     *  determine the configured component. For those options, this function
     *  must not be called.
     */
    i64 get(Option option) const;

    /** @brief  Queries a configuration option.
     *
     *  This is the main function to query a configuration option.
     *
     *  @param option   The option to query
     *  @param id       The component to query
     *
     *  @note This function must only be called for those options that require
     *  an additional parameter to uniquely determine the configured component.
     */
    i64 get(Option option, long id) const throws;

    /** Configures the emulator to match a specific Amiga model
     *
     *  @param model    The Amiga model to emulate
     */
    void set(ConfigScheme model);

    /** @brief  Configures a component.
     *
     *  This is the main function to set an configuration option.
     *
     *  @param opt      The option to set
     *  @param value    The option's value
     *
     *  @note If this function is called for an options that applies to multiple
     *  components, all components are configured with the specified value.
     */
    void set(Option opt, i64 value) throws;

    /** @brief  Configures a component.
     *
     *  This is the main function to set an configuration option.
     *
     *  @param opt      The option to set
     *  @param id       The component to configure
     *  @param value    The option's value
     *
     *  @note This function must only be called for those options that require
     *  an additional parameter to uniquely determine the configured component.
     */
    void set(Option opt, i64 value, long id) throws;


    /** @brief  Exports the current configuration.
     *
     *  The current configuration is exported in form of a RetroShell script.
     *  Reading in the script at a later point will restore the configuration.
     */
    void exportConfig(const std::filesystem::path &path) const;
    void exportConfig(std::ostream& stream) const;


    /// @}
    /// @name Using the command queue
    /// @{

    /** @brief  Feeds a command into the command queue.
     */
    void put(const Cmd &cmd);
    void put(CmdType type, i64 payload = 0, i64 payload2 = 0) { put(Cmd(type, payload, payload2)); }
    void put(CmdType type, ConfigCmd payload)  { put(Cmd(type, payload)); }
    /*
    void put(CmdType type, KeyCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, CoordCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, GamePadCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, TapeCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, AlarmCmd payload)  { put(Cmd(type, payload)); }
    */
    /// @}
};

}
