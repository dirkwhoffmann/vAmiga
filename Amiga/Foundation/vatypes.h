// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef VATYPES_H
#define VATYPES_H

//
// Amiga hardware
//

typedef enum {
    AMIGA_500,
    AMIGA_1000,
    AMIGA_2000
} VAModel;

inline bool isVAModel(VAModel model) {
    return model >= AMIGA_500 && model <= AMIGA_2000;
}


//
// External files (snapshots, disk images, etc.)
//

typedef enum
{
    FILETYPE_UKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_BOOT_ROM,
    FILETYPE_KICKSTART_ROM
    
} VAFileType;

inline bool isVAFileType(VAFileType model) {
    return model >= FILETYPE_UKNOWN && model <= FILETYPE_KICKSTART_ROM;
}

#endif
