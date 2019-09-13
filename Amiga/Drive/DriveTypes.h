// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DRIVE_T_INC
#define _DRIVE_T_INC

typedef enum : long
{
    DRIVE_35_DD,
    DRIVE_35_DD_PC,
    DRIVE_35_HD,
    DRIVE_35_HD_PC,
    DRIVE_525_SD
}
DriveType;

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

typedef enum
{
    DRIVE_DMA_OFF,     // Drive is idle
    
    DRIVE_DMA_WAIT,    // Drive is waiting for the sync word
    DRIVE_DMA_READ,    // Drive is reading
    
    DRIVE_DMA_WRITE,   // Drive is writing
    DRIVE_DMA_FLUSH,   // Drive is finishing up the write process
}
DriveState;

inline bool isDriveState(long value)
{
    return value >= DRIVE_DMA_OFF && value <= DRIVE_DMA_FLUSH;
}

inline const char *driveStateName(DriveState state)
{
    assert(isDriveState(state));

    switch (state) {
        case DRIVE_DMA_OFF:   return "DRIVE_DMA_OFF";
        case DRIVE_DMA_WAIT:  return "DRIVE_DMA_WAIT";
        case DRIVE_DMA_READ:  return "DRIVE_DMA_READ";
        case DRIVE_DMA_WRITE: return "DRIVE_DMA_WRITE";
        case DRIVE_DMA_FLUSH: return "DRIVE_DMA_FLUSH";
        default:              return "???";
    }
}

typedef struct
{
    /* Drive type.
     * At the moment, we only support standard 3.5" DD drives.
     */
     DriveType type;

    /* Acceleration factor.
     * This value equals the number of words that get transfered into memory
     * during a single disk DMA cycle. This value must be 1 to emulate a real
     * Amiga. If it set to, e.g., 2, the drive loads twice as fast.
     * A negative value indicates a turbo drive for which the exact value of
     * the acceleration factor has no meaning.
     */
    int16_t speed;
}
DriveConfig;

#endif
