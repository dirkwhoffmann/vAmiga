// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _MESSAGE_QUEUE_T_INC
#define _MESSAGE_QUEUE_T_INC

// List of all known message id's
typedef enum
{
    MSG_NONE = 0,
    
    // Queue
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
    MSG_DMA_DEBUG_ON,
    MSG_DMA_DEBUG_OFF,
    
    // CPU
    MSG_BREAKPOINT_CONFIG,
    MSG_BREAKPOINT_REACHED,
    MSG_WATCHPOINT_REACHED,

    // Memory
    MSG_MEM_LAYOUT,
    
    // Keyboard
    // MSG_MAP_CMD_KEYS,
    // MSG_UNMAP_CMD_KEYS,
    
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
    MSG_DISK_INSERTED,
    MSG_DISK_EJECTED,
    MSG_DISK_SAVED,
    MSG_DISK_UNSAVED,
    MSG_DISK_PROTECTED,
    MSG_DISK_UNPROTECTED,

    // Keyboard
    MSG_CTRL_AMIGA_AMIGA,
    
    // Ports
    MSG_SER_IN,
    MSG_SER_OUT,

    // Snapshots
    MSG_AUTO_SNAPSHOT_TAKEN,
    MSG_USER_SNAPSHOT_TAKEN
}
MessageType;

/* A single message
 * Only a very messages utilize the data file. E.g., the drive related message
 * use it to code the drive number (0 = df0 etc.).
 */
typedef struct
{
    MessageType type;
    long data;
}
Message;

// Callback function signature
typedef void Callback(const void *, unsigned, long);

#endif
