// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"
#include "FloppyDiskTypes.h"
#include "BootBlockImageTypes.h"

//
// Enumerations
//

enum_long(DRIVE_TYPE)
{
    DRIVE_DD_35,
    DRIVE_HD_35,
    DRIVE_DD_525
};
typedef DRIVE_TYPE FloppyDriveType;

#ifdef __cplusplus
struct FloppyDriveTypeEnum : vamiga::util::Reflection<FloppyDriveTypeEnum, FloppyDriveType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DRIVE_DD_525;

    static const char *prefix() { return "DRIVE"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case DRIVE_DD_35:   return "DD_35";
            case DRIVE_HD_35:   return "HD_35";
            case DRIVE_DD_525:  return "DD_525";
        }
        return "???";
    }
};
#endif

enum_long(DRIVE_MECHANICS)
{
    MECHANICS_NONE,
    MECHANICS_A1010
};
typedef DRIVE_MECHANICS DriveMechanics;

#ifdef __cplusplus
struct DriveMechanicsEnum : vamiga::util::Reflection<DriveMechanicsEnum, DriveMechanics>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = MECHANICS_A1010;

    static const char *prefix() { return "DMECHANICS"; }
    static const char *_key(long value)
    {
        switch (value) {

            case MECHANICS_NONE:    return "NONE";
            case MECHANICS_A1010:   return "A1010";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    // Connection status
    bool connected;
    
    // Drive model
    FloppyDriveType type;

    // Drive mechanics
    DriveMechanics mechanics;

    /* Revolutions per minute. A standard Amiga drive rotates with 300 rpm.
     * Rotation speed can be measured with AmigaTestKit which analyzes the
     * delay between consecutive index pulses. 300 rpm corresponds to an index
     * pulse delay of 200 ms.
     */
    isize rpm;

    // Delay between ejecting an old disk and inserting a new one
    Cycle diskSwapDelay;
    
    // Noise settings
    i16 pan;
    u8 stepVolume;
    u8 pollVolume;
    u8 insertVolume;
    u8 ejectVolume;
}
FloppyDriveConfig;

typedef struct
{
    isize nr;
    DriveHead head;
    bool isConnected;
    bool hasDisk;
    bool hasModifiedDisk;
    bool hasUnmodifiedDisk;
    bool hasProtectedDisk;
    bool hasUnprotectedDisk;
    bool motor;
    bool writing;
}
FloppyDriveInfo;

