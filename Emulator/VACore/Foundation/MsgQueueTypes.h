// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HdControllerTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class Msg : long
{
    NONE = 0,
    
    // Emulator state
    CONFIG,
    POWER,
    RUN,
    PAUSE,
    STEP,
    RESET,
    SHUTDOWN,
    ABORT,
    WARP,
    TRACK,
    MUTE,
    POWER_LED_ON,
    POWER_LED_DIM,
    POWER_LED_OFF,
    
    // Retro shell
    RSH_CLOSE,          ///< RetroShell has been closed
    RSH_UPDATE,         ///< RetroShell has generated new output
    RSH_DEBUGGER,       ///< The RetroShell debugger has been opend or closed
    RSH_WAIT,           ///< Execution has peen postponed due to a wait command
    RSH_ERROR,          ///< Command execution has been aborted due to an error
    
    // Amiga
    VIDEO_FORMAT,
    
    // CPU
    OVERCLOCKING,
    GUARD_UPDATED,
    BREAKPOINT_REACHED,
    WATCHPOINT_REACHED,
    CATCHPOINT_REACHED,
    SWTRAP_REACHED,
    CPU_HALT,
    
    // Agnus
    EOL_REACHED,
    EOF_REACHED,
    BEAMTRAP_REACHED,
    BEAMTRAP_UPDATED,
    
    // Copper
    COPPERBP_REACHED,
    COPPERBP_UPDATED,
    COPPERWP_REACHED,
    COPPERWP_UPDATED,
    
    // Denise
    VIEWPORT,
    
    // Memory
    MEM_LAYOUT,
    
    // Floppy drives
    DRIVE_CONNECT,
    DRIVE_SELECT,
    DRIVE_READ,
    DRIVE_WRITE,
    DRIVE_LED,
    DRIVE_MOTOR,
    DRIVE_STEP,
    DRIVE_POLL,
    DISK_INSERT,
    DISK_EJECT,
    DISK_PROTECTED,
    
    // Hard drive controllers
    HDC_CONNECT,
    HDC_STATE,
    
    // Hard drives
    HDR_STEP,
    HDR_READ,
    HDR_WRITE,
    HDR_IDLE,
    
    // Keyboard
    CTRL_AMIGA_AMIGA,
    
    // Mouse
    SHAKING,
    
    // Ports
    SER_IN,
    SER_OUT,
    
    // Snapshots
    SNAPSHOT_TAKEN,
    SNAPSHOT_RESTORED,
    
    // Workspaces
    WORKSPACE_LOADED,
    WORKSPACE_SAVED,

    // Screen recording
    RECORDING_STARTED,
    RECORDING_STOPPED,
    RECORDING_ABORTED,
    
    // DMA Debugging
    DMA_DEBUG,
    
    // Remote server
    SRV_STATE,
    SRV_RECEIVE,
    SRV_SEND,
    
    // Scheduled alarms
    ALARM
};

struct MsgEnum : Reflection<MsgEnum, Msg>
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
            case Msg::RESET:                 return "RESET";
            case Msg::SHUTDOWN:              return "SHUTDOWN";
            case Msg::ABORT:                 return "ABORT";
            case Msg::WARP:                  return "WARP";
            case Msg::TRACK:                 return "TRACK";
            case Msg::MUTE:                  return "MUTE";
            case Msg::POWER_LED_ON:          return "POWER_LED_ON";
            case Msg::POWER_LED_DIM:         return "POWER_LED_DIM";
            case Msg::POWER_LED_OFF:         return "POWER_LED_OFF";
                
            case Msg::RSH_CLOSE:             return "RSH_CLOSE";
            case Msg::RSH_UPDATE:            return "RSH_UPDATE";
            case Msg::RSH_DEBUGGER:          return "RSH_DEBUGGER";
            case Msg::RSH_WAIT:              return "RSH_WAIT";
            case Msg::RSH_ERROR:             return "RSH_ERROR";
                
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
            case Msg::BEAMTRAP_UPDATED:      return "BEAMTRAP_UPDATED";
                
            case Msg::COPPERBP_REACHED:      return "COPPERBP_REACHED";
            case Msg::COPPERBP_UPDATED:      return "COPPERBP_UPDATED";
            case Msg::COPPERWP_REACHED:      return "COPPERWP_REACHED";
            case Msg::COPPERWP_UPDATED:      return "COPPERWP_UPDATED";
                
            case Msg::VIEWPORT:              return "VIEWPORT";
                
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
                
            case Msg::HDR_STEP:              return "HDR_STEP";
            case Msg::HDR_READ:              return "HDR_READ";
            case Msg::HDR_WRITE:             return "HDR_WRITE";
            case Msg::HDR_IDLE:              return "HDR_IDLE";
                
            case Msg::CTRL_AMIGA_AMIGA:      return "CTRL_AMIGA_AMIGA";
                
            case Msg::SHAKING:               return "SHAKING";
                
            case Msg::SER_IN:                return "SER_IN";
            case Msg::SER_OUT:               return "SER_OUT";
                
            case Msg::SNAPSHOT_TAKEN:        return "SNAPSHOT_TAKEN";
            case Msg::SNAPSHOT_RESTORED:     return "SNAPSHOT_RESTORED";
                
            case Msg::WORKSPACE_LOADED:      return "WORKSPACE_LOADED";
            case Msg::WORKSPACE_SAVED:       return "WORKSPACE_SAVED";
                
            case Msg::RECORDING_STARTED:     return "RECORDING_STARTED";
            case Msg::RECORDING_STOPPED:     return "RECORDING_STOPPED";
            case Msg::RECORDING_ABORTED:     return "RECORDING_ABORTED";
                
            case Msg::DMA_DEBUG:             return "DMA_DEBUG";
                
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
typedef struct { i16 nr; i16 value; i16 volume; i16 pan; } DriveMsg;
typedef struct { i16 nr; HdcState state; } HdcMsg;
typedef struct { i16 hstrt; i16 vstrt; i16 hstop; i16 vstop; } ViewportMsg;
typedef struct { isize line; i16 delay; } ScriptMsg;
typedef struct { void *snapshot; } SnapshotMsg;

typedef struct
{
    // Header
    Msg type;
    
    // Payload
    union {
        i64 value;
        CpuMsg cpu;
        DriveMsg drive;
        HdcMsg hdc;
        ScriptMsg script;
        ViewportMsg viewport;
        SnapshotMsg snapshot;
    };
}
Message;


//
// Signatures
//

typedef void Callback(const void *, Message);

}
