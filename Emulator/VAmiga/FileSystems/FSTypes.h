// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

enum class FSVolumeType : long
{
    OFS      = 0,    // Original File System
    FFS      = 1,    // Fast File System
    OFS_INTL = 2,    // "International" (not supported)
    FFS_INTL = 3,    // "International" (not supported)
    OFS_DC   = 4,    // "Directory Cache" (not supported)
    FFS_DC   = 5,    // "Directory Cache" (not supported)
    OFS_LNFS = 6,    // "Long Filenames" (not supported)
    FFS_LNFS = 7,    // "Long Filenames" (not supported)
    NODOS
};

struct FSVolumeTypeEnum : Reflection<FSVolumeTypeEnum, FSVolumeType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FSVolumeType::NODOS);
    
    static const char *_key(FSVolumeType value)
    {
        switch (value) {
                
            case FSVolumeType::OFS:       return "OFS";
            case FSVolumeType::FFS:       return "FFS";
            case FSVolumeType::OFS_INTL:  return "OFS_INTL";
            case FSVolumeType::FFS_INTL:  return "FFS_INTL";
            case FSVolumeType::OFS_DC:    return "OFS_DC";
            case FSVolumeType::FFS_DC:    return "FFS_DC";
            case FSVolumeType::OFS_LNFS:  return "OFS_LNFS";
            case FSVolumeType::FFS_LNFS:  return "FFS_LNFS";
            case FSVolumeType::NODOS:     return "NODOS";
        }
        return "???";
    }
    static const char *help(FSVolumeType value)
    {
        return "";
    }
};

inline bool isOFSVolumeType(FSVolumeType value)
{
    switch (value) {
            
        case FSVolumeType::OFS:
        case FSVolumeType::OFS_INTL:
        case FSVolumeType::OFS_DC:
        case FSVolumeType::OFS_LNFS:    return true;
        default:                        return false;
    }
}

inline bool isFFSVolumeType(FSVolumeType value)
{
    switch (value) {
            
        case FSVolumeType::FFS:
        case FSVolumeType::FFS_INTL:
        case FSVolumeType::FFS_DC:
        case FSVolumeType::FFS_LNFS: return true;
        default:                        return false;
    }
}

enum class FSBlockType : long
{
    UNKNOWN_BLOCK,
    EMPTY_BLOCK,
    BOOT_BLOCK,
    ROOT_BLOCK,
    BITMAP_BLOCK,
    BITMAP_EXT_BLOCK,
    USERDIR_BLOCK,
    FILEHEADER_BLOCK,
    FILELIST_BLOCK,
    DATA_BLOCK_OFS,
    DATA_BLOCK_FFS
};

struct FSBlockTypeEnum : Reflection<FSBlockTypeEnum, FSBlockType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FSBlockType::DATA_BLOCK_FFS);
    
    static const char *_key(FSBlockType value)
    {
        switch (value) {
                
            case FSBlockType::UNKNOWN_BLOCK:     return "UNKNOWN_BLOCK";
            case FSBlockType::EMPTY_BLOCK:       return "EMPTY_BLOCK";
            case FSBlockType::BOOT_BLOCK:        return "BOOT_BLOCK";
            case FSBlockType::ROOT_BLOCK:        return "ROOT_BLOCK";
            case FSBlockType::BITMAP_BLOCK:      return "BITMAP_BLOCK";
            case FSBlockType::BITMAP_EXT_BLOCK:  return "BITMAP_EXT_BLOCK";
            case FSBlockType::USERDIR_BLOCK:     return "USERDIR_BLOCK";
            case FSBlockType::FILEHEADER_BLOCK:  return "FILEHEADER_BLOCK";
            case FSBlockType::FILELIST_BLOCK:    return "FILELIST_BLOCK";
            case FSBlockType::DATA_BLOCK_OFS:    return "DATA_BLOCK_OFS";
            case FSBlockType::DATA_BLOCK_FFS:    return "DATA_BLOCK_FFS";
        }
        return "???";
    }
    static const char *help(FSBlockType value)
    {
        return "";
    }
};

enum class FSItemType
{
    UNKNOWN,
    UNUSED,
    DOS_HEADER,
    DOS_VERSION,
    BOOTCODE,
    TYPE_ID,
    SUBTYPE_ID,
    SELF_REF,
    CHECKSUM,
    HASHTABLE_SIZE,
    HASH_REF,
    PROT_BITS,
    BCPL_STRING_LENGTH,
    BCPL_DISK_NAME,
    BCPL_DIR_NAME,
    BCPL_FILE_NAME,
    BCPL_COMMENT,
    CREATED_DAY,
    CREATED_MIN,
    CREATED_TICKS,
    MODIFIED_DAY,
    MODIFIED_MIN,
    MODIFIED_TICKS,
    NEXT_HASH_REF,
    PARENT_DIR_REF,
    FILEHEADER_REF,
    EXT_BLOCK_REF,
    BITMAP_BLOCK_REF,
    BITMAP_EXT_BLOCK_REF,
    BITMAP_VALIDITY,
    FILESIZE,
    DATA_BLOCK_NUMBER,
    DATA_BLOCK_REF_COUNT,
    FIRST_DATA_BLOCK_REF,
    NEXT_DATA_BLOCK_REF,
    DATA_BLOCK_REF,
    DATA_COUNT,
    DATA,
    BITMAP
};

struct FSItemTypeEnum : Reflection<FSItemTypeEnum, FSItemType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FSItemType::BITMAP);
    
    static const char *_key(FSItemType value)
    {
        switch (value) {
                
            case FSItemType::UNKNOWN:               return "UNKNOWN";
            case FSItemType::UNUSED:                return "UNUSED";
            case FSItemType::DOS_HEADER:            return "DOS_HEADER";
            case FSItemType::DOS_VERSION:           return "DOS_VERSION";
            case FSItemType::BOOTCODE:              return "BOOTCODE";
            case FSItemType::TYPE_ID:               return "TYPE_ID";
            case FSItemType::SUBTYPE_ID:            return "SUBTYPE_ID";
            case FSItemType::SELF_REF:              return "SELF_REF";
            case FSItemType::CHECKSUM:              return "CHECKSUM";
            case FSItemType::HASHTABLE_SIZE:        return "HASHTABLE_SIZE";
            case FSItemType::HASH_REF:              return "HASH_REF";
            case FSItemType::PROT_BITS:             return "PROT_BITS";
            case FSItemType::BCPL_STRING_LENGTH:    return "BCPL_STRING_LENGTH";
            case FSItemType::BCPL_DISK_NAME:        return "BCPL_DISK_NAME";
            case FSItemType::BCPL_DIR_NAME:         return "BCPL_DIR_NAME";
            case FSItemType::BCPL_FILE_NAME:        return "BCPL_FILE_NAME";
            case FSItemType::BCPL_COMMENT:          return "BCPL_COMMENT";
            case FSItemType::CREATED_DAY:           return "CREATED_DAY";
            case FSItemType::CREATED_MIN:           return "CREATED_MIN";
            case FSItemType::CREATED_TICKS:         return "CREATED_TICKS";
            case FSItemType::MODIFIED_DAY:          return "MODIFIED_DAY";
            case FSItemType::MODIFIED_MIN:          return "MODIFIED_MIN";
            case FSItemType::MODIFIED_TICKS:        return "MODIFIED_TICKS";
            case FSItemType::NEXT_HASH_REF:         return "NEXT_HASH_REF";
            case FSItemType::PARENT_DIR_REF:        return "PARENT_DIR_REF";
            case FSItemType::FILEHEADER_REF:        return "FILEHEADER_REF";
            case FSItemType::EXT_BLOCK_REF:         return "EXT_BLOCK_REF";
            case FSItemType::BITMAP_BLOCK_REF:      return "BITMAP_BLOCK_REF";
            case FSItemType::BITMAP_EXT_BLOCK_REF:  return "BITMAP_EXT_BLOCK_REF";
            case FSItemType::BITMAP_VALIDITY:       return "BITMAP_VALIDITY";
            case FSItemType::FILESIZE:              return "FILESIZE";
            case FSItemType::DATA_BLOCK_NUMBER:     return "DATA_BLOCK_NUMBER";
            case FSItemType::DATA_BLOCK_REF_COUNT:  return "DATA_BLOCK_REF_COUNT";
            case FSItemType::FIRST_DATA_BLOCK_REF:  return "FIRST_DATA_BLOCK_REF";
            case FSItemType::NEXT_DATA_BLOCK_REF:   return "NEXT_DATA_BLOCK_REF";
            case FSItemType::DATA_BLOCK_REF:        return "DATA_BLOCK_REF";
            case FSItemType::DATA_COUNT:            return "DATA_COUNT";
            case FSItemType::DATA:                  return "DATA";
            case FSItemType::BITMAP:                return "BITMAP";
        }
        return "???";
    }
};


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

typedef struct
{
    FSVolumeType dos;
    bool ofs;
    bool ffs;
    
    isize blocks;
    isize bytes;
    isize bsize;
}
FSTraits;

}
