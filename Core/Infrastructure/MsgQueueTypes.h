// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <exception>
#include "HdControllerTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class Msg : long
{
    NONE,               ///< No message
    
    // Emulator state
    CONFIG,             ///< A configuration request has been carried out (the exception is attached if it failed)
    POWER,              ///< The emulator was switched on (value = 1) or off (value = 0)
    RUN,                ///< The emulator started to run
    PAUSE,              ///< The emulator was paused
    STEP,               ///< The emulator completed a single step (string attached)
    STOP,               ///< A remote debugger requested a stop
    RESET,              ///< A hard reset was performed
    SHUTDOWN,           ///< The emulator thread is about to terminate
    ABORT,              ///< The application is asked to terminate (value = exit code)
    WARP,               ///< Warp mode was switched on (value = 1) or off (value = 0)
    TRACK,              ///< Track mode was switched on (value = 1) or off (value = 0)
    MUTE,               ///< The audio output was muted (value = 1) or unmuted (value = 0)
    EASTER_EGG,         ///< The easter egg was triggered
    POWER_LED_ON,       ///< The power LED is on
    POWER_LED_DIM,      ///< The power LED is dimmed
    POWER_LED_OFF,      ///< The power LED is off
    
    // Retro shell
    RSH_CLOSE,          ///< RetroShell has been closed
    RSH_UPDATE,         ///< RetroShell has generated new output
    RSH_SWITCH,         ///< RetroShell has switched to another console
    RSH_WAIT,           ///< Execution has peen postponed due to a wait command
    RSH_ERROR,          ///< Command execution has been aborted due to an error
    RSH_DONE,           ///< All pending commands have been executed
    RSH_EXPORT,         ///< Data has been exported to the file system

    // Amiga
    VIDEO_FORMAT,       ///< The video format has changed (value = PAL or NTSC)
    
    // CPU
    OVERCLOCKING,       ///< The CPU overclocking factor has changed (value = factor)
    GUARD_UPDATED,      ///< A guard list has been modified (value = GuardType)
    BREAKPOINT_REACHED, ///< A CPU breakpoint was reached (cpu.pc = address)
    WATCHPOINT_REACHED, ///< A CPU watchpoint was reached (cpu.pc = watched address)
    CATCHPOINT_REACHED, ///< A CPU catchpoint was reached (cpu.pc = program counter, cpu.vector = vector)
    SWTRAP_REACHED,     ///< A software trap was reached (cpu.pc = program counter)
    CPU_HALT,           ///< The CPU has halted
    
    // Agnus
    EOL_REACHED,        ///< The end of a raster line was reached (string attached)
    EOF_REACHED,        ///< The end of a frame was reached (string attached)
    BEAMTRAP_REACHED,   ///< A beamtrap was reached (beam.v, beam.h = beam position)
    
    // Copper
    COPPERBP_REACHED,   ///< A Copper breakpoint was reached (cpu.pc = Copper address)
    COPPERWP_REACHED,   ///< A Copper watchpoint was reached (cpu.pc = watched address)
    
    // Denise
    VIEWPORT,           ///< The visible region of the texture has changed (viewport = coordinates)

    // Audio
    AUDBUF_UNDERFLOW,   ///< The audio buffer ran empty (value = milliseconds since the previous underflow)
    AUDBUF_OVERFLOW,    ///< The audio buffer ran full (value = milliseconds since the previous overflow)

    // Memory
    MEM_LAYOUT,         ///< The memory layout (bank mapping) has changed
    
    // Floppy drives
    DRIVE_CONNECT,      ///< A floppy drive was connected or disconnected (drive.nr = drive, drive.value = connected)
    DRIVE_SELECT,       ///< A floppy drive was selected (value = drive)
    DRIVE_READ,         ///< A floppy drive is reading (value = drive)
    DRIVE_WRITE,        ///< A floppy drive is writing (value = drive)
    DRIVE_LED,          ///< The LED of a floppy drive changed (drive.nr = drive, drive.value = on or off)
    DRIVE_MOTOR,        ///< The motor of a floppy drive changed (drive.nr = drive, drive.value = on or off)
    DRIVE_STEP,         ///< A floppy drive moved its head (drive.nr = drive, drive.value = cylinder, drive.volume, drive.pan)
    DRIVE_POLL,         ///< A floppy drive without a disk moved its head to poll for one (same payload as DRIVE_STEP)
    DISK_INSERT,        ///< A disk was inserted (drive.nr = drive, drive.volume, drive.pan; carries the exception if it failed)
    DISK_EJECT,         ///< A disk was ejected (drive.nr = drive, drive.volume, drive.pan)
    DISK_PROTECTED,     ///< The write protection of the disk changed (value = protected)
    
    // Hard drive controllers
    HDC_CONNECT,        ///< A hard drive controller was connected (drive.nr = drive)
    HDC_STATE,          ///< The state of a hard drive controller changed (hdc.nr = drive, hdc.state)
    
    // Hard drives
    HDR_ATTACH,         ///< A hard drive has been attached (value: drive; carries the exception if it failed)
    HDR_IMPORTED,       ///< A folder has been imported (value: drive; carries the exception if it failed)
    HDR_DETACH,         ///< A hard drive was detached (value = drive)
    HDR_STEP,           ///< A hard drive moved its head (drive.nr = drive, drive.value = cylinder, drive.volume, drive.pan)
    HDR_READ,           ///< A hard drive is reading
    HDR_WRITE,          ///< A hard drive is writing
    HDR_IDLE,           ///< A hard drive became idle (value = drive)
    
    // Monitor
    MON_SETTING,        ///< A monitor setting has changed (value = option, value2 = new value)
    
    // Keyboard
    CTRL_AMIGA_AMIGA,   ///< The reset key combination (Ctrl + both Amiga keys) was pressed
    KB_PRESS,           ///< A key has been pressed
    KB_RELEASE,         ///< A key has been released
    KB_LOCK,            ///< A key has been locked
    KB_UNLOCK,          ///< A key has been unlocked
    
    // Mouse
    SHAKING,            ///< The mouse was shaken
    
    // Ports
    SER_IN,             ///< Data arrived at the serial port while its log buffer was empty
    SER_OUT,            ///< Data was sent from the serial port while its log buffer was empty
    
    // ROMs
    ROM_LOADED,         ///< A ROM was loaded (value = 0: Kickstart ROM, 1: extension ROM; carries the exception if it failed)

    // Snapshots
    SNAPSHOT_TAKEN,     ///< An automatic snapshot was taken (snapshot.snapshot = heap object that the receiver must delete)
    SNAPSHOT_RESTORED,  ///< A snapshot was restored (carries the exception if it failed)
    SNAPSHOT_SAVED,     ///< A snapshot has been saved (carries the exception if it failed)
    
    // Workspaces
    WORKSPACE_LOADED,   ///< A workspace was loaded
    WORKSPACE_SAVED,    ///< A workspace has been saved (carries the exception if it failed)

    // Hibernation
    HIBERNATED,         ///< The machine has been hibernated (carries the exception if it failed)

    // Command queue
    QUEUE_FULL,         ///< The command queue is full and commands are dropped

    // DMA Debugging
    XRAY,               ///< The X-Ray mode has changed (value = mode)
    
    // Remote server
    SRV_STATE,          ///< A remote server changed its state (value = SrvState)
    SRV_RECEIVE,        ///< A remote server received data (value = server, value2 = log entry, string = packet)
    SRV_SEND,           ///< A remote server sent data (value = server, value2 = log entry, string = packet)
    
    // Scheduled alarms
    ALARM               ///< A scheduled alarm went off (value = the payload given when the alarm was set)
};

struct MsgEnum : Reflectable<MsgEnum, Msg>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Msg::ALARM);
    
    static const char *_key(Msg value)
    {
        switch (value) {
                
            case Msg::NONE:                  return "NONE";
                
            case Msg::CONFIG:                return "CONFIG";
            case Msg::POWER:                 return "POWER";
            case Msg::RUN:                   return "RUN";
            case Msg::PAUSE:                 return "PAUSE";
            case Msg::STEP:                  return "STEP";
            case Msg::STOP:                  return "STOP";
            case Msg::RESET:                 return "RESET";
            case Msg::SHUTDOWN:              return "SHUTDOWN";
            case Msg::ABORT:                 return "ABORT";
            case Msg::WARP:                  return "WARP";
            case Msg::TRACK:                 return "TRACK";
            case Msg::MUTE:                  return "MUTE";
            case Msg::EASTER_EGG:            return "EASTER_EGG";
            case Msg::POWER_LED_ON:          return "POWER_LED_ON";
            case Msg::POWER_LED_DIM:         return "POWER_LED_DIM";
            case Msg::POWER_LED_OFF:         return "POWER_LED_OFF";
                
            case Msg::RSH_CLOSE:             return "RSH_CLOSE";
            case Msg::RSH_UPDATE:            return "RSH_UPDATE";
            case Msg::RSH_SWITCH:            return "RSH_SWITCH";
            case Msg::RSH_WAIT:              return "RSH_WAIT";
            case Msg::RSH_ERROR:             return "RSH_ERROR";
            case Msg::RSH_DONE:              return "RSH_DONE";
            case Msg::RSH_EXPORT:            return "RSH_EXPORT";
                
            case Msg::VIDEO_FORMAT:          return "VIDEO_FORMAT";
                
            case Msg::OVERCLOCKING:          return "OVERCLOCKING";
            case Msg::GUARD_UPDATED:         return "GUARD_UPDATED";
            case Msg::BREAKPOINT_REACHED:    return "BREAKPOINT_REACHED";
            case Msg::WATCHPOINT_REACHED:    return "WATCHPOINT_REACHED";
            case Msg::CATCHPOINT_REACHED:    return "CATCHPOINT_REACHED";
            case Msg::SWTRAP_REACHED:        return "SWTRAP_REACHED";
            case Msg::CPU_HALT:              return "CPU_HALT";
                
            case Msg::EOL_REACHED:           return "EOL_REACHED";
            case Msg::EOF_REACHED:           return "EOF_REACHED";
            case Msg::BEAMTRAP_REACHED:      return "BEAMTRAP_REACHED";
                
            case Msg::COPPERBP_REACHED:      return "COPPERBP_REACHED";
            case Msg::COPPERWP_REACHED:      return "COPPERWP_REACHED";
                
            case Msg::VIEWPORT:              return "VIEWPORT";

            case Msg::AUDBUF_UNDERFLOW:      return "AUDBUF_UNDERFLOW";
            case Msg::AUDBUF_OVERFLOW:       return "AUDBUF_OVERFLOW";

            case Msg::MEM_LAYOUT:            return "MEM_LAYOUT";
                
            case Msg::DRIVE_CONNECT:         return "DRIVE_CONNECT";
            case Msg::DRIVE_SELECT:          return "DRIVE_SELECT";
            case Msg::DRIVE_READ:            return "DRIVE_READ";
            case Msg::DRIVE_WRITE:           return "DRIVE_WRITE";
            case Msg::DRIVE_LED:             return "DRIVE_LED";
            case Msg::DRIVE_MOTOR:           return "DRIVE_MOTOR";
            case Msg::DRIVE_STEP:            return "DRIVE_STEP";
            case Msg::DRIVE_POLL:            return "DRIVE_POLL";
            case Msg::DISK_INSERT:           return "DISK_INSERT";
            case Msg::DISK_EJECT:            return "DISK_EJECT";
            case Msg::DISK_PROTECTED:        return "DISK_PROTECTED";
                
            case Msg::HDC_CONNECT:           return "HDC_CONNECT";
            case Msg::HDC_STATE:             return "HDC_STATE";
                
            case Msg::HDR_ATTACH:            return "HDR_ATTACH";
            case Msg::HDR_IMPORTED:          return "HDR_IMPORTED";
            case Msg::HDR_DETACH:            return "HDR_DETACH";
            case Msg::HDR_STEP:              return "HDR_STEP";
            case Msg::HDR_READ:              return "HDR_READ";
            case Msg::HDR_WRITE:             return "HDR_WRITE";
            case Msg::HDR_IDLE:              return "HDR_IDLE";
                
            case Msg::MON_SETTING:           return "MON_SETTING";

            case Msg::CTRL_AMIGA_AMIGA:      return "CTRL_AMIGA_AMIGA";
            case Msg::KB_PRESS:              return "KB_PRESS";
            case Msg::KB_RELEASE:            return "KB_RELEASE";
            case Msg::KB_LOCK:               return "KB_LOCK";
            case Msg::KB_UNLOCK:             return "KB_UNLOCK";
                
            case Msg::SHAKING:               return "SHAKING";
                
            case Msg::SER_IN:                return "SER_IN";
            case Msg::SER_OUT:               return "SER_OUT";
                
            case Msg::ROM_LOADED:            return "ROM_LOADED";
            case Msg::SNAPSHOT_TAKEN:        return "SNAPSHOT_TAKEN";
            case Msg::SNAPSHOT_RESTORED:     return "SNAPSHOT_RESTORED";
            case Msg::SNAPSHOT_SAVED:        return "SNAPSHOT_SAVED";
            case Msg::HIBERNATED:            return "HIBERNATED";
            case Msg::QUEUE_FULL:            return "QUEUE_FULL";
                
            case Msg::WORKSPACE_LOADED:      return "WORKSPACE_LOADED";
            case Msg::WORKSPACE_SAVED:       return "WORKSPACE_SAVED";

            case Msg::XRAY:             return "XRAY";
                
            case Msg::SRV_STATE:             return "SRV_STATE";
            case Msg::SRV_RECEIVE:           return "SRV_RECEIVE";
            case Msg::SRV_SEND:              return "SRV_SEND";
                
            case Msg::ALARM:                 return "ALARM";
        }
        return "???";
    }
    
    static const char *help(Msg value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct { u32 pc; u8 vector; } CpuMsg;
typedef struct { i16 v; i16 h; } BeamMsg;
/* Reports a configuration request. The option is -1 if the request has changed
 * several options (e.g., a configuration scheme, in which case 'value' is the
 * scheme) or if it is not known which ones. The component id is -1 if the
 * request was not targeted at a single component.
 */
typedef struct { i32 option; i32 id; i64 value; } ConfigMsg;
typedef struct { i16 nr; i16 value; i16 volume; i16 pan; } DriveMsg;
typedef struct { i16 nr; HdcState state; } HdcMsg;
typedef struct { isize line; i16 delay; } ScriptMsg;
typedef struct { void *snapshot; } SnapshotMsg;
typedef struct { i16 hstrt; i16 vstrt; i16 hstop; i16 vstop; } ViewportMsg; // hstrt/hstop in super-hires pixels

typedef struct
{
    // Header
    Msg type;

    // Payload
    union {
        struct { i64 value; i64 value2; };
        CpuMsg cpu;
        ConfigMsg config;
        BeamMsg beam;
        DriveMsg drive;
        HdcMsg hdc;
        ScriptMsg script;
        SnapshotMsg snapshot;
        ViewportMsg viewport;
    };

    // Attached string (may be null)
    const char *str;
}
Message;


//
// Signatures
//

/* The callback receives the message and, for a message that reports a
 * failure, the exception that caused it (null otherwise). Pointers inside the
 * message are only valid for the duration of the call; the exception is a
 * shared handle and can be kept.
 */
typedef void Callback(const void *, Message, std::exception_ptr);

}
