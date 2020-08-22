// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DISK_TYPES_H
#define _DISK_TYPES_H

#include "VA_ENUM.h"

//
// Enumerations
//

typedef VA_ENUM(long, DiskType)
{
    DISK_35_DD,
    DISK_35_DD_PC,
    DISK_35_HD,
    DISK_35_HD_PC,
    DISK_525_SD
};

inline bool isDiskType(DiskType model)
{
    return model >= DISK_35_DD && model <= DISK_525_SD;
}

inline const char *diskTypeName(DiskType type)
{
    assert(isDiskType(type));
    
    switch (type) {
        case DISK_35_DD:    return "3.5\" DD";
        case DISK_35_DD_PC: return "3.5\" DD (PC)";
        case DISK_35_HD:    return "3.5\" HD";
        case DISK_35_HD_PC: return "3.5\" HD (PC)";
        case DISK_525_SD:   return "5.25\" SD";
        default:            return "???";
    }
}

typedef VA_ENUM(long, FileSystemType)
{
    FS_NONE,
    FS_OFS,
    FS_OFS_BOOTABLE,
    FS_FFS,
    FS_FFS_BOOTABLE
};

inline bool isFileSystemType(FileSystemType type)
{
    return type >= FS_NONE && type <= FS_FFS_BOOTABLE;
}

inline const char *fileSystemTypeName(FileSystemType type)
{
    switch (type) {
        case FS_NONE:         return "None";
        case FS_OFS:          return "OFS";
        case FS_OFS_BOOTABLE: return "OFS (bootable)";
        case FS_FFS:          return "FFS";
        case FS_FFS_BOOTABLE: return "FFS (bootable)";
        default:              return "???";
    }
}

#endif
