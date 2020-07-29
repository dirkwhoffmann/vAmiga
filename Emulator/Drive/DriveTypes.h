// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DRIVE_TYPES_H
#define _DRIVE_TYPES_H

#include "AmigaEnums.h"

//
// Enumerations
//

typedef VA_ENUM(long, DriveType)
{
    DRIVE_35_DD,
    DRIVE_35_DD_PC,
    DRIVE_35_HD,
    DRIVE_35_HD_PC,
    DRIVE_525_SD
};

inline bool isDriveType(long value)
{
    return value >= DRIVE_35_DD && value <= DRIVE_525_SD;
}

inline const char *driveTypeName(DriveType type)
{
    assert(isDriveType(type));
    
    switch (type) {
        case DRIVE_35_DD:    return "Drive 3.5\" DD";
        case DRIVE_35_DD_PC: return "Drive 3.5\" DD (PC)";
        case DRIVE_35_HD:    return "Drive 3.5\" HD";
        case DRIVE_35_HD_PC: return "Drive 3.5\" HD (PC)";
        case DRIVE_525_SD:   return "Drive 5.25\" SD";
        default:             return "???";
    }
}

//
// Structures
//

typedef struct
{
     u8 side;
     u8 cylinder;
     u16 offset;
 }
DriveHead;

typedef struct
{
    /* Drive type
     * At the moment, we only support standard 3.5" DD drives.
     */
     DriveType type;

    /* Acceleration factor
     * This value equals the number of words that get transfered into memory
     * during a single disk DMA cycle. This value must be 1 to emulate a real
     * Amiga. If it set to, e.g., 2, the drive loads twice as fast.
     * A negative value indicates a turbo drive for which the exact value of
     * the acceleration factor has no meaning.
     */
    i16 speed;
    
    /* Mechanical delays
     * The start and stop delays specify the number of cycles that pass between
     * switching the drive motor on or off until the drive motor runs at full
     * speed or came to rest, respectively. The step delay specifies the number
     * of cycle needed by the drive head to move to another cylinder. During
     * this time, the FIFO is filled with garbage data.
     */
    Cycle startDelay;
    Cycle stopDelay;
    Cycle stepDelay;
}
DriveConfig;

inline bool isValidDriveSpeed(i16 speed)
{
    switch (speed) {
        case -1: case 1: case 2: case 4: case 8: return true;
    }
    return false;
}

typedef struct
{
    DriveHead head;
    bool hasDisk;
    bool motor;
}
DriveInfo;

#endif
