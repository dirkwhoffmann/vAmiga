// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_TYPES_H
#define _FS_TYPES_H

#include "Aliases.h"

VAMIGA_ENUM(long, FSVolumeType)
{
    FS_NONE = -1,
    FS_OFS = 0,         // Original File System
    FS_FFS = 1,         // Fast File System
    FS_OFS_INTL = 2,    // "International" (not supported)
    FS_FFS_INTL = 3,    // "International" (not supported)
    FS_OFS_DC = 4,      // "Directory Cache" (not supported)
    FS_FFS_DC = 5,      // "Directory Cache" (not supported)
    FS_OFS_LNFS = 6,    // "Long Filenames" (not supported)
    FS_FFS_LNFS = 7     // "Long Filenames" (not supported)
};

inline bool isFSVolumeType(long value)
{
    return value >= FS_NONE && value <= FS_FFS_LNFS;
}

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

inline const char *sFSVolumeType(FSVolumeType value)
{
    switch (value) {
        case FS_NONE:     return "None";
        case FS_OFS:      return "OFS";
        case FS_FFS:      return "FFS";
        case FS_OFS_INTL: return "OFS_INTL";
        case FS_FFS_INTL: return "FFS_INTL";
        case FS_OFS_DC:   return "OFS_DC";
        case FS_FFS_DC:   return "FFS_DC";
        case FS_OFS_LNFS: return "OFS_LNFS";
        case FS_FFS_LNFS: return "FFS_LNFS";
        default:          return "???";
    }
}

VAMIGA_ENUM(long, FSBootCode)
{
    FS_BOOTBLOCK_NONE,
    FS_BOOTBLOCK_KICK_1_3,  // Standard Kickstart 1.3 boot code
    FS_BOOTBLOCK_KICK_2_0   // Standard Kickstart 2.0 boot code
};

VAMIGA_ENUM(long, FSBlockType)
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
    FS_DATA_BLOCK_FFS
};

inline bool
isFSBlockType(long value)
{
    return value >= FS_UNKNOWN_BLOCK && value <= FS_DATA_BLOCK_FFS;
}

inline const char *
sFSBlockType(FSBlockType type)
{
    assert(isFSBlockType(type));

    switch (type) {
        case FS_EMPTY_BLOCK:      return "FS_EMPTY_BLOCK";
        case FS_BOOT_BLOCK:       return "FS_BOOT_BLOCK";
        case FS_ROOT_BLOCK:       return "FS_ROOT_BLOCK";
        case FS_BITMAP_BLOCK:     return "FS_BITMAP_BLOCK";
        case FS_BITMAP_EXT_BLOCK: return "FS_BITMAP_EXT_BLOCK";
        case FS_USERDIR_BLOCK:    return "FS_USERDIR_BLOCK";
        case FS_FILEHEADER_BLOCK: return "FS_FILEHEADER_BLOCK";
        case FS_FILELIST_BLOCK:   return "FS_FILELIST_BLOCK";
        case FS_DATA_BLOCK_OFS:   return "FS_DATA_BLOCK_OFS";
        case FS_DATA_BLOCK_FFS:   return "FS_DATA_BLOCK_FFS";
        default:                  return "???";
    }
}

VAMIGA_ENUM(long, FSItemType)
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
};

inline bool
isFSBlockItem(long value)
{
    return value >= 0 && value <= FSI_BITMAP;
}

VAMIGA_ENUM(long, FSError)
{
    FS_OK,
    
    // File system errors
    FS_UNKNOWN,
    FS_UNSUPPORTED,
    FS_WRONG_BSIZE,
    FS_WRONG_CAPACITY,
    FS_HAS_CYCLES,
    FS_CORRUPTED,

    // Export errors
    FS_DIRECTORY_NOT_EMPTY,
    FS_CANNOT_CREATE_DIR,
    FS_CANNOT_CREATE_FILE,

    // Block errros
    FS_EXPECTED_VALUE,
    FS_EXPECTED_SMALLER_VALUE,
    FS_EXPECTED_DOS_REVISION,
    FS_EXPECTED_NO_REF,
    FS_EXPECTED_REF,
    FS_EXPECTED_SELFREF,
    FS_PTR_TO_UNKNOWN_BLOCK,
    FS_PTR_TO_EMPTY_BLOCK,
    FS_PTR_TO_BOOT_BLOCK,
    FS_PTR_TO_ROOT_BLOCK,
    FS_PTR_TO_BITMAP_BLOCK,
    FS_PTR_TO_BITMAP_EXT_BLOCK,
    FS_PTR_TO_USERDIR_BLOCK,
    FS_PTR_TO_FILEHEADER_BLOCK,
    FS_PTR_TO_FILELIST_BLOCK,
    FS_PTR_TO_DATA_BLOCK,
    FS_EXPECTED_DATABLOCK_NR,
    FS_INVALID_HASHTABLE_SIZE,
};

inline bool isFSError(FSError value)
{
    return value >= FS_OK && value <= FS_CORRUPTED;
}

inline const char *sFSError(FSError value)
{
    switch (value) {
            
        case FS_OK:                      return "FS_OK";
        case FS_UNKNOWN:                 return "FS_UNKNOWN";
        case FS_UNSUPPORTED:             return "FS_UNSUPPORTED";
        case FS_WRONG_BSIZE:             return "FS_WRONG_BSIZE";
        case FS_WRONG_CAPACITY:          return "FS_WRONG_CAPACITY";
        case FS_HAS_CYCLES:              return "FS_HAS_CYCLES";
        case FS_CORRUPTED:               return "FS_CORRUPTED";
            
        case FS_DIRECTORY_NOT_EMPTY:     return "FS_DIRECTORY_NOT_EMPTY";
        case FS_CANNOT_CREATE_DIR:       return "FS_CANNOT_CREATE_DIR";
        case FS_CANNOT_CREATE_FILE:      return "FS_CANNOT_CREATE_FILE";

        case FS_EXPECTED_VALUE:          return "FS_EXPECTED_VALUE";
        case FS_EXPECTED_SMALLER_VALUE:  return "FS_EXPECTED_SMALLER_VALUE";
        case FS_EXPECTED_DOS_REVISION:   return "FS_EXPECTED_DOS_REVISION";
        case FS_EXPECTED_NO_REF:         return "FS_EXPECTED_NO_REF";
        case FS_EXPECTED_REF:            return "FS_EXPECTED_REF";
        case FS_EXPECTED_SELFREF:        return "FS_EXPECTED_SELFREF";
        case FS_PTR_TO_UNKNOWN_BLOCK:    return "FS_PTR_TO_UNKNOWN_BLOCK";
        case FS_PTR_TO_EMPTY_BLOCK:      return "FS_PTR_TO_EMPTY_BLOCK";
        case FS_PTR_TO_BOOT_BLOCK:       return "FS_PTR_TO_BOOT_BLOCK";
        case FS_PTR_TO_ROOT_BLOCK:       return "FS_PTR_TO_ROOT_BLOCK";
        case FS_PTR_TO_BITMAP_BLOCK:     return "FS_PTR_TO_BITMAP_BLOCK";
        case FS_PTR_TO_BITMAP_EXT_BLOCK: return "FS_PTR_TO_BITMAP_EXT_BLOCK";
        case FS_PTR_TO_USERDIR_BLOCK:    return "FS_PTR_TO_USERDIR_BLOCK";
        case FS_PTR_TO_FILEHEADER_BLOCK: return "FS_PTR_TO_FILEHEADER_BLOCK";
        case FS_PTR_TO_FILELIST_BLOCK:   return "FS_PTR_TO_FILELIST_BLOCK";
        case FS_PTR_TO_DATA_BLOCK:       return "FS_PTR_TO_DATA_BLOCK";
        case FS_EXPECTED_DATABLOCK_NR:   return "FS_EXPECTED_DATABLOCK_NR";
        case FS_INVALID_HASHTABLE_SIZE:  return "FS_INVALID_HASHTABLE_SIZE";
        
        default:
            return isFSError(value) ? "<other>" : "???";
    }
}

typedef struct
{
    long bitmapErrors;
    long corruptedBlocks;
    long firstErrorBlock;
    long lastErrorBlock;
}
FSErrorReport;

#endif
