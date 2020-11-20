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

VAMIGA_ENUM(long, FSBlockType)
{
    FS_UNKNOWN_BLOCK,
    FS_EMPTY_BLOCK,
    FS_BOOT_BLOCK,
    FS_ROOT_BLOCK,
    FS_BITMAP_BLOCK,
    FS_USERDIR_BLOCK,
    FS_FILEHEADER_BLOCK,
    FS_FILELIST_BLOCK,
    FS_DATA_BLOCK
};

inline bool
isFSBlockType(long value)
{
    return value >= FS_UNKNOWN_BLOCK && value <= FS_DATA_BLOCK;
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
        case FS_USERDIR_BLOCK:    return "FS_USERDIR_BLOCK";
        case FS_FILEHEADER_BLOCK: return "FS_FILEHEADER_BLOCK";
        case FS_FILELIST_BLOCK:   return "FS_FILELIST_BLOCK";
        case FS_DATA_BLOCK:       return "FS_DATA_BLOCK";
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
    FS_CORRUPTED,
    
    // Block errros
    FS_EXPECTED_D,
    FS_EXPECTED_O,
    FS_EXPECTED_S,
    FS_EXPECTED_00,
    FS_EXPECTED_01,
    FS_EXPECTED_02,
    FS_EXPECTED_03,
    FS_EXPECTED_08,
    FS_EXPECTED_10,
    FS_EXPECTED_FD,
    FS_EXPECTED_FF,
    FS_EXPECTED_DOS_REVISION,
    FS_EXPECTED_NO_REF,
    FS_EXPECTED_REF,
    FS_EXPECTED_SELFREF,
    FS_EXPECTED_FILEHEADER_REF,
    FS_EXPECTED_FILELIST_REF,
    FS_EXPECTED_DATABLOCK_REF,
    FS_EXPECTED_HASH_REF,
    FS_EXPECTED_PARENTDIR_REF,
    FS_EXPECTED_DATABLOCK_NR,
    FS_INVALID_HASHTABLE_SIZE,
    FS_INVALID_CHECKSUM,
    FS_OUT_OF_RANGE,
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
        case FS_CORRUPTED:               return "FS_CORRUPTED";
        case FS_EXPECTED_D:              return "FS_EXPECTED_D";
        case FS_EXPECTED_O:              return "FS_EXPECTED_O";
        case FS_EXPECTED_S:              return "FS_EXPECTED_S";
        case FS_EXPECTED_00:             return "FS_EXPECTED_00";
        case FS_EXPECTED_01:             return "FS_EXPECTED_01";
        case FS_EXPECTED_02:             return "FS_EXPECTED_02";
        case FS_EXPECTED_03:             return "FS_EXPECTED_03";
        case FS_EXPECTED_08:             return "FS_EXPECTED_08";
        case FS_EXPECTED_10:             return "FS_EXPECTED_10";
        case FS_EXPECTED_FD:             return "FS_EXPECTED_FD";
        case FS_EXPECTED_FF:             return "FS_EXPECTED_FF";
        case FS_EXPECTED_DOS_REVISION:   return "FS_EXPECTED_DOS_REVISION";
        case FS_EXPECTED_NO_REF:         return "FS_EXPECTED_NO_REF";
        case FS_EXPECTED_REF:            return "FS_EXPECTED_REF";
        case FS_EXPECTED_SELFREF:        return "FS_EXPECTED_SELFREF";
        case FS_EXPECTED_FILEHEADER_REF: return "FS_EXPECTED_FILEHEADER_REF";
        case FS_EXPECTED_FILELIST_REF:   return "FS_EXPECTED_FILELIST_REF";
        case FS_EXPECTED_DATABLOCK_REF:  return "FS_EXPECTED_DATABLOCK_REF";
        case FS_EXPECTED_HASH_REF:       return "FS_EXPECTED_HASH_REF";
        case FS_EXPECTED_PARENTDIR_REF:  return "FS_EXPECTED_PARENTDIR_REF";
        case FS_EXPECTED_DATABLOCK_NR:   return "FS_EXPECTED_DATABLOCK_NR";
        case FS_INVALID_HASHTABLE_SIZE:  return "FS_INVALID_HASHTABLE_SIZE";
        case FS_INVALID_CHECKSUM:        return "FS_INVALID_CHECKSUM";
        case FS_OUT_OF_RANGE:            return "FS_OUT_OF_RANGE";
        
        default:
            return isFSError(value) ? "<other>" : "???";
    }
}

typedef struct
{
    long corruptedBlocks;
    long firstErrorBlock;
    long lastErrorBlock;
}
FSErrorReport;

#endif
