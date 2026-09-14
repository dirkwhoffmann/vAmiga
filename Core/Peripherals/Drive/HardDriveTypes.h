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
#include "FileSystems/Amiga/FSTypes.h"
#include "DeviceDescriptors.h"

namespace vamiga {

//
// Enumerations
//

enum class HardDriveType : long
{
    GENERIC
};

struct HardDriveTypeEnum : Reflectable<HardDriveTypeEnum, HardDriveType>
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

/* Decides when the changes made to a disk reach the file it lives in
 *
 * A disk that was built in memory has no file to go back to; for such a disk,
 * the setting has no effect. Writing is always performed by the main
 * instance, never by the run-ahead instance.
 */
enum class WriteThroughMode : long
{
    NEVER,      // The changes stay in memory
    ON_IDLE,    // They are written when the drive goes idle
    ALWAYS      // They are written straight away
};

struct WriteThroughModeEnum : Reflectable<WriteThroughModeEnum, WriteThroughMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(WriteThroughMode::ALWAYS);

    static const char *_key(WriteThroughMode value)
    {
        switch (value) {

            case WriteThroughMode::NEVER:     return "NEVER";
            case WriteThroughMode::ON_IDLE:   return "ON_IDLE";
            case WriteThroughMode::ALWAYS:    return "ALWAYS";
        }
        return "???";
    }
    static const char *help(WriteThroughMode value)
    {
        switch (value) {

            case WriteThroughMode::NEVER:     return "Keep all changes in memory";
            case WriteThroughMode::ON_IDLE:   return "Write back when the drive goes idle";
            case WriteThroughMode::ALWAYS:    return "Write back after each write access";
        }
        return "???";
    }
};

enum class HardDriveState : long
{
    IDLE,
    READING,
    WRITING
};

struct HardDriveStateEnum : Reflectable<HardDriveStateEnum, HardDriveState>
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

    // Decides when a change reaches the file (see HardDrive::persist)
    WriteThroughMode writeThrough;
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
    i64 blocks;
    i64 bytes;
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
    amiga::FSFormat fsType;
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
