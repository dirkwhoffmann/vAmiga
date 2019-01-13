// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VATYPES_H
#define _VATYPES_H

//
// Amiga hardware
//

typedef enum {
    AMIGA_500,
    AMIGA_1000,
    AMIGA_2000
} AmigaModel;

inline bool isAmigaModel(AmigaModel model) {
    return model >= AMIGA_500 && model <= AMIGA_2000;
}

inline const char *modelName(AmigaModel model) {
    return
    model == AMIGA_500 ? "Amiga 500" :
    model == AMIGA_1000 ? "Amiga 1000" :
    model == AMIGA_2000 ? "Amiga 2000" : "???";
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
