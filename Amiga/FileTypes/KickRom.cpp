// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "KickRom.h"

const uint8_t KickRom::magicBytes[KickRom::signatureCnt][7] = {

    // AROS Kickstart replacement
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 1.2 and 1.3
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFC, 0x00 },

    // Kickstart 2.04
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 3.1
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    // { 0x11, 0x16, 0x4E, 0xF9, 0x00, 0x20, 0x00 }, not working
    
    // Diagnostic v2.0 (Logica)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x04 }
};

KickRom::KickRom()
{
    setDescription("KickRom");
}

bool
KickRom::isKickRomBuffer(const uint8_t *buffer, size_t length)
{
    if (length != KB(256) && length != KB(512)) return false;

    for (int i = 0; i < signatureCnt; i++) {
        if (matchingBufferHeader(buffer, magicBytes[i], sizeof(magicBytes[i])))
            return true;
    }

    return false;
}

bool
KickRom::isKickRomFile(const char *path)
{
    if (!checkFileSize(path, KB(256)) &&
        !checkFileSize(path, KB(512))) return false;

    for (int i = 0; i < signatureCnt; i++) {
        if (matchingFileHeader(path, magicBytes[i], sizeof(magicBytes[i])))
            return true;
    }

    return false;
}

KickRom *
KickRom::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    KickRom *rom = new KickRom();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

KickRom *
KickRom::makeWithFile(const char *path)
{
    KickRom *rom = new KickRom();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

bool
KickRom::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isKickRomBuffer(buffer, length);
}
