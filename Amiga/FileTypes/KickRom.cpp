// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "KickRom.h"

const uint8_t KickRom::magicBytes[] = { 0x11, 0x11, 0x4E, 0xF9, 0x00 };

KickRom::KickRom()
{
    setDescription("KickRom");
}

bool
KickRom::isKickRomBuffer(const uint8_t *buffer, size_t length)
{
    if (length != KB(256) && length != KB(512)) return false;
    
    return
    matchingBufferHeader(buffer, magicBytes, sizeof(magicBytes));
}

bool
KickRom::isKickRomFile(const char *path)
{
    if (!checkFileSize(path, KB(256), KB(256)) &&
        !checkFileSize(path, KB(512), KB(512))) return false;
    
    return
    matchingFileHeader(path, magicBytes, sizeof(magicBytes));
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
