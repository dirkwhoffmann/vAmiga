// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
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
    FILETYPE_HDF,
    FILETYPE_EXT,
    FILETYPE_IMG,
    FILETYPE_DMS,
    FILETYPE_EXE,
    FILETYPE_DIR,
    FILETYPE_ROM,
    FILETYPE_EXTENDED_ROM    
};
typedef FILETYPE FileType;

#ifdef __cplusplus
struct FileTypeEnum : util::Reflection<FileTypeEnum, FileType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FILETYPE_EXTENDED_ROM;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "FILETYPE"; }
    static const char *key(FileType value)
    {
        switch (value) {
                
            case FILETYPE_UNKNOWN:      return "UNKNOWN";
            case FILETYPE_SNAPSHOT:     return "SNAPSHOT";
            case FILETYPE_SCRIPT:       return "SCRIPT";
            case FILETYPE_ADF:          return "ADF";
            case FILETYPE_HDF:          return "HDF";
            case FILETYPE_EXT:          return "EXT";
            case FILETYPE_IMG:          return "IMG";
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
