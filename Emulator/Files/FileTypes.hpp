// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Disk.hpp"

namespace va {

#include "FileTypes.h"

struct FileTypeEnum : util::Reflection<FileTypeEnum, FileType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < FILETYPE_COUNT;
    }
    
    static const char *prefix() { return "FILETYPE"; }
    static const char *key(FileType value)
    {
        switch (value) {
                
            case FILETYPE_UKNOWN:       return "UKNOWN";
            case FILETYPE_SNAPSHOT:     return "SNAPSHOT";
            case FILETYPE_ADF:          return "ADF";
            case FILETYPE_HDF:          return "HDF";
            case FILETYPE_EXT:          return "EXT";
            case FILETYPE_IMG:          return "IMG";
            case FILETYPE_DMS:          return "DMS";
            case FILETYPE_EXE:          return "EXE";
            case FILETYPE_DIR:          return "DIR";
            case FILETYPE_ROM:          return "ROM";
            case FILETYPE_EXTENDED_ROM: return "EXTENDED_ROM";
            case FILETYPE_COUNT:        return "???";
        }
        return "???";
    }
};

}
