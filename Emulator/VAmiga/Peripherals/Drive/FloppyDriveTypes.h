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

namespace vamiga {

//
// Enumerations
//

enum class FloppyDriveType : long
{
    DD_35,
    HD_35,
    DD_525
};

struct FloppyDriveTypeEnum : Reflection<FloppyDriveTypeEnum, FloppyDriveType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FloppyDriveType::DD_525);
    
    static const char *_key(FloppyDriveType value)
    {
        switch (value) {
                
            case FloppyDriveType::DD_35:   return "DD_35";
            case FloppyDriveType::HD_35:   return "HD_35";
            case FloppyDriveType::DD_525:  return "DD_525";
        }
        return "???";
    }
    static const char *help(FloppyDriveType value)
    {
        return "";
    }
};

enum class DriveMechanics
{
    NONE,
    A1010
};

struct DriveMechanicsEnum : Reflection<DriveMechanicsEnum, DriveMechanics>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DriveMechanics::A1010);
    
    static const char *_key(DriveMechanics value)
    {
        switch (value) {
                
            case DriveMechanics::NONE:    return "NONE";
            case DriveMechanics::A1010:   return "A1010";
        }
        return "???";
    }
    static const char *help(DriveMechanics value)
    {
        switch (value) {
                
            case DriveMechanics::NONE:    return "No mechanical delays";
            case DriveMechanics::A1010:   return "Commodore Floppy drive";
        }
        return "???";
    }
};


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

}
