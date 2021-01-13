// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

enum_long(MSG_TYPE)
{
    MSG_NONE = 0,
    
    // Message queue
    MSG_REGISTER,
    MSG_UNREGISTER,
    
    // Emulator state
    MSG_CONFIG,
    MSG_POWER_ON,
    MSG_POWER_OFF,
    MSG_RUN,
    MSG_PAUSE,
    MSG_RESET,
    MSG_WARP_ON,
    MSG_WARP_OFF,
    MSG_MUTE_ON,
    MSG_MUTE_OFF,
    MSG_POWER_LED_ON,
    MSG_POWER_LED_DIM,
    MSG_POWER_LED_OFF,
        
    // CPU
    MSG_BREAKPOINT_CONFIG,
    MSG_BREAKPOINT_REACHED,
    MSG_WATCHPOINT_REACHED,
    MSG_CPU_HALT,

    // Memory
    MSG_MEM_LAYOUT,
        
    // Floppy drives
    MSG_DRIVE_CONNECT,
    MSG_DRIVE_DISCONNECT,
    MSG_DRIVE_SELECT,
    MSG_DRIVE_READ,
    MSG_DRIVE_WRITE,
    MSG_DRIVE_LED_ON,
    MSG_DRIVE_LED_OFF,
    MSG_DRIVE_MOTOR_ON,
    MSG_DRIVE_MOTOR_OFF,
    MSG_DRIVE_HEAD,
    MSG_DRIVE_HEAD_POLL,
    MSG_DISK_INSERT,
    MSG_DISK_EJECT,
    MSG_DISK_SAVED,
    MSG_DISK_UNSAVED,
    MSG_DISK_PROTECT,
    MSG_DISK_UNPROTECT,

    // Keyboard
    MSG_CTRL_AMIGA_AMIGA,
    
    // Ports
    MSG_SER_IN,
    MSG_SER_OUT,

    // Snapshots
    MSG_AUTO_SNAPSHOT_TAKEN,
    MSG_USER_SNAPSHOT_TAKEN,
    MSG_SNAPSHOT_RESTORED,

    // Screen recording
    MSG_RECORDING_STARTED,
    MSG_RECORDING_STOPPED,
    
    // Debugging
    MSG_DMA_DEBUG_ON,
    MSG_DMA_DEBUG_OFF
};
typedef MSG_TYPE MsgType;

inline bool isMsgType(long value)
{
    return (unsigned long)value <= MSG_DMA_DEBUG_OFF;
}

inline const char *MsgTypeName(MsgType type)
{
    switch (type) {
            
        case MSG_NONE:                return "NONE";
        case MSG_REGISTER:            return "REGISTER";
        case MSG_UNREGISTER:          return "UNREGISTER";
            
        case MSG_CONFIG:              return "CONFIG";
        case MSG_POWER_ON:            return "POWER_ON";
        case MSG_POWER_OFF:           return "POWER_OFF";
        case MSG_RUN:                 return "RUN";
        case MSG_PAUSE:               return "PAUSE";
        case MSG_RESET:               return "RESET";
        case MSG_WARP_ON:             return "WARP_ON";
        case MSG_WARP_OFF:            return "WARP_OFF";
        case MSG_MUTE_ON:             return "MUTE_ON";
        case MSG_MUTE_OFF:            return "MUTE_OFF";
        case MSG_POWER_LED_ON:        return "POWER_LED_ON";
        case MSG_POWER_LED_DIM:       return "POWER_LED_DIM";
        case MSG_POWER_LED_OFF:       return "POWER_LED_OFF";
                
        case MSG_BREAKPOINT_CONFIG:   return "BREAKPOINT_CONFIG";
        case MSG_BREAKPOINT_REACHED:  return "BREAKPOINT_REACHED";
        case MSG_WATCHPOINT_REACHED:  return "WATCHPOINT_REACHED";
        case MSG_CPU_HALT:            return "CPU_HALT";

        case MSG_MEM_LAYOUT:          return "LAYOUT";
                
        case MSG_DRIVE_CONNECT:       return "DRIVE_CONNECT";
        case MSG_DRIVE_DISCONNECT:    return "DRIVE_DISCONNECT";
        case MSG_DRIVE_SELECT:        return "DRIVE_SELECT";
        case MSG_DRIVE_READ:          return "DRIVE_READ";
        case MSG_DRIVE_WRITE:         return "DRIVE_WRITE";
        case MSG_DRIVE_LED_ON:        return "DRIVE_LED_ON";
        case MSG_DRIVE_LED_OFF:       return "DRIVE_LED_OFF";
        case MSG_DRIVE_MOTOR_ON:      return "DRIVE_MOTOR_ON";
        case MSG_DRIVE_MOTOR_OFF:     return "DRIVE_MOTOR_OFF";
        case MSG_DRIVE_HEAD:          return "DRIVE_HEAD";
        case MSG_DRIVE_HEAD_POLL:     return "DRIVE_HEAD_POLL";
        case MSG_DISK_INSERT:         return "DISK_INSERT";
        case MSG_DISK_EJECT:          return "DISK_EJECT";
        case MSG_DISK_SAVED:          return "DISK_SAVED";
        case MSG_DISK_UNSAVED:        return "DISK_UNSAVED";
        case MSG_DISK_PROTECT:        return "DISK_PROTECT";
        case MSG_DISK_UNPROTECT:      return "DISK_UNPROTECT";

        case MSG_CTRL_AMIGA_AMIGA:    return "CTRL_AMIGA_AMIGA";
            
        case MSG_SER_IN:              return "SER_IN";
        case MSG_SER_OUT:             return "SER_OUT";

        case MSG_AUTO_SNAPSHOT_TAKEN: return "AUTO_SNAPSHOT_TAKEN";
        case MSG_USER_SNAPSHOT_TAKEN: return "USER_SNAPSHOT_TAKEN";
        case MSG_SNAPSHOT_RESTORED:   return "SNAPSHOT_RESTORED";

        case MSG_RECORDING_STARTED:   return "RECORDING_STARTED";
        case MSG_RECORDING_STOPPED:   return "RECORDING_STOPPED";
            
        case MSG_DMA_DEBUG_ON:        return "DMA_DEBUG_ON";
        case MSG_DMA_DEBUG_OFF:       return "DMA_DEBUG_OFF";            
    }
    return "???";
}

typedef struct
{
    MsgType type;
    long data;
}
Message;

// Callback function signature
typedef void Callback(const void *, long, long);
