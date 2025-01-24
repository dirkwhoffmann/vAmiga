// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"
#include "HdControllerTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class MsgType : long
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

struct MsgTypeEnum : util::Reflection<MsgTypeEnum, MsgType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(MsgType::ALARM);
    
    // static const char *prefix() { return "MSG"; }
    static const char *_key(MsgType value)
    {
        switch (value) {
                
            case MsgType::NONE:                  return "NONE";
                
            case MsgType::CONFIG:                return "CONFIG";
            case MsgType::POWER:                 return "POWER";
            case MsgType::RUN:                   return "RUN";
            case MsgType::PAUSE:                 return "PAUSE";
            case MsgType::STEP:                  return "STEP";
            case MsgType::RESET:                 return "RESET";
            case MsgType::SHUTDOWN:              return "SHUTDOWN";
            case MsgType::ABORT:                 return "ABORT";
            case MsgType::WARP:                  return "WARP";
            case MsgType::TRACK:                 return "TRACK";
            case MsgType::MUTE:                  return "MUTE";
            case MsgType::POWER_LED_ON:          return "POWER_LED_ON";
            case MsgType::POWER_LED_DIM:         return "POWER_LED_DIM";
            case MsgType::POWER_LED_OFF:         return "POWER_LED_OFF";
                
            case MsgType::RSH_CLOSE:             return "RSH_CLOSE";
            case MsgType::RSH_UPDATE:            return "RSH_UPDATE";
            case MsgType::RSH_DEBUGGER:          return "RSH_DEBUGGER";
            case MsgType::RSH_WAIT:              return "RSH_WAIT";
            case MsgType::RSH_ERROR:             return "RSH_ERROR";
                
            case MsgType::VIDEO_FORMAT:          return "VIDEO_FORMAT";
                
            case MsgType::OVERCLOCKING:          return "OVERCLOCKING";
            case MsgType::GUARD_UPDATED:         return "GUARD_UPDATED";
            case MsgType::BREAKPOINT_REACHED:    return "BREAKPOINT_REACHED";
            case MsgType::WATCHPOINT_REACHED:    return "WATCHPOINT_REACHED";
            case MsgType::CATCHPOINT_REACHED:    return "CATCHPOINT_REACHED";
            case MsgType::SWTRAP_REACHED:        return "SWTRAP_REACHED";
            case MsgType::CPU_HALT:              return "CPU_HALT";
                
            case MsgType::EOL_REACHED:           return "EOL_REACHED";
            case MsgType::EOF_REACHED:           return "EOF_REACHED";
            case MsgType::BEAMTRAP_REACHED:      return "BEAMTRAP_REACHED";
            case MsgType::BEAMTRAP_UPDATED:      return "BEAMTRAP_UPDATED";
                
            case MsgType::COPPERBP_REACHED:      return "COPPERBP_REACHED";
            case MsgType::COPPERBP_UPDATED:      return "COPPERBP_UPDATED";
            case MsgType::COPPERWP_REACHED:      return "COPPERWP_REACHED";
            case MsgType::COPPERWP_UPDATED:      return "COPPERWP_UPDATED";
                
            case MsgType::VIEWPORT:              return "VIEWPORT";
                
            case MsgType::MEM_LAYOUT:            return "MEM_LAYOUT";
                
            case MsgType::DRIVE_CONNECT:         return "DRIVE_CONNECT";
            case MsgType::DRIVE_SELECT:          return "DRIVE_SELECT";
            case MsgType::DRIVE_READ:            return "DRIVE_READ";
            case MsgType::DRIVE_WRITE:           return "DRIVE_WRITE";
            case MsgType::DRIVE_LED:             return "DRIVE_LED";
            case MsgType::DRIVE_MOTOR:           return "DRIVE_MOTOR";
            case MsgType::DRIVE_STEP:            return "DRIVE_STEP";
            case MsgType::DRIVE_POLL:            return "DRIVE_POLL";
            case MsgType::DISK_INSERT:           return "DISK_INSERT";
            case MsgType::DISK_EJECT:            return "DISK_EJECT";
            case MsgType::DISK_PROTECTED:        return "DISK_PROTECTED";
                
            case MsgType::HDC_CONNECT:           return "HDC_CONNECT";
            case MsgType::HDC_STATE:             return "HDC_STATE";
                
            case MsgType::HDR_STEP:              return "HDR_STEP";
            case MsgType::HDR_READ:              return "HDR_READ";
            case MsgType::HDR_WRITE:             return "HDR_WRITE";
            case MsgType::HDR_IDLE:              return "HDR_IDLE";
                
            case MsgType::CTRL_AMIGA_AMIGA:      return "CTRL_AMIGA_AMIGA";
                
            case MsgType::SHAKING:               return "SHAKING";
                
            case MsgType::SER_IN:                return "SER_IN";
            case MsgType::SER_OUT:               return "SER_OUT";
                
            case MsgType::SNAPSHOT_TAKEN:        return "SNAPSHOT_TAKEN";
            case MsgType::SNAPSHOT_RESTORED:     return "SNAPSHOT_RESTORED";
                
            case MsgType::RECORDING_STARTED:     return "RECORDING_STARTED";
            case MsgType::RECORDING_STOPPED:     return "RECORDING_STOPPED";
            case MsgType::RECORDING_ABORTED:     return "RECORDING_ABORTED";
                
            case MsgType::DMA_DEBUG:             return "DMA_DEBUG";
                
            case MsgType::SRV_STATE:             return "SRV_STATE";
            case MsgType::SRV_RECEIVE:           return "SRV_RECEIVE";
            case MsgType::SRV_SEND:              return "SRV_SEND";
                
            case MsgType::ALARM:                 return "ALARM";
        }
        return "???";
    }
    
    static const char *help(MsgType value)
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
    MsgType type;
    
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
