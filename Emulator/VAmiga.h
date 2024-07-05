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
// Components
//

struct AmigaAPI : API {

    class Amiga *amiga = nullptr;

    /// @name Analyzing the emulator
    /// @{

    /** @brief  Returns the component's current configuration.
     */
    const AmigaConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const AmigaInfo &getInfo() const;
    const AmigaInfo &getCachedInfo() const;

    /// @}
    /// @name Resetting the Amiga
    /// @{

    /** @brief  Performs a hard reset
     *
     *  A hard reset affects all components. The effect is similar to
     *  switching power off and on.
     */
    void hardReset();

    /** @brief  Performs a hard reset
     *
     *  A soft reset emulates the execution of the CPU's reset instruction.
     */
    void softReset();

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

    /** @brief  Returns the component's current configuration.
     */
    const AgnusConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const AgnusInfo &getInfo() const;
    const AgnusInfo &getCachedInfo() const;

    /** @brief  Provides details about the currently selected chip revision.
     */
    AgnusTraits getTraits() const;
};

struct BlitterAPI : API {

    class Blitter *blitter = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const BlitterConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const BlitterInfo &getInfo() const;
    const BlitterInfo &getCachedInfo() const;
};

struct CIAAPI : API {

    class CIA *cia = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const CIAConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const CIAInfo &getInfo() const;
    const CIAInfo &getCachedInfo() const;
};

struct CopperAPI : API {

    class Copper *copper = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    // const BlitterConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const CopperInfo &getInfo() const;
    const CopperInfo &getCachedInfo() const;
};

struct CPUAPI : API {

    class CPU *cpu = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const CPUConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const CPUInfo &getInfo() const;
    const CPUInfo &getCachedInfo() const;
};

struct GuardsAPI : API {

    class moira::Guards *guards = nullptr;
};

struct DeniseAPI : API {

    class Denise *denise = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const DeniseConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DeniseInfo &getInfo() const;
    const DeniseInfo &getCachedInfo() const;
};

struct DiskControllerAPI : API {

    class DiskController *diskController = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const DiskControllerConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DiskControllerInfo &getInfo() const;
    const DiskControllerInfo &getCachedInfo() const;
};

struct DmaDebuggerAPI : API {

    class DmaDebugger *dmaDebugger = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const DmaDebuggerConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DmaDebuggerInfo &getInfo() const;
    const DmaDebuggerInfo &getCachedInfo() const;
};

struct MemoryAPI : API {

    class Memory *mem = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const MemConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
     const MemInfo &getInfo() const;
     const MemInfo &getCachedInfo() const;
};

struct PaulaAPI : API {

    class Paula *paula = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    // const PaulaConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const PaulaInfo &getInfo() const;
    const PaulaInfo &getCachedInfo() const;
};

struct RtcAPI : API {

    class RTC *rtc = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const RTCConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    // const RTCInfo &getInfo() const;
    // const RTCInfo &getCachedInfo() const;
};


//
// Peripherals
//

struct FloppyDriveAPI : API {

    class FloppyDrive *drive = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const FloppyDriveConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const FloppyDriveInfo &getInfo() const;
    const FloppyDriveInfo &getCachedInfo() const;
};

struct HardDriveAPI : API {

    class HardDrive *drive = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const HardDriveConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const HardDriveInfo &getInfo() const;
    const HardDriveInfo &getCachedInfo() const;
};

struct JoystickAPI : API {

    class Joystick *joystick = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const JoystickConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    // const JoystickInfo &getInfo() const;
    // const JoystickInfo &getCachedInfo() const;
};

struct KeyboardAPI : API {

    class Keyboard *keyboard = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const KeyboardConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    // const KeyboardInfo &getInfo() const;
    // const KeyboardInfo &getCachedInfo() const;

    /** @brief  Checks if a key is currently pressed.
     *  @param  key     The key to check.
     */
    bool isPressed(KeyCode key) const;

    /** @brief  Presses a key
     *  @param  key     The key to press.
     *  @param  delay   An optional delay in seconds.
     *
     *  If no delay is specified, the function will immediately modify the
     *  C64's keyboard matrix. Otherwise, it will ask the event scheduler
     *  to modify the matrix with the specified delay.
     *
     *  @note If you wish to press multiple keys, make sure to let some time
     *  pass between two key presses. You need to give the C64 time to scan the
     *  keyboard matrix before another key can be pressed.
     */
    void press(KeyCode key, double delay = 0.0);

    /** @brief  Releases a key
     *  @param  key     The key to release.
     *  @param  delay   An optional delay in seconds.
     *
     *  If no delay is specified, the function will immediately modify the
     *  C64's keyboard matrix. Otherwise, it will ask the event scheduler
     *  to modify the matrix with the specified delay.
     */
    void release(KeyCode key, double delay = 0.0);

    /** @brief  Releases all currently pressed keys
     */
    void releaseAll();

    /** @brief  Uses the auto-typing daemon to type a string.
     *  @param  text    The text to type.
     */
    void autoType(const string &text);

    /** @brief  Aborts any active auto-typing activity.
     */
    void abortAutoTyping();
};

struct MouseAPI : API {

    class Mouse *mouse = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const MouseConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    // const MouseInfo &getInfo() const;
    // const MouseInfo &getCachedInfo() const;
};


//
// Ports
//

struct SerialPortAPI : API {

    class SerialPort *serialPort = nullptr;
};

struct ControlPortAPI : API {

    class ControlPort *controlPort = nullptr;

    JoystickAPI joystick;
    MouseAPI mouse;

    /** @brief  Returns the component's current configuration.
     */
    // const ControlPortConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const ControlPortInfo &getInfo() const;
    const ControlPortInfo &getCachedInfo() const;
};

struct VideoPortAPI : API {

    class VideoPort *videoPort = nullptr;

    /** @brief  Returns the component's current configuration.
     */
    const VideoPortConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const VideoPortInfo &getInfo() const;
    const VideoPortInfo &getCachedInfo() const;

    /// @}
    /// @name Retrieving video data
    /// @{

    /** @brief  Returns a pointer to the most recent stable texture
     *
     * The texture dimensions are given by constants vc64::Texture::width
     * and vc64::Texture::height texels. Each texel is represented by a
     * 32 bit color value.
     */
    const class FrameBuffer &getTexture() const;

};



//
// Media
//


//
// Misc
//

struct DebuggerAPI : API {

    class Debugger *debugger = nullptr;

    /** @brief  Returns a string representations for a portion of memory.
     */
    string ascDump(Accessor acc, u32 addr, isize bytes) const;
    string hexDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;
    string memDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;
};

/** The user's defaults storage
 *
 *  The defaults storage manages all configuration settings that persist across
 *  multiple application launches. It provides the following functionality:
 *
 *  - **Loading and saving the storage data**
 *
 *    You can persist the user's defaults storage in a file, a stream, or a
 *    string stream.
 *
 *  - **Reading and writing key-value pairs**
 *
 *    The return value is read from the user's defaults storage for registered
 *    keys. For unknown keys, an exception is thrown.
 *
 *  - **Registerung fallback values**
 *
 *    The fallback value is used for registered keys with no custom value set.
 *
 *    @note Setting a fallback value for an unknown key is permitted. In this
 *    case, a new key is registered together with the provided default value.
 *    The GUI utilizes this feature to register additional keys, such as keys
 *    storing shader-relevant parameters that are irrelevant to the emulation
 *    core.
 */
struct DefaultsAPI : API {

    class Defaults *defaults = nullptr;

    DefaultsAPI(Defaults *defaults) : defaults(defaults) { }

    ///
    /// @{
    /// @name Loading and saving the key-value storage

public:

    /** @brief  Loads a storage file from disk
     *  @throw  VC64Error (#ERROR_FILE_NOT_FOUND)
     *  @throw  VC64Error (#ERROR_SYNTAX)
     */
    void load(const std::filesystem::path &path);

    /** @brief  Loads a storage file from a stream
     *  @throw  VC64Error (#ERROR_SYNTAX)
     */
    void load(std::ifstream &stream);

    /** @brief  Loads a storage file from a string stream
     *  @throw  VC64Error (#ERROR_SYNTAX)
     */
    void load(std::stringstream &stream);

    /** @brief  Saves a storage file to disk
     *  @throw  VC64Error (#ERROR_FILE_CANT_WRITE)
     */
    void save(const std::filesystem::path &path);

    /** @brief  Saves a storage file to stream
     */
    void save(std::ofstream &stream);

    /** @brief  Saves a storage file to a string stream
     */
    void save(std::stringstream &stream);


    /// @}
    /// @{
    /// @name Reading key-value pairs

public:

    /** @brief  Queries a key-value pair.
     *  @param  key     The key.
     *  @result The value as a string.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    string getRaw(const string &key) const;

    /** @brief  Queries a key-value pair.
     *  @param  key     The key.
     *  @result The value as an integer. 0 if the value cannot not be parsed.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    i64 get(const string &key) const;

    /** @brief  Queries a key-value pair.
     *  @param  option  A config option whose name is used as the prefix of the key.
     *  @param  nr      Optional number that is appened to the key as suffix.
     *  @result The value as an integer.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    i64 get(Option option, isize nr = 0) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  key     The key.
     *  @result The value as a string.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    string getFallbackRaw(const string &key) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  key     The key.
     *  @result The value as an integer. 0 if the value cannot not be parsed.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    i64 getFallback(const string &key) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  option  A config option whose name is used as the key.
     *  @param  nr      Optional number that is appened to the key as suffix.
     *  @result The value as an integer.
     *  @throw  VC64Error (#ERROR\_INVALID\_KEY)
     */
    i64 getFallback(Option option, isize nr = 0) const;


    /// @}
    /// @{
    /// @name Writing key-value pairs

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  key     The key, given as a string.
     *  @param  value   The value, given as a string.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void set(const string &key, const string &value);

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as a string.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void set(Option opt, const string &value);

    /** @brief  Writes multiple key-value pairs into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value for all pairs, given as a string.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void set(Option opt, const string &value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void set(Option opt, i64 value);

    /** @brief  Writes multiple key-value pairs into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value for all pairs, given as an integer.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void set(Option opt, i64 value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  key     The key, given as a string.
     *  @param  value   The value, given as a string.
     */
    void setFallback(const string &key, const string &value);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void setFallback(Option opt, const string &value);

    /** @brief  Writes multiple key-value pairs into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The shared value for all pairs, given as a string.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     */
    void setFallback(Option opt, const string &value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void setFallback(Option opt, i64 value);

    /** @brief  Writes multiple key-value pairs into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The shared value for all pairs, given as an integer.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     */
    void setFallback(Option opt, i64 value, std::vector<isize> objids);


    /// @}
    /// @{
    /// @name Deleting key-value pairs

    /** @brief  Deletes all key-value pairs.
     */
    void remove();

    /** @brief  Deletes a key-value pair
     *  @param  key     The key of the key-value pair.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void remove(const string &key) throws;

    /** @brief  Deletes a key-value pair
     *  @param  option  The option's name forms the key.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void remove(Option option) throws;

    /** @brief  Deletes multiple key-value pairs.
     *  @param  option  The option's name forms the prefix of the keys.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  VC64Error (#ERROR_INVALID_KEY)
     */
    void remove(Option option, std::vector <isize> objids) throws;

    /// @}
};

struct HostAPI : API {

    class Host *host = nullptr;
};

/** RetroShell Public API
 */
struct RetroShellAPI : API {

    class RetroShell *retroShell = nullptr;
    
    /// @name Querying the console
    /// @{
    ///
    /** @brief  Returns a pointer to the text buffer.
     *  The text buffer contains the complete contents of the console. It
     *  will be expanded when new output is generated. When the buffer
     *  grows too large, old contents is cropped.
     */
    const char *text();

    /** @brief  Returns the relative cursor position.
     *  The returned value is relative to the end of the input line. A value
     *  of 0 indicates that the cursor is at the rightmost position, that
     *  is, one character after the last character of the input line. If the
     *  cursor is at the front of the input line, the value matches the
     *  length of the input line.
     */
    isize cursorRel();

    /// @}
    /// @name Typing characters and strings
    /// @{

    /** @brief  Informs RetroShell that a key has been typed.
     *  @param  key     The pressed key
     *  @param  shift   Status of the shift key
     */
    void press(RetroShellKey key, bool shift = false);

    /** @brief  Informs RetroShell that a key has been typed.
     *  @param  c       The pressed key
     */
    void press(char c);

    /** @brief  Informs RetroShell that multiple keys have been typed.
     *  @param  s       The typed text
     */
    void press(const string &s);

    /// @}
    /// @name Controlling the output stream
    /// @{

    /** @brief  Assign an additional output stream.
     *  In addition to writing the RetroShell output into the text buffer,
     *  RetroShell will write the output into the provides stream.
     */
    void setStream(std::ostream &os);

    /// @}
    /// @name Executing scripts
    /// @{

    /** @brief  Executes a script.
     *  The script is executes asynchroneously. However, RetroShell will
     *  send messages back to the GUI thread to inform about the execution
     *  state. After the last script command has been executed,
     *  MSG\_SCRIPT\_DONE is sent. If shell execution has been aborted due
     *  to an error, MSG\_SCRIPT\_ABORT is sent.
     */
    void execScript(std::stringstream &ss);
    void execScript(const std::ifstream &fs);
    void execScript(const string &contents);
    // void execScript(const MediaFile &file);

    /// @}
};


struct RecorderAPI : API {

    class Recorder *recorder = nullptr;
};

struct RemoteManagerAPI : API {

    class RemoteManager *remoteManager = nullptr;
};


//
// Top-level API
//

class VAmiga : public API {

public:

    static DefaultsAPI defaults;

    AmigaAPI amiga;
    AgnusAPI agnus;
    BlitterAPI blitter;
    GuardsAPI breakpoints;
    CIAAPI ciaA, ciaB;
    VideoPortAPI videoPort;
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

    /// @name Analyzing the emulator
    /// @{

    /** @brief  Returns the component's current state.
     */
    const EmulatorInfo &getInfo() const;
    const EmulatorInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    const EmulatorStats &getStats() const;

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

    /** @brief   Performs a hard reset
     *
     *  A hard reset affects all components. The effect is similar to
     *  switching power off and on.
     */
    void hardReset();

    /** @brief   Performs a soft reset
     *
     *  A soft reset is similar to executing the CPU's reset instruction.
     */
    void softReset();

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

    /** @brief  Returns true if the emulator has been launched.
     */
    bool isLaunched() const;

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
    void put(CmdType type, KeyCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, GamePadCmd payload)  { put(Cmd(type, payload)); }
    void put(CmdType type, CoordCmd payload)  { put(Cmd(type, payload)); }
    /*
    void put(CmdType type, AlarmCmd payload)  { put(Cmd(type, payload)); }
    */
    /// @}
};

}
