// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DISK_T_INC
#define _DISK_T_INC

//
// Disk type (format and density)
//

typedef enum : long
{
    DISK_35_DD,
    DISK_35_DD_PC,
    DISK_35_HD,
    DISK_35_HD_PC,
    DISK_525_SD
}
DiskType;

inline bool isDiskType(DiskType model)
{
    return model >= DISK_35_DD && model <= DISK_525_SD;
}

inline const char *diskTypeName(DiskType type)
{
    return
    type == DISK_35_DD  ? "3.5 DD" :
    type == DISK_525_SD ? "5.25 SD" : "???";
}


//
// File system type (Original File System or Fast File System)
//

typedef enum : long
{
    AMIGA_OFS,
    AMIGA_FFS
}
FileSystemType;

inline bool isFileSystemType(FileSystemType type)
{
    return type >= AMIGA_OFS && type <= AMIGA_FFS;
}

inline const char *fileSystemTypeName(FileSystemType type)
{
    return
    type == AMIGA_OFS ? "OFS" :
    type == AMIGA_FFS ? "FFS" : "???";
}

#endif
