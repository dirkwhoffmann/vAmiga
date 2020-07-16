// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _FILE_TYPES_H
#define _FILE_TYPES_H

typedef enum
{
    FILETYPE_UKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_DMS,
    FILETYPE_BOOT_ROM,
    FILETYPE_KICK_ROM,
    FILETYPE_ENCRYPTED_KICK_ROM,
    FILETYPE_EXT_ROM
}
AmigaFileType;

inline bool isAmigaFileType(long value) {
    return value >= FILETYPE_UKNOWN && value <= FILETYPE_EXT_ROM;
}

#endif
