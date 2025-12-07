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
#include "AppError.h"
#include "MediaFile.h"

namespace vamiga {

//
// Base class for all APIs
//

class API {

public:

    class Emulator *emu = nullptr;

    API() { }
    API(Emulator *emu) : emu(emu) { }

    void suspend() const;
    void resume() const;
};

//
// Components
//

class AmigaAPI : public API {

    friend class VAmiga;
    
public:

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
    
    /** @brief  Prints debug information about the component
     *
     *  @param  category    Debug information category
     *  @param  os Output stream
     */
    void dump(Category category, std::ostream &os) const;


    /// @}
    /// @name Accessing the message queue
    /// @{

    /** @brief  Reads a message from the message queue
     *
     *  @param  msg    The returned message (if any)
     *  @return True if a message could be read, false if the queue was empty
     */
    bool getMsg(Message &msg);


    /// @}
    /// @name Working with workspaces and snapshots
    /// @{

    /** @brief  Loads a workspace.
     *
     *  @param  path    Path to a workspace folder
     */
    void loadWorkspace(const std::filesystem::path &path);

    /** @brief  Saves a workspace to disk.
     *
     *  @param  path    Destination path
     */
    void saveWorkspace(const std::filesystem::path &path) const;

    /** @brief  Takes a snapshot
     *
     *  @param  compressor   The snapshot compression method
     *  @param  delay              Optional delay in seconds
     *  @param  repeat            if true, snapshots are taken continuously
     *
     *  @return A pointer to the created Snapshot object.
     *
     *  @note   The function transfers the ownership to the caller. It is
     *          his responsibility of the caller to free the object.
     */
    std::unique_ptr<MediaFile> takeSnapshot(Compressor compressor, isize delay = 0, bool repeat = false);

    /** @brief  Loads a snapshot.
     *
     *  @param  snapshot    Reference to a snapshot
     */
    void loadSnapshot(const MediaFile &snapshot);

    /** @brief  Loads a snapshot
     *
     *  @param  path    Source path
     */
    void loadSnapshot(const std::filesystem::path &path);

    /** @brief  Saves a snapshot to disk.
     *
     *  @param  path    Destination path
     */
    void saveSnapshot(const std::filesystem::path &path) const;

    
    /// @}
    /// @name Auto-inspecting components
    /// @{

    /** @brief  Gets the current auto-inspection mask
     *  The GUI utilizes Auto-Inspection to display life updates of the internal
     *  emulator state in the Inspector panel. As soon as an auto-inspection
     *  mask is set, the emulator caches the internal states of the inspected
     *  components at periodic intervals. The inspected components are
     *  specified as a bit mask.
     *
     *  @return A bit mask indicating the components under inspection
     */
    u64 getAutoInspectionMask() const;

    /** @brief  Sets the current auto-inspection mask
     *
     *  @example The following call enables auto-inspections for the CIA chips
     *  and Paula: setAutoInspectionMask(1 << COMP_CIA | 1 << COMP_PAULA);
     *
     *  @param  mask A bit mask indicating the components under inspection
     */
    void setAutoInspectionMask(u64 mask);
    
    /// @}
};


//
// Agnus
//

class DmaDebuggerAPI : public API {

    friend class VAmiga;
    
public:

    class DmaDebugger *dmaDebugger = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const DmaDebuggerConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DmaDebuggerInfo &getInfo() const;
    const DmaDebuggerInfo &getCachedInfo() const;
};

class LogicAnalyzerAPI : public API {

    friend class VAmiga;
    
public:

    class LogicAnalyzer *logicAnalyzer = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const LogicAnalyzerConfig &getConfig() const;
    
    /** @brief  Returns the component's current state.
     */
    const LogicAnalyzerInfo &getInfo() const;
    const LogicAnalyzerInfo &getCachedInfo() const;
};

class BlitterAPI : public API {

    friend class VAmiga;
    
public:

    class Blitter *blitter = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const BlitterConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const BlitterInfo &getInfo() const;
    const BlitterInfo &getCachedInfo() const;
};

class CopperAPI : public API {

    friend class VAmiga;
    
public:

    class Copper *copper = nullptr;

    /** @brief  Returns the component's current state.
     */
    const CopperInfo &getInfo() const;
    const CopperInfo &getCachedInfo() const;

    /** @brief  Disassembles a Copper instruction.
     *  @param  list     The Cooper list to take the instruction from
     *  @param  offset   Offset of the instruction relative to the start of the
     *                   list.
     *  @param  symbolic The output format. The flag indicates whether the
     *                   instruction should be be disassembled in symbolic,
     *                   human-readable form or in raw form as a sequence of
     *                   hexadecimal numbers.
     */
    string disassemble(isize list, isize offset, bool symbolic) const;

    /** @brief  Disassembles a Copper instruction.
     *  @param  addr     The address of the Cooper instruction in memory.
     *  @param  symbolic Output format.
     */
    string disassemble(u32 addr, bool symbolic) const;

    /** @brief  Checks whether a Copper instruction is illegal.
     *          A Copper instruction is classified as illegal if it is tries
     *          custom chip register it has no access to.
     *  @param  addr     The address of the Cooper instruction in memory.
     */
    bool isIllegalInstr(u32 addr) const;
};

class AgnusAPI : public API {

    friend class VAmiga;
    
public:

    class Agnus *agnus = nullptr;
    
    CopperAPI copper;
    BlitterAPI blitter;
    DmaDebuggerAPI dmaDebugger;
    LogicAnalyzerAPI logicAnalyzer;
    
    /** @brief  Returns the component's current configuration.
     */
    const AgnusConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const AgnusInfo &getInfo() const;
    const AgnusInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    const AgnusMetrics &getMetrics() const;
    const AgnusMetrics &getCachedMetrics() const;

    /** @brief  Provides details about the currently selected chip revision.
     */
    const AgnusTraits getTraits() const;
};


//
// CIA
//

class CIAAPI : public API {

    friend class VAmiga;
    
public:

    class CIA *cia = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const CIAConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const CIAInfo &getInfo() const;
    const CIAInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    CIAMetrics getMetrics() const;
    CIAMetrics getCachedMetrics() const;
};


//
// CPU
//

namespace moira { class Guards; class Debugger; }

class GuardsAPI : public API {

    friend class VAmiga;
    
public:

    class GuardList *guards = nullptr;
    
    /** @brief  Returns the number of guards in the guard list.
     */
    isize elements() const;

    /** @brief  Returns information about a guard.
     *  @param  nr      Number of the guard in the guard list
     */
    std::optional<GuardInfo> guardNr(long nr) const;

    /** @brief  Returns information about a guard.
     *  @param  target  The target of the guard to query
     */
    std::optional<GuardInfo> guardAt(u32 target) const;

    /** @brief  Sets a guard.
     *  @param  target  The observed target. For breakpoints and watchpoints,
     *                  the target is a memory address. For catchpoints, the
     *                  target is a vector number (interrupts or traps).
     *  @param  ignores If a value greater zero is given, the guard has to
     *                  to be hit the specified number of times until program
     *                  execution is paused.
     */
    void setAt(u32 target, isize ignores = 0);

    /** @brief  Relocates a guard.
     *  @param  nr      Number of the guard in the guard list
     *  @param  target  New target
     */
    void moveTo(isize nr, u32 target);

    /** @brief  Deletes a guard.
     *  @param  nr      Number of the guard in the guard list
     */
    void remove(isize nr);

    /** @brief  Deletes a guard.
     *  @param  target  The target of the guard to be deleted.
     */
    void removeAt(u32 target);

    /** @brief  Deletes all guards.
     */
    void removeAll();

    /** @brief  Enables a guard.
     *  @param  nr      Number of the guard in the guard list
     */
    void enable(isize nr);

    /** @brief  Enables a guard.
     *  @param  target  The target of the guard to be deleted
     */
    void enableAt(u32 target);

    /** @brief  Enables all guards.
     */
    void enableAll();

    /** @brief  Disables a guard.
     *  @param  nr      Number of the guard in the guard list
     */
    void disable(isize nr);

    /** @brief  Disables a guard.
     *  @param  target  The target of the guard to be deleted
     */
    void disableAt(u32 target);

    /** @brief  Disables all guards.
     */
    void disableAll();
    void toggle(isize nr);

};

class CPUDebuggerAPI : public API {

    friend class VAmiga;
    
public:

    class CPU *cpu = nullptr;

    /** @brief  Returns the number of instructions in the record buffer.
     *  @note   The record buffer is only filled in track mode. To save
     *          computation time, the GUI enables track mode when the CPU
     *          inspector is opened and disables track mode when it is
     *          closed.
     */
    isize loggedInstructions() const;

    /** @brief  Empties the record buffer.
     */
    void clearLog();

    /** @brief  Disassembles a recorded instruction from the log buffer
     */
    const char *disassembleRecordedInstr(isize i, isize *len);
    const char *disassembleRecordedWords(isize i, isize len);
    const char *disassembleRecordedFlags(isize i);
    const char *disassembleRecordedPC(isize i);

    /** @brief  Disassembles the instruction at the specified address
     */
    const char *disassembleWord(u16 value);
    const char *disassembleAddr(u32 addr);
    const char *disassembleInstr(u32 addr, isize *len);
    const char *disassembleWords(u32 addr, isize len);

    string vectorName(isize i);
};

class CPUAPI : public API {

    friend class VAmiga;
    
public:

    class CPU *cpu = nullptr;

    CPUDebuggerAPI debugger;
    GuardsAPI breakpoints;
    GuardsAPI watchpoints;

    /** @brief  Returns the component's current configuration.
     */
    const CPUConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const CPUInfo &getInfo() const;
    const CPUInfo &getCachedInfo() const;
};

class DeniseAPI : public API {

    friend class VAmiga;
    
public:

    class Denise *denise = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const DeniseConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DeniseInfo &getInfo() const;
    const DeniseInfo &getCachedInfo() const;
};


//
// Memory
//

class MemoryDebuggerAPI : public API {

    friend class VAmiga;
    
public:

    class Memory *mem = nullptr;
    
    /// @name Debugging memory
    /// @{

    /**  @brief  Returns the memory source for a given address
     */
    MemSrc getMemSrc(Accessor acc, u32 addr) const;
    
    /** @brief  Reads a value from memory without causing side effects.
     */
    u8 spypeek8(Accessor acc, u32 addr) const;
    u16 spypeek16(Accessor acc, u32 addr) const;

    /** @brief  Returns a string representations for a portion of memory.
     */
    string ascDump(Accessor acc, u32 addr, isize bytes) const;
    string hexDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;
    string memDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;

    /// @}
};

class MemoryAPI : public API {

    friend class VAmiga;
    
public:

    class Memory *mem = nullptr;
    
    MemoryDebuggerAPI debugger;

    /// @name Analying the component
    /// @{

    /** @brief  Returns the component's current configuration.
     */
    const MemConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const MemInfo &getInfo() const;
    const MemInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    const MemMetrics &getMetrics() const;
    const MemMetrics &getCachedMetrics() const;

    /** @brief  Provides details about the installed ROM, WOM, or ROM extension.
     */
    const RomTraits &getRomTraits() const;
    const RomTraits &getWomTraits() const;
    const RomTraits &getExtTraits() const;

    /// @}
    /// @name Handling ROMs
    /// @{

    /** @brief  Loads a ROM from a file
     *          The ROM type is determined automatically.
     */
    void loadRom(const std::filesystem::path &path);
    void loadExt(const std::filesystem::path &path);

    /** @brief  Loads a ROM provided by a RomFile
     */
    void loadRom(MediaFile &file);
    void loadExt(MediaFile &file);

    /** @brief  Loads a ROM provided by a memory buffer
     */
    void loadRom(const u8 *buf, isize len);
    void loadExt(const u8 *buf, isize len);

    /** @brief  Saves a Rom to disk
     */
    void saveRom(const std::filesystem::path &path);
    void saveWom(const std::filesystem::path &path);
    void saveExt(const std::filesystem::path &path);

    /** @brief  Removes a ROM
     */
    void deleteRom();
    void deleteWom();
    void deleteExt();

    /// @}
};

//
// Paula
//

class AudioChannelAPI : public API {

    friend class VAmiga;
    isize channel = 0;
    
public:

    class Paula *paula = nullptr;
    
    AudioChannelAPI(isize channel) : API(), channel(channel) { }

    /** @brief  Returns the component's current state.
     */
    const StateMachineInfo &getInfo() const;
    const StateMachineInfo &getCachedInfo() const;
};

class DiskControllerAPI : public API {

    friend class VAmiga;
    
public:

    class DiskController *diskController = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const DiskControllerConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const DiskControllerInfo &getInfo() const;
    const DiskControllerInfo &getCachedInfo() const;
};

class UARTAPI : public API {

    friend class VAmiga;
    
public:

    class UART *uart = nullptr;
    
    /** @brief  Returns the component's current state.
     */
    const UARTInfo &getInfo() const;
    const UARTInfo &getCachedInfo() const;
};

class PaulaAPI : public API {

    friend class VAmiga;
    
public:

    class Paula *paula = nullptr;
    
    AudioChannelAPI audioChannel0 = AudioChannelAPI(0);
    AudioChannelAPI audioChannel1 = AudioChannelAPI(1);
    AudioChannelAPI audioChannel2 = AudioChannelAPI(2);
    AudioChannelAPI audioChannel3 = AudioChannelAPI(3);
    DiskControllerAPI diskController;
    UARTAPI uart;

    /** @brief  Returns the component's current configuration.
     */
    // const PaulaConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const PaulaInfo &getInfo() const;
    const PaulaInfo &getCachedInfo() const;
};

class RTCAPI : public API {

    friend class VAmiga;
    
public:

    class RTC *rtc = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const RTCConfig &getConfig() const;

    /** @brief  Updates the RTC's internal state.
     *          Call this function if you want, e.g., spypeek to return an
     *          up-to-date value from the RTC register memory locations.
     */
    void update();
};


//
// Peripherals
//

//
// Peripherals (FloppyDrive)
//

class FloppyDriveAPI : public API {

    friend class VAmiga;
    
public:

    class FloppyDrive *drive = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const FloppyDriveConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const FloppyDriveInfo &getInfo() const;
    const FloppyDriveInfo &getCachedInfo() const;

    /** @brief  Getter for the raw disk object
     *  @return A pointer to the disk object or nullptr if no disk is present.
     */
    class FloppyDisk &getDisk() const;

    /** @brief Queries a disk flag
     */
    bool getFlag(DiskFlags mask) const;

    /** @brief Sets or clears one or more disk flags
     */
    void setFlag(DiskFlags mask, bool value);

    /** @brief  Checks whether the drive is compatible with disks of a
     *          particular geometry.
     */
    bool isInsertable(Diameter t, Density d) const;

    /** @brief  Inserts a new disk.
     *  @param  fstype  File system format
     *  @param  id  Boot block identifier
     *  @param  name  Name of the disk
     *  @param  path Optional folder to import
     */
    void insertBlankDisk(FSFormat fstype, BootBlockId id, string name, const std::filesystem::path &path = {});

    /** @brief  Inserts a disk created from a media file.
     *  @param  file    A media file wrapper object.
     *  @param  wp      Write-protection status of the disk.
     */
    void insertMedia(MediaFile &file, bool wp);
    void insert(const std::filesystem::path &path, bool wp);

    /** @brief  Inserts a disk created from a file system.
     *  @param  fs      A file system wrapper object.
     *  @param  wp      Write-protection status of the disk.
     */
    void insertFileSystem(const class FileSystem &fs, bool wp);

    /** @brief  Ejects the current disk.
     */
    void ejectDisk();

    std::unique_ptr<MediaFile> exportDisk(FileType type);

    /** @brief  Creates a textual bit representation of a track's data
     */
    string readTrackBits(isize track);
};


//
// Peripherals (HardDrive)
//

class HdControllerAPI : public API {

    friend class VAmiga;
    
public:

    class HdController *controller = nullptr;
    
    /** @brief  Provides details about the controller
     */
    // const HdcTraits &getTraits() const;

    /** @brief  Returns the component's current state.
     */
    const HdcInfo &getInfo() const;
    const HdcInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    const HdcStats &getStats() const;
    const HdcStats &getCachedStats() const;
};

class HardDriveAPI : public API {

    friend class VAmiga;
    
public:

    class HardDrive *drive = nullptr;
    
    HdControllerAPI controller;

    /** @brief  Getter for the raw disk object
     */
    class HardDrive &getDrive();

    /** @brief  Returns the component's current configuration.
     */
    const HardDriveConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const HardDriveInfo &getInfo() const;
    const HardDriveInfo &getCachedInfo() const;

    /** @brief  Provides details about the hard drive and its partitions
     */
    const HardDriveTraits &getTraits() const;
    const PartitionTraits &getPartitionTraits(isize nr) const;

    /** @brief Queries a disk flag
     */
    bool getFlag(DiskFlags mask);

    /** @brief Sets or clears one or more disk flags
     */
    void setFlag(DiskFlags mask, bool value);

    /** @brief Returns possible drive geometries for a given capacity.
     *  The function takes a number of blocks and returns all common
     *  cyclinder/heads/sectors combinations that match the given size.
     */
    std::vector<std::tuple<isize,isize,isize>> geometries(isize numBlocks) const;

    /** @brief Changes the drives geometry.
     *  @param c    Cylinders
     *  @param h    Heads
     *  @param s    Sectors
     *  @param b    Block size
     */
    void changeGeometry(isize c, isize h, isize s, isize b = 512);

    /** @brief  Formats the hard drive
     */
    void format(FSFormat fs, const string &name);
    
    /** @brief  Attaches a hard drive provided by an URL to a media file.
     *  @param  path    Path to the media file.
     */
    void attach(const std::filesystem::path &path);

    /** @brief  Attaches a hard drive provided by a media file.
     *  @param  file    A media file wrapper object.
     */
    void attach(const MediaFile &file);

    /** @brief  Attaches a hard drive with a particular geometry.
     *  @param  c       Number of cylinders
     *  @param  h       Number of heads
     *  @param  s       Number of sectors
     *  @param  b       Bytes per sector
     */
    void attach(isize c, isize h, isize s, isize b = 512);

    /** @brief  Imports files from a folder
     *  @note   All existing files are deleted prior to importing the folder.
     */
    void importFiles(const std::filesystem::path &path);

    /** @brief  Exports the hard drive to an HDF file on disk
     */
    void writeToFile(const std::filesystem::path &path);

    /** @brief  Converts the hard drive to an HDF media file
     */
    std::unique_ptr<MediaFile> createHDF();
};


//
// Peripherals (Joystick)
//

class JoystickAPI : public API {

    friend class VAmiga;
    
public:

    class Joystick *joystick = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const JoystickConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const JoystickInfo &getInfo() const;
    const JoystickInfo &getCachedInfo() const;

    /** @brief  Triggers a joystick action.
     */
    void trigger(GamePadAction event);
};


//
// Peripherals (Keyboard)
//

class KeyboardAPI : public API {

    friend class VAmiga;
    
public:

    class Keyboard *keyboard = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const KeyboardConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const KeyboardInfo &getInfo() const;
    const KeyboardInfo &getCachedInfo() const;

    /** @brief  Checks if a key is currently pressed.
     *  @param  key     The key to check.
     */
    bool isPressed(KeyCode key) const;

    /** @brief  Presses a key
     *  @param  key         The key to press.
     *  @param  delay       An optional delay in seconds until the key is pressed.
     *  @param  duration    If specified, the key will be automatically released.
     */
    void press(KeyCode key, double delay = 0.0, double duration = 0.0);

    /** @brief  Toggles a key
     *  @param  key         The key to toggle.
     *  @param  delay       An optional delay in seconds until the key is toggled.
     *  @param  duration    If specified, the key will be toggled again after the additional delay.
     */
    void toggle(KeyCode key, double delay = 0.0, double duration = 0.0);

    /** @brief  Releases a key
     *  @param  key     The key to release.
     *  @param  delay   An optional delay in seconds.
     */
    void release(KeyCode key, double delay = 0.0);

    /** @brief  Releases all currently pressed keys
     */
    void releaseAll();

    /** @brief  Deletes all pending keyboard events
     */
    void abortTyping();
};


//
// Peripherals (Mouse)
//

class MouseAPI : public API {

    friend class VAmiga;
    
public:

    class Mouse *mouse = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const MouseConfig &getConfig() const;

    /** Moves the mouse
     *  @param x    New absolute horizontal coordinate
     *  @param y    New absolute vertical coordinate
     */
    void setXY(double x, double y);

    /** Moves the mouse
     *  @param dx       Relative horizontal mouse movement
     *  @param dy       Relative vertical mouse movement
     */
    void setDxDy(double dx, double dy);

    /** Triggers a mouse button event
     *  @param action   The triggered event
     */
    void trigger(GamePadAction action);

    /** Feeds a coordinate into the shake detector.
     *
     *  The shake detector keeps track of the transmitted coordinates and
     *  scans for rapid movements caused by shaking the mouse.
     *
     *  @param x    Current horizontal mouse position.
     *  @param y    Current vertical mouse position.
     *  @return     true iff a shaking mouse has been detected.
     */
    bool detectShakeXY(double x, double y);

    /** Feeds a coordinate into the shake detector.
     *
     *  The shake detector keeps track of the transmitted coordinates and
     *  scans for rapid movements caused by shaking the mouse.
     *
     *  @param dx   Current horizontal mouse position, relative to the
     *              previous position.
     *  @param dy   Current vertical mouse position, relative to the
     *              previous position.
     *  @return     true iff a shaking mouse has been detected.
     */
    bool detectShakeDxDy(double dx, double dy);
};


//
// Ports
//


//
// Ports (AudioPort)
//

class AudioPortAPI : public API {

    friend class VAmiga;
    
public:

    class AudioPort *port = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const AudioPortConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const AudioPortInfo &getInfo() const;
    const AudioPortInfo &getCachedInfo() const;

    /** @brief  Returns statistical information about the components.
     */
    const AudioPortMetrics &getStats() const;
    const AudioPortMetrics &getCachedStats() const;

    /// @}
    /// @name Retrieving audio data
    /// @{

    /** @brief  Extracts a number of mono samples from the audio buffer
     *  Internally, the audio port maintains a ringbuffer storing stereo
     *  audio samples. When this function is used, both internal stream are
     *  added together and written to to the destination buffer.
     *  @param  buffer  Pointer to the destination buffer
     *  @param  n       Number of sound samples to copy.
     *  @return         Number of actually copied sound sound samples.
     */
    isize copyMono(float *buffer, isize n);

    /** @brief  Extracts a number of stereo samples from the audio buffer.
     *  @param  left    Pointer to the left channel's destination buffer.
     *  @param  right   Pointer to the right channel's destination buffer.
     *  @param  n       Number of sound samples to copy.
     *  @return         Number of actually copied sound sound samples.
     */
    isize copyStereo(float *left, float *right, isize n);

    /** @brief  Extracts a number of stereo samples from the audio buffer.
     *  This function has to be used if a stereo stream is managed in a
     *  single destination buffer. The samples of both channels will be
     *  interleaved, that is, a sample for the left channel will be
     *  followed by a sample of the right channel and vice versa.
     *  @param  buffer  Pointer to the destinationleft buffer.
     *  @param  n       Number of sound samples to copy.
     *  @return         Number of actually copied sound sound samples.
     */
    isize copyInterleaved(float *buffer, isize n);

    /// @}
    /// @name Visualizing waveforms
    /// @{

    /** @brief  Draws a visual representation of the waveform.
     *  The Mac app uses this function to visualize the contents of the
     *  audio buffer in one of it's inspector panels. */
    void drawL(u32 *buffer, isize width, isize height, u32 color) const;
    void drawR(u32 *buffer, isize width, isize height, u32 color) const;

    /// @}};
};


//
// Ports (ControlPort)
//

class ControlPortAPI : public API {

    friend class VAmiga;
    
public:

    class ControlPort *controlPort = nullptr;
    
    JoystickAPI joystick;
    MouseAPI mouse;

    /** @brief  Returns the component's current state.
     */
    const ControlPortInfo &getInfo() const;
    const ControlPortInfo &getCachedInfo() const;
};


//
// Ports (SerialPort)
//

class SerialPortAPI : public API {

    friend class VAmiga;
    
public:

    class SerialPort *serialPort = nullptr;
    
    /** @brief  Returns the component's current configuration.
     */
    const SerialPortConfig &getConfig() const;

    /** @brief  Returns the component's current state.
     */
    const SerialPortInfo &getInfo() const;
    const SerialPortInfo &getCachedInfo() const;

    int readIncomingPrintableByte() const;
    int readOutgoingPrintableByte() const;
};


//
// Ports (VideoPort)
//

class VideoPortAPI : public API {

    friend class VAmiga;
    
public:

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

    /** @brief  Locks the emulator texture
     *
     * This function aquires a mutex that prevents the emulator to modify the
     * stable texture. Call this function prior to getTexture().
     */
    void lockTexture();

    /** @brief  Unlocks the emulator texture
     *
     * This function releases the mutex acquired in lockTexture(). Call this
     * function when the pointer returned by getTexture() is no longer needed.
     */
    void unlockTexture();

    /** @brief  Returns a pointer to the most recent stable texture
     *
     * The texture dimensions are given by constants vamiga::Texture::width
     * and vamiga::Texture::height texels. Each texel is represented by a
     * 32 bit color value.
     */
    const u32 *getTexture() const;
    const u32 *getTexture(isize *nr, bool *lof, bool *prevlof) const;

    /** @brief Experimental
     */
    void findInnerArea(isize &x1, isize &x2, isize &y1, isize &y2) const;
    void findInnerAreaNormalized(double &x1, double &x2, double &y1, double &y2) const;
};


//
// Media
//


//
// Misc (MsgQueue)
//

class MsgQueueAPI : public API {

    friend class VAmiga;
    
public:

    class MsgQueue *msgQueue = nullptr;

    /** @brief  Locks the message queue
     */
    void lockMsgQueue();

    /** @brief  Unlocks the message queue
     */
    void unlockMsgQueue();

    /** @brief  Reads a message from the message queue
     */
    bool getMsg(Message &msg);

    /** @brief  Used by the WASM builds to pass additional data
     */
    string getPayload(isize index);

    /** @brief  Reads multiple messages from the message queue
     */
    // isize getMsg(isize count, Message *buffer);
};

//
// Misc (Debugger)
//

class DebuggerAPI : public API {

    friend class VAmiga;
    
public:

    class Debugger *debugger = nullptr;
    
    /** @brief  Returns a string representations for a portion of memory.
     */
    /*
    string ascDump(Accessor acc, u32 addr, isize bytes) const;
    string hexDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;
    string memDump(Accessor acc, u32 addr, isize bytes, isize sz = 1) const;
     */
};


//
// Misc (Defaults)
//

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
class DefaultsAPI : public API {

    friend class VAmiga;
    
public:

    class Defaults *defaults = nullptr;
    
    DefaultsAPI(Defaults *defaults) : defaults(defaults) { }

    ///
    /// @{
    /// @name Loading and saving the key-value storage

    /** @brief  Loads a storage file from disk
     *  @throw  AppError (#Fault::FILE_NOT_FOUND)
     *  @throw  AppError (#Fault::SYNTAX)
     */
    void load(const std::filesystem::path &path);

    /** @brief  Loads a storage file from a stream
     *  @throw  AppError (#Fault::SYNTAX)
     */
    void load(std::ifstream &stream);

    /** @brief  Loads a storage file from a string stream
     *  @throw  AppError (#Fault::SYNTAX)
     */
    void load(std::stringstream &stream);

    /** @brief  Saves a storage file to disk
     *  @throw  AppError (#Fault::FILE_CANT_WRITE)
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

    /** @brief  Queries a key-value pair.
     *  @param  key     The key.
     *  @result The value as a string.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    string getRaw(const string &key) const;

    /** @brief  Queries a key-value pair.
     *  @param  key     The key.
     *  @result The value as an integer. 0 if the value cannot not be parsed.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    i64 get(const string &key) const;

    /** @brief  Queries a key-value pair.
     *  @param  option  A config option whose name is used as the prefix of the key.
     *  @param  nr      Optional number that is appened to the key as suffix.
     *  @result The value as an integer.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    i64 get(Opt option, isize nr = 0) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  key     The key.
     *  @result The value as a string.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    string getFallbackRaw(const string &key) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  key     The key.
     *  @result The value as an integer. 0 if the value cannot not be parsed.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    i64 getFallback(const string &key) const;

    /** @brief  Queries a fallback key-value pair.
     *  @param  option  A config option whose name is used as the key.
     *  @param  nr      Optional number that is appened to the key as suffix.
     *  @result The value as an integer.
     *  @throw  AppError (#ERROR\_INVALID\_KEY)
     */
    i64 getFallback(Opt option, isize nr = 0) const;


    /// @}
    /// @{
    /// @name Writing key-value pairs

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  key     The key, given as a string.
     *  @param  value   The value, given as a string.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void set(const string &key, const string &value);

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as a string.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void set(Opt opt, const string &value);

    /** @brief  Writes multiple key-value pairs into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value for all pairs, given as a string.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void set(Opt opt, const string &value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void set(Opt opt, i64 value);

    /** @brief  Writes multiple key-value pairs into the user storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value for all pairs, given as an integer.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void set(Opt opt, i64 value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  key     The key, given as a string.
     *  @param  value   The value, given as a string.
     */
    void setFallback(const string &key, const string &value);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void setFallback(Opt opt, const string &value);

    /** @brief  Writes multiple key-value pairs into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The shared value for all pairs, given as a string.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     */
    void setFallback(Opt opt, const string &value, std::vector<isize> objids);

    /** @brief  Writes a key-value pair into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The value, given as an integer.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void setFallback(Opt opt, i64 value);

    /** @brief  Writes multiple key-value pairs into the fallback storage.
     *  @param  opt     The option's name forms the prefix of the keys.
     *  @param  value   The shared value for all pairs, given as an integer.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     */
    void setFallback(Opt opt, i64 value, std::vector<isize> objids);


    /// @}
    /// @{
    /// @name Deleting key-value pairs

    /** @brief  Deletes all key-value pairs.
     */
    void remove();

    /** @brief  Deletes a key-value pair
     *  @param  key     The key of the key-value pair.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void remove(const string &key);

    /** @brief  Deletes a key-value pair
     *  @param  option  The option's name forms the key.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void remove(Opt option);

    /** @brief  Deletes multiple key-value pairs.
     *  @param  option  The option's name forms the prefix of the keys.
     *  @param  objids  The keys are parameterized by adding the vector values as suffixes.
     *  @throw  AppError (#Fault::INVALID_KEY)
     */
    void remove(Opt option, std::vector <isize> objids);

    /// @}
};


//
// Misc (RetroShell)
//

/** RetroShell Public API
 */
class RetroShellAPI : public API {

    friend class VAmiga;
    
public:

    class RetroShell *retroShell = nullptr;
    
    /// @name Querying the console
    /// @{
    ///

    /** @brief  Returns the component's current state.
     */
    const RetroShellInfo &getInfo() const;
    const RetroShellInfo &getCachedInfo() const;

    /** @brief  Returns a pointer to the text buffer.
     *  The text buffer contains the complete contents of the console. It
     *  will be expanded when new output is generated. When the buffer
     *  grows too large, old contents is cropped.
     */
    const char *text();

    /// @}
    /// @name Typing characters and strings
    /// @{

    /** @brief  Informs RetroShell that a key has been typed.
     *  @param  key     The pressed key
     *  @param  shift   Status of the shift key
     */
    void press(RSKey key, bool shift = false);

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
    void execScript(const fs::path &path);
    void execScript(std::stringstream &ss);
    void execScript(const std::ifstream &fs);
    void execScript(const string &contents);
    void execScript(const MediaFile &file);

    /// @}
    /// @name Misc
    /// @{

    /** @brief  Installs a file system in the file system navigator
     */
    void import(const FloppyDrive &dfn);
    void import(const HardDrive &hdn, isize part);
    void importDf(isize n);
    void importHd(isize n, isize part);
    void import(const std::filesystem::path &path, bool recursive = true, bool contents = false);


    /** @brief  Exports the file system in the file system navigator
     */
    void exportBlocks(const std::filesystem::path &path);

    /// @}
};


//
// Misc (RemoteManager)
//

class RemoteManagerAPI : public API {

    friend class VAmiga;
    
public:

    class RemoteManager *remoteManager = nullptr;
    
    /// @name Analyzing the emulator
    /// @{

    /** @brief  Returns the component's current state.
     */
    const RemoteManagerInfo &getInfo() const;
    const RemoteManagerInfo &getCachedInfo() const;

    /// @}
};


//
// Top-level API
//

class VAmiga : public API {
    
public:
    
    static DefaultsAPI defaults;
    
    // Components
    AmigaAPI amiga;
    AgnusAPI agnus;
    CIAAPI ciaA, ciaB;
    CPUAPI cpu;
    DeniseAPI denise;
    MemoryAPI mem;
    PaulaAPI paula;
    RTCAPI rtc;
    
    // Ports
    AudioPortAPI audioPort;
    ControlPortAPI controlPort1, controlPort2;
    SerialPortAPI serialPort;
    VideoPortAPI videoPort;

    // Peripherals
    FloppyDriveAPI df0, df1, df2, df3;
    HardDriveAPI hd0, hd1, hd2, hd3;
    KeyboardAPI keyboard;
    
    // Misc
    GuardsAPI copperBreakpoints; // TODO: Move inside AgnusAPI
    MsgQueueAPI msgQueue;
    DebuggerAPI debugger; // TODO: No longer needed? It's not 'wired'
    RemoteManagerAPI remoteManager;
    RetroShellAPI retroShell;

    // Shortcuts
    FloppyDriveAPI *df[4] = { &df0, &df1, &df2, &df3 };
    HardDriveAPI *hd[4] = { &hd0, &hd1, &hd2, &hd3 };


    //
    // Static methods
    //
    
    /** @brief  Returns a version string for this release.
     */
    static string version();
    
    /** @brief  Returns a build-number string for this release.
     */
    static string build();
    
    
    //
    // Initializing
    //
    
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
    const EmulatorMetrics &getMetrics() const;
    const EmulatorMetrics &getCachedMetrics() const;

    /// @}
    /// @name Querying the emulator state
    /// @{
    
    /** @brief  Returns true iff the emulator if the emulator is powered on.
     */
    bool isPoweredOn() const;
    
    /** @brief  Returns true iff the emulator if the emulator is powered off.
     */
    bool isPoweredOff() const;
    
    /** @brief  Returns true iff the emulator is in paused state.
     */
    bool isPaused() const;
    
    /** @brief  Returns true iff the emulator is running.
     */
    bool isRunning() const;
    
    /** @brief  Returns true iff the emulator has been suspended.
     */
    bool isSuspended() const;
    
    /** @brief  Returns true iff the emulator has shut down.
     */
    bool isHalted() const;
    
    /** @brief  Returns true iff warp mode is active.
     */
    bool isWarping() const;
    
    /** @brief  Returns true iff the emulator runs in track mode.
     */
    bool isTracking() const;
    
    /** @brief  Checks if the emulator is runnable.
     *  The function checks if the necessary ROMs are installed to lauch the
     *  emulator. On success, the functions returns. Otherwise, an exception
     *  is thrown.
     */
    void isReady() const;
    
    
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
    
    /** @brief  Fnishes the current rasterline
     *
     *  Calling this functions runs the CPU until the end of the current rasterline.
     */
    void finishLine();
    
    /** @brief  Fnishes the current frame
     *
     *  Calling this functions runs the CPU until the end of the current frame.
     */
    void finishFrame();
    
    
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
    void launch(const void *listener = nullptr, Callback *func = nullptr);
    
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
    i64 get(Opt option) const;
    
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
    i64 get(Opt option, long id) const;
    
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
    void set(Opt opt, i64 value);
    
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
    void set(Opt opt, i64 value, long id);

    /** @brief  Exports the current configuration.
     *
     *  The current configuration is exported in form of a RetroShell script.
     *  Reading in the script at a later point will restore the configuration.
     *
     *  @param diff When set to true, the script will only contains settings
     *              for items that differ from the default configuration.
     *              This feature is useful for debugging to compare two virtual
     *              machine configurations.
     */
    void exportConfig(const std::filesystem::path &path, bool diff = false) const;
    void exportConfig(std::ostream &stream, bool diff = false) const;
    
    
    /// @}
    /// @name Using the command queue
    /// @{
    
    /** @brief  Feeds a command into the command queue.
     */
    void put(const Command &cmd);
    void put(Cmd type, i64 payload = 0, i64 payload2 = 0) { put(Command(type, payload, payload2)); }
    void put(Cmd type, ConfigCmd payload)  { put(Command(type, payload)); }
    void put(Cmd type, KeyCmd payload)  { put(Command(type, payload)); }
    void put(Cmd type, GamePadCmd payload)  { put(Command(type, payload)); }
    void put(Cmd type, CoordCmd payload)  { put(Command(type, payload)); }
    void put(Cmd type, AlarmCmd payload)  { put(Command(type, payload)); }
    
    /// @}
};

}
