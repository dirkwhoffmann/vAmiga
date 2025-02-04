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

namespace vamiga {

//
// Enumerations
//

enum class FileType : long
{
    UNKNOWN,
    SNAPSHOT,
    SCRIPT,
    ADF,
    EADF,
    HDF,
    IMG,
    ST,
    DMS,
    EXE,
    DIR,
    ROM,
    EXTENDED_ROM
};

struct FileTypeEnum : util::Reflection<FileTypeEnum, FileType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(FileType::EXTENDED_ROM);
    
    static const char *_key(FileType value)
    {
        switch (value) {
                
            case FileType::UNKNOWN:      return "UNKNOWN";
            case FileType::SNAPSHOT:     return "SNAPSHOT";
            case FileType::SCRIPT:       return "SCRIPT";
            case FileType::ADF:          return "ADF";
            case FileType::EADF:         return "EADF";
            case FileType::HDF:          return "HDF";
            case FileType::IMG:          return "IMG";
            case FileType::ST:           return "ST";
            case FileType::DMS:          return "DMS";
            case FileType::EXE:          return "EXE";
            case FileType::DIR:          return "DIR";
            case FileType::ROM:          return "ROM";
            case FileType::EXTENDED_ROM: return "EXTENDED_ROM";
        }
        return "???";
    }
    static const char *help(FileType value)
    {
        switch (value) {
                
            case FileType::UNKNOWN:      return "Unknown";
            case FileType::SNAPSHOT:     return "Snapshot";
            case FileType::SCRIPT:       return "RetroShell Script";
            case FileType::ADF:          return "Amiga Disk File";
            case FileType::EADF:         return "Extended Amiga Disk File";
            case FileType::HDF:          return "Hard Disk File";
            case FileType::IMG:          return "PC Disk Image";
            case FileType::ST:           return "AtariST Disk Image";
            case FileType::DMS:          return "Disk Masher System";
            case FileType::EXE:          return "Amiga Executable";
            case FileType::DIR:          return "Directory";
            case FileType::ROM:          return "Kickstart ROM";
            case FileType::EXTENDED_ROM: return "Extended Kickstart ROM";
        }
        return "???";
    }
};


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

}
