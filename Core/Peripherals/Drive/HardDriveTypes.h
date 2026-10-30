// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "DriveTypes.h"
#include "FileSystems/Amiga/FSTypes.h"
#include "DeviceDescriptors.h"

namespace vamiga {

//
// Enumerations
//

/// Type of a hard drive.
enum class HardDriveType : long
{
    GENERIC
};

struct HardDriveTypeEnum : utl::Reflectable<HardDriveTypeEnum, HardDriveType>
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
/// When a hard drive writes its data through to the image file.
enum class WriteThroughMode : long
{
    NEVER,      // The changes stay in memory
    ON_IDLE,    // They are written when the drive goes idle
    ALWAYS      // They are written straight away
};

struct WriteThroughModeEnum : utl::Reflectable<WriteThroughModeEnum, WriteThroughMode>
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

/// State of a hard drive.
enum class HardDriveState : long
{
    IDLE,
    READING,
    WRITING
};

struct HardDriveStateEnum : utl::Reflectable<HardDriveStateEnum, HardDriveState>
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

/// Configuration options of a hard drive.
typedef struct
{
    HardDriveType type;                     ///< Drive model
    i16 pan;                                ///< Stereo position of the drive sounds
    u8 stepVolume;                          ///< Head step volume

    // Decides when a change reaches the file (see HardDrive::persist)
    WriteThroughMode writeThrough;

    /* Indicates whether a disk is included in snapshots (see
     * HardDrive::serializeDisk). A disk that lives in a file is stored as its
     * path whatever this says -- the option is about a disk held in memory,
     * which has nowhere else to go. It cannot be enabled for a disk larger
     * than hdrSnapshotLimit.
     */
    bool snapshot;

    /* Largest disk that may be kept in memory, in MB (1...16384)
     *
     * A disk that does not live in a file is part of every snapshot, in full.
     * Beyond this size that is no longer practical, so such a disk has to
     * stay in its file (see HardDrive::loadIntoMemory).
     */
    isize memLimit;
}
HardDriveConfig;

// Largest disk a snapshot can carry, in MB
constexpr isize hdrSnapshotLimit = 128;

/// Static properties of a hard drive.
typedef struct
{
    // Object information
    isize nr;
    
    // Product information
    const char *diskVendor;
    const char *diskProduct;                ///< Product name of the disk
    const char *diskRevision;               ///< Revision of the disk
    const char *controllerVendor;           ///< Vendor of the controller
    const char *controllerProduct;          ///< Product name of the controller
    const char *controllerRevision;         ///< Revision of the controller
    
    // Physical layout
    isize cylinders;
    isize heads;                            ///< Number of heads
    isize sectors;                          ///< Number of sectors per track
    isize bsize;                            ///< Block size (in bytes)
    
    // Derived values
    isize tracks;
    i64 blocks;                             ///< Number of blocks
    i64 bytes;                              ///< Capacity (in bytes)
    isize upperCyl;                         ///< Highest cylinder
    isize upperHead;                        ///< Highest head
    isize upperTrack;                       ///< Highest track
}
HardDriveTraits;

/// Static properties of a hard drive partition.
typedef struct
{
    isize nr;                               ///< Number of the partition
    string name;                            ///< Name of the partition
    isize lowerCyl;                         ///< First cylinder
    isize upperCyl;                         ///< Last cylinder
    retro::vault::amiga::FSFormat fsType;                 ///< File system type
}
PartitionTraits;

/// State of a hard drive.
typedef struct
{
    isize nr;                               ///< Number of the drive
    
    // Drive properties
    bool isConnected;
    bool isCompatible;                      ///< The drive is compatible with the emulated controller
    
    // Disk properties
    bool hasDisk;
    bool hasModifiedDisk;                   ///< The disk has unsaved changes
    bool hasUnmodifiedDisk;                 ///< The disk has no unsaved changes
    bool hasProtectedDisk;                  ///< The disk is write-protected
    bool hasUnprotectedDisk;                ///< The disk is not write-protected
    bool snapshotable;                      ///< The disk can be included in snapshots

    // Logical layout (partitions)
    isize partitions;
    
    // Flags
    bool writeProtected;

    /* The guest has written to the disk since it was attached, or since it was
     * last marked as unmodified (e.g. by exporting it). What an unsaved-changes
     * warning wants to know; it also applies to a disk that has no file.
     * Persisting does not clear it.
     */
    bool modified;

    /* The disk holds changes its file does not have yet, i.e. persist() has
     * something to do. Only a disk that lives in a file can be dirty, and
     * persisting clears it. Changes that do not come from the guest (formatting,
     * importing a folder) make a disk dirty without making it modified.
     */
    bool dirty;
    
    // State
    HardDriveState state;
    DriveHead head;                         ///< Position of the drive head
}
HardDriveInfo;

}
