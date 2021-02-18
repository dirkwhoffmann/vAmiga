// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

typedef u32 Block;

enum_long(FS_VOLUME_TYPE)
{
    FS_NODOS = -1,
    FS_OFS = 0,         // Original File System
    FS_FFS = 1,         // Fast File System
    FS_OFS_INTL = 2,    // "International" (not supported)
    FS_FFS_INTL = 3,    // "International" (not supported)
    FS_OFS_DC = 4,      // "Directory Cache" (not supported)
    FS_FFS_DC = 5,      // "Directory Cache" (not supported)
    FS_OFS_LNFS = 6,    // "Long Filenames" (not supported)
    FS_FFS_LNFS = 7     // "Long Filenames" (not supported)
};
typedef FS_VOLUME_TYPE FSVolumeType;

inline bool isOFSVolumeType(long value)
{
    switch (value) {
        case FS_OFS:
        case FS_OFS_INTL:
        case FS_OFS_DC:
        case FS_OFS_LNFS: return true;
        default:          return false;
    }
}

inline bool isFFSVolumeType(long value)
{
    switch (value) {
        case FS_FFS:
        case FS_FFS_INTL:
        case FS_FFS_DC:
        case FS_FFS_LNFS: return true;
        default:          return false;
    }
}

enum_long(FS_BLOCK_TYPE)
{
    FS_UNKNOWN_BLOCK,
    FS_EMPTY_BLOCK,
    FS_BOOT_BLOCK,
    FS_ROOT_BLOCK,
    FS_BITMAP_BLOCK,
    FS_BITMAP_EXT_BLOCK,
    FS_USERDIR_BLOCK,
    FS_FILEHEADER_BLOCK,
    FS_FILELIST_BLOCK,
    FS_DATA_BLOCK_OFS,
    FS_DATA_BLOCK_FFS,
    
    FS_BLOCK_TYPE_COUNT
};
typedef FS_BLOCK_TYPE FSBlockType;

enum_long(FSI_TYPE)
{
    FSI_UNKNOWN,
    FSI_UNUSED,
    FSI_DOS_HEADER,
    FSI_DOS_VERSION,
    FSI_BOOTCODE,
    FSI_TYPE_ID,
    FSI_SUBTYPE_ID,
    FSI_SELF_REF,
    FSI_CHECKSUM,
    FSI_HASHTABLE_SIZE,
    FSI_HASH_REF,
    FSI_PROT_BITS,
    FSI_BCPL_STRING_LENGTH,
    FSI_BCPL_DISK_NAME,
    FSI_BCPL_DIR_NAME,
    FSI_BCPL_FILE_NAME,
    FSI_BCPL_COMMENT,
    FSI_CREATED_DAY,
    FSI_CREATED_MIN,
    FSI_CREATED_TICKS,
    FSI_MODIFIED_DAY,
    FSI_MODIFIED_MIN,
    FSI_MODIFIED_TICKS,
    FSI_NEXT_HASH_REF,
    FSI_PARENT_DIR_REF,
    FSI_FILEHEADER_REF,
    FSI_EXT_BLOCK_REF,
    FSI_BITMAP_BLOCK_REF,
    FSI_BITMAP_EXT_BLOCK_REF,
    FSI_BITMAP_VALIDITY,
    FSI_FILESIZE,
    FSI_DATA_BLOCK_NUMBER,
    FSI_DATA_BLOCK_REF_COUNT,
    FSI_FIRST_DATA_BLOCK_REF,
    FSI_NEXT_DATA_BLOCK_REF,
    FSI_DATA_BLOCK_REF,
    FSI_DATA_COUNT,
    FSI_DATA,
    FSI_BITMAP,
    
    FSI_COUNT
};
typedef FSI_TYPE FSItemType;


//
// Structures
//

typedef struct
{
    long bitmapErrors;
    long corruptedBlocks;
    long firstErrorBlock;
    long lastErrorBlock;
}
FSErrorReport;
