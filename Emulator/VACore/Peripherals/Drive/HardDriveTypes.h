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
#include "DriveDescriptors.h"

namespace vamiga {

//
// Enumerations
//

enum class HardDriveType : long
{
    GENERIC
};

struct HardDriveTypeEnum : Reflection<HardDriveTypeEnum, HardDriveType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(HardDriveType::GENERIC);
    
    static const char *_key(HardDriveType value)
    {
        switch (value) {
                
            case HardDriveType::GENERIC:   return "GENERIC";
        }
        return "???";
    }
    static const char *help(HardDriveType value)
    {
        return "";
    }
};

enum class HardDriveState : long
{
    IDLE,
    READING,
    WRITING
};

struct HardDriveStateEnum : Reflection<HardDriveStateEnum, HardDriveState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(HardDriveState::WRITING);
    
    static const char *_key(HardDriveState value)
    {
        switch (value) {
                
            case HardDriveState::IDLE:      return "IDLE";
            case HardDriveState::READING:   return "READING";
            case HardDriveState::WRITING:   return "WRITING";
        }
        return "???";
    }
    static const char *help(HardDriveState value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    HardDriveType type;
    i16 pan;
    u8 stepVolume;
}
HardDriveConfig;

typedef struct
{
    // Object information
    isize nr;
    
    // Product information
    const char *diskVendor;
    const char *diskProduct;
    const char *diskRevision;
    const char *controllerVendor;
    const char *controllerProduct;
    const char *controllerRevision;
    
    // Physical layout
    isize cylinders;
    isize heads;
    isize sectors;
    isize bsize;
    
    // Derived values
    isize tracks;
    isize blocks;
    isize bytes;
    isize upperCyl;
    isize upperHead;
    isize upperTrack;
}
HardDriveTraits;

typedef struct
{
    isize nr;
    string name;
    isize lowerCyl;
    isize upperCyl;
    FSVolumeType fsType;
}
PartitionTraits;

typedef struct
{
    isize nr;
    
    // Drive properties
    bool isConnected;
    bool isCompatible;
    
    // Disk properties
    bool hasDisk;
    bool hasModifiedDisk;
    bool hasUnmodifiedDisk;
    bool hasProtectedDisk;
    bool hasUnprotectedDisk;
    
    // Logical layout (partitions)
    isize partitions;
    
    // Flags
    bool writeProtected;
    bool modified;
    
    // State
    HardDriveState state;
    DriveHead head;
}
HardDriveInfo;

}
