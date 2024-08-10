// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(FILETYPE)
{
    FILETYPE_UNKNOWN,
    FILETYPE_SNAPSHOT,
    FILETYPE_SCRIPT,
    FILETYPE_ADF,
    FILETYPE_EADF,
    FILETYPE_HDF,
    FILETYPE_IMG,
    FILETYPE_ST,
    FILETYPE_DMS,
    FILETYPE_EXE,
    FILETYPE_DIR,
    FILETYPE_ROM,
    FILETYPE_EXTENDED_ROM
};
typedef FILETYPE FileType;

#ifdef __cplusplus
struct FileTypeEnum : vamiga::util::Reflection<FileTypeEnum, FileType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILETYPE_EXTENDED_ROM;

    static const char *prefix() { return "FILETYPE"; }
    static const char *_key(long value)
    {
        switch (value) {

            case FILETYPE_UNKNOWN:      return "UNKNOWN";
            case FILETYPE_SNAPSHOT:     return "SNAPSHOT";
            case FILETYPE_SCRIPT:       return "SCRIPT";
            case FILETYPE_ADF:          return "ADF";
            case FILETYPE_EADF:         return "EADF";
            case FILETYPE_HDF:          return "HDF";
            case FILETYPE_IMG:          return "IMG";
            case FILETYPE_ST:           return "ST";
            case FILETYPE_DMS:          return "DMS";
            case FILETYPE_EXE:          return "EXE";
            case FILETYPE_DIR:          return "DIR";
            case FILETYPE_ROM:          return "ROM";
            case FILETYPE_EXTENDED_ROM: return "EXTENDED_ROM";
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
    isize cyls;
    isize heads;
    isize sectors;
    isize bsize;
    isize tracks;
    isize blocks;
    isize bytes;
}
DiskInfo;

typedef struct
{
    isize partitions;
    isize drivers;
    bool hasRDB;
}
HDFInfo;
