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

#include <CoreFoundation/CoreFoundation.h>

typedef CF_ENUM(CFIndex, AmigaFileType)
{
    FILETYPE_UKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_DMS,
    FILETYPE_BOOT_ROM,
    FILETYPE_KICK_ROM,
    FILETYPE_ENCRYPTED_KICK_ROM,
    FILETYPE_EXT_ROM
};

inline bool isAmigaFileType(long value) {
    return value >= FILETYPE_UKNOWN && value <= FILETYPE_EXT_ROM;
}

typedef CF_ENUM(CFIndex, RomIdentifier)
{
    ROM_MISSING,
    ROM_UNKNOWN,

    // Boot Roms (A1000)
    ROM_BOOT_A1000_8K,
    ROM_BOOT_A1000_64K,

    // Kickstart V1.x
    ROM_KICK11_31_034,
    ROM_KICK12_33_166,
    ROM_KICK12_33_180,
    ROM_KICK121_34_004,
    ROM_KICK13_34_005,
    ROM_KICK13_34_005_SK,

    // Kickstart V2.x
    ROM_KICK20_36_028,
    ROM_KICK202_36_207,
    ROM_KICK204_37_175,
    ROM_KICK205_37_299,
    ROM_KICK205_37_300,
    ROM_KICK205_37_350,

    // Kickstart V3.x
    ROM_KICK30_39_106,
    ROM_KICK31_40_063,

    // Hyperion
    ROM_HYP314_46_143,

    // Free Kickstart Rom replacements
    ROM_AROS_55696,
    ROM_AROS_55696_EXT,

    // Diagnostic cartridges
    ROM_DIAG11,
    ROM_DIAG12,
    ROM_LOGICA20,

    ROM_CNT
};

static inline bool isRomRevision(long value) { return value >= 0 && value <= ROM_CNT; }

typedef enum
{
    DECRYPT_ROM_KEY_ERROR,
    DECRYPT_DATA_ERROR
}
DecryptionError;

#endif
