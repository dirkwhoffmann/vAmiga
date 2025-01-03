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

//
// Enumerations
//

enum_long(MSG_TYPE)
{
    MSG_NONE = 0,
    
    // Emulator state
    MSG_CONFIG,
    MSG_POWER,
    MSG_RUN,
    MSG_PAUSE,
    MSG_STEP,
    MSG_RESET,
    MSG_SHUTDOWN,
    MSG_ABORT,
    MSG_WARP,
    MSG_TRACK,
    MSG_MUTE,
    MSG_POWER_LED_ON,
    MSG_POWER_LED_DIM,
    MSG_POWER_LED_OFF,

    // Retro shell
    MSG_RSH_CLOSE,          ///< RetroShell has been closed
    MSG_RSH_UPDATE,         ///< RetroShell has generated new output
    MSG_RSH_DEBUGGER,       ///< The RetroShell debugger has been opend or closed
    MSG_RSH_WAIT,           ///< Execution has peen postponed due to a wait command
    MSG_RSH_ERROR,          ///< Command execution has been aborted due to an error

    // Amiga
    MSG_VIDEO_FORMAT,

    // CPU
    MSG_OVERCLOCKING,
    MSG_GUARD_UPDATED,
    MSG_BREAKPOINT_REACHED,
    MSG_WATCHPOINT_REACHED,
    MSG_CATCHPOINT_REACHED,
    MSG_SWTRAP_REACHED,
    MSG_CPU_HALT,
    
    // Agnus
    MSG_EOL_REACHED,
    MSG_EOF_REACHED,
    MSG_BEAMTRAP_REACHED,
    MSG_BEAMTRAP_UPDATED,

    // Copper
    MSG_COPPERBP_REACHED,
    MSG_COPPERBP_UPDATED,
    MSG_COPPERWP_REACHED,
    MSG_COPPERWP_UPDATED,

    // Denise
    MSG_VIEWPORT,
    
    // Memory
    MSG_MEM_LAYOUT,
        
    // Floppy drives
    MSG_DRIVE_CONNECT,
    MSG_DRIVE_SELECT,
    MSG_DRIVE_READ,
    MSG_DRIVE_WRITE,
    MSG_DRIVE_LED,
    MSG_DRIVE_MOTOR,
    MSG_DRIVE_STEP,
    MSG_DRIVE_POLL,
    MSG_DISK_INSERT,
    MSG_DISK_EJECT,
    MSG_DISK_PROTECTED,

    // Hard drive controllers
    MSG_HDC_CONNECT,
    MSG_HDC_STATE,
    
    // Hard drives
    MSG_HDR_STEP,
    MSG_HDR_READ,
    MSG_HDR_WRITE,
    MSG_HDR_IDLE,
    
    // Keyboard
    MSG_CTRL_AMIGA_AMIGA,
    
    // Mouse
    MSG_SHAKING,
    
    // Ports
    MSG_SER_IN,
    MSG_SER_OUT,

    // Snapshots
    MSG_SNAPSHOT_TAKEN,
    MSG_SNAPSHOT_RESTORED,

    // Screen recording
    MSG_RECORDING_STARTED,
    MSG_RECORDING_STOPPED,
    MSG_RECORDING_ABORTED,
        
    // DMA Debugging
    MSG_DMA_DEBUG,

    // Remote server
    MSG_SRV_STATE,
    MSG_SRV_RECEIVE,
    MSG_SRV_SEND,

    // Scheduled alarms
    MSG_ALARM
};
typedef MSG_TYPE MsgType;

#ifdef __cplusplus
struct MsgTypeEnum : vamiga::util::Reflection<MsgTypeEnum, MsgType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = MSG_ALARM;

    static const char *prefix() { return "MSG"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case MSG_NONE:                  return "NONE";

            case MSG_CONFIG:                return "CONFIG";
            case MSG_POWER:                 return "POWER";
            case MSG_RUN:                   return "RUN";
            case MSG_PAUSE:                 return "PAUSE";
            case MSG_STEP:                  return "STEP";
            case MSG_RESET:                 return "RESET";
            case MSG_SHUTDOWN:              return "SHUTDOWN";
            case MSG_ABORT:                 return "ABORT";
            case MSG_WARP:                  return "WARP";
            case MSG_TRACK:                 return "TRACK";
            case MSG_MUTE:                  return "MUTE";
            case MSG_POWER_LED_ON:          return "POWER_LED_ON";
            case MSG_POWER_LED_DIM:         return "POWER_LED_DIM";
            case MSG_POWER_LED_OFF:         return "POWER_LED_OFF";

            case MSG_RSH_CLOSE:             return "RSH_CLOSE";
            case MSG_RSH_UPDATE:            return "RSH_UPDATE";
            case MSG_RSH_DEBUGGER:          return "RSH_DEBUGGER";
            case MSG_RSH_WAIT:              return "RSH_WAIT";
            case MSG_RSH_ERROR:             return "RSH_ERROR";

            case MSG_VIDEO_FORMAT:          return "VIDEO_FORMAT";
                
            case MSG_OVERCLOCKING:          return "OVERCLOCKING";
            case MSG_GUARD_UPDATED:         return "GUARD_UPDATED";
            case MSG_BREAKPOINT_REACHED:    return "BREAKPOINT_REACHED";
            case MSG_WATCHPOINT_REACHED:    return "WATCHPOINT_REACHED";
            case MSG_CATCHPOINT_REACHED:    return "CATCHPOINT_REACHED";
            case MSG_SWTRAP_REACHED:        return "SWTRAP_REACHED";
            case MSG_CPU_HALT:              return "CPU_HALT";

            case MSG_EOL_REACHED:           return "EOL_REACHED";
            case MSG_EOF_REACHED:           return "EOF_REACHED";
            case MSG_BEAMTRAP_REACHED:      return "BEAMTRAP_REACHED";
            case MSG_BEAMTRAP_UPDATED:      return "BEAMTRAP_UPDATED";

            case MSG_COPPERBP_REACHED:      return "COPPERBP_REACHED";
            case MSG_COPPERBP_UPDATED:      return "COPPERBP_UPDATED";
            case MSG_COPPERWP_REACHED:      return "COPPERWP_REACHED";
            case MSG_COPPERWP_UPDATED:      return "COPPERWP_UPDATED";
                
            case MSG_VIEWPORT:              return "VIEWPORT";
                
            case MSG_MEM_LAYOUT:            return "MEM_LAYOUT";
                    
            case MSG_DRIVE_CONNECT:         return "DRIVE_CONNECT";
            case MSG_DRIVE_SELECT:          return "DRIVE_SELECT";
            case MSG_DRIVE_READ:            return "DRIVE_READ";
            case MSG_DRIVE_WRITE:           return "DRIVE_WRITE";
            case MSG_DRIVE_LED:             return "DRIVE_LED";
            case MSG_DRIVE_MOTOR:           return "DRIVE_MOTOR";
            case MSG_DRIVE_STEP:            return "DRIVE_STEP";
            case MSG_DRIVE_POLL:            return "DRIVE_POLL";
            case MSG_DISK_INSERT:           return "DISK_INSERT";
            case MSG_DISK_EJECT:            return "DISK_EJECT";
            case MSG_DISK_PROTECTED:        return "DISK_PROTECTED";

            case MSG_HDC_CONNECT:           return "HDC_CONNECT";
            case MSG_HDC_STATE:             return "HDC_STATE";
                
            case MSG_HDR_STEP:              return "HDR_STEP";
            case MSG_HDR_READ:              return "HDR_READ";
            case MSG_HDR_WRITE:             return "HDR_WRITE";
            case MSG_HDR_IDLE:              return "HDR_IDLE";
                
            case MSG_CTRL_AMIGA_AMIGA:      return "CTRL_AMIGA_AMIGA";

            case MSG_SHAKING:               return "SHAKING";

            case MSG_SER_IN:                return "SER_IN";
            case MSG_SER_OUT:               return "SER_OUT";

            case MSG_SNAPSHOT_TAKEN:        return "SNAPSHOT_TAKEN";
            case MSG_SNAPSHOT_RESTORED:     return "SNAPSHOT_RESTORED";
                
            case MSG_RECORDING_STARTED:     return "RECORDING_STARTED";
            case MSG_RECORDING_STOPPED:     return "RECORDING_STOPPED";
            case MSG_RECORDING_ABORTED:     return "RECORDING_ABORTED";
                                
            case MSG_DMA_DEBUG:             return "DMA_DEBUG";
                                
            case MSG_SRV_STATE:             return "SRV_STATE";
            case MSG_SRV_RECEIVE:           return "SRV_RECEIVE";
            case MSG_SRV_SEND:              return "SRV_SEND";

            case MSG_ALARM:                 return "ALARM";
        }
        return "???";
    }
};
#endif


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
