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

#ifdef __cplusplus
#include "DriveDescriptors.h"
#include <vector>
#endif

//
// Enumerations
//

enum_long(HDR_TYPE)
{
    HDR_GENERIC
};
typedef HDR_TYPE HardDriveType;

#ifdef __cplusplus
struct HardDriveTypeEnum : util::Reflection<HardDriveTypeEnum, HardDriveType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDR_GENERIC;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "HDR"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case HDR_GENERIC:   return "GENERIC";
        }
        return "???";
    }
};
#endif

enum_long(HDR_STATE)
{
    HDR_STATE_IDLE,
    HDR_STATE_READING,
    HDR_STATE_WRITING
};
typedef HDR_STATE HardDriveState;

#ifdef __cplusplus
struct HardDriveStateEnum : util::Reflection<HardDriveStateEnum, HardDriveState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDR_STATE_WRITING;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "HDR_STATE"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case HDR_STATE_IDLE:      return "IDLE";
            case HDR_STATE_READING:   return "READING";
            case HDR_STATE_WRITING:   return "WRITING";
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
    const char *name;
    isize lowerCyl;
    isize upperCyl;
}
PartitionTraits;

typedef struct
{
    // Drive properties
    bool isConnected;
    bool isCompatible;
    bool writeThrough;
    
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
