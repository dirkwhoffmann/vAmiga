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

#include "Aliases.h"

//
// Enumerations
//

typedef VA_ENUM(long, DiskType)
{
    DISK_35_DD,
    DISK_35_DD_PC,
    DISK_35_HD,
    DISK_35_HD_PC,
    DISK_525_DD,
    DISK_525_DD_PC
};

inline bool isDiskType(DiskType type)
{
    return type >= DISK_35_DD && type <= DISK_525_DD_PC;
}

inline bool isAmigaDiskType(DiskType type)
{
    return type == DISK_35_DD || type == DISK_35_HD || type == DISK_525_DD;
}

inline const char *diskTypeName(DiskType type)
{
    assert(isDiskType(type));
    
    switch (type) {
        case DISK_35_DD:     return "3.5\" DD";
        case DISK_35_DD_PC:  return "3.5\" DD (PC)";
        case DISK_35_HD:     return "3.5\" HD";
        case DISK_35_HD_PC:  return "3.5\" HD (PC)";
        case DISK_525_DD:    return "5.25\" DD";
        case DISK_525_DD_PC: return "5.25\" DD (PC)";
        default:             return "???";
    }
}

typedef VA_ENUM(long, EmptyDiskFormat)
{
    FS_EMPTY,
    FS_EMPTY_OFS,
    FS_EMPTY_OFS_BOOTABLE,
    FS_EMPTY_FFS,
    FS_EMPTY_FFS_BOOTABLE
};

inline bool isEmptyDiskFormat(EmptyDiskFormat type)
{
    return type >= FS_EMPTY && type <= FS_EMPTY_FFS_BOOTABLE;
}

inline const char *emptyDiskFormatName(EmptyDiskFormat type)
{
    switch (type) {
        case FS_EMPTY:              return "None";
        case FS_EMPTY_OFS:          return "OFS";
        case FS_EMPTY_OFS_BOOTABLE: return "OFS (bootable)";
        case FS_EMPTY_FFS:          return "FFS";
        case FS_EMPTY_FFS_BOOTABLE: return "FFS (bootable)";
        default:                    return "???";
    }
}

#endif
