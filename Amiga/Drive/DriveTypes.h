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
    DRIVE_525_SD
}
DriveType;

inline bool isDriveType(DriveType model)
{
    return model >= DRIVE_35_DD && model <= DRIVE_525_SD;
}

inline const char *driveTypeName(DriveType type)
{
    return
    type == DRIVE_35_DD  ? "3.5 DD" :
    type == DRIVE_525_SD ? "5.25 SD" : "???";
}

typedef enum : uint32_t
{
    DRIVE_ID_NONE  = 0x00000000,
    DRIVE_ID_35DD  = 0xFFFFFFFF,
    DRIVE_ID_35HD  = 0xAAAAAAAA,
    DRIVE_ID_525SD = 0x55555555
}
DriveIdCode;

typedef enum
{
    DRIVE_DMA_OFF = 0,   // Drive is idle
    DRIVE_DMA_SYNC_WAIT, // Drive is waiting for the sync word
    DRIVE_DMA_READ,      // Drive is reading (via DMA)
    DRIVE_DMA_WRITE      // Drive is writing (via DMA)
}
DriveState;


#endif
