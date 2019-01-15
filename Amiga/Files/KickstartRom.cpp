// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "KickstartRom.h"

const uint8_t KickstartRom::magicBytes[] = { 0x11, 0x11, 0x4E, 0xF9, 0x00 };

KickstartRom::KickstartRom()
{
    setDescription("KickstartRom");
}

bool
KickstartRom::isKickstartRomBuffer(const uint8_t *buffer, size_t length)
{
    if (length != KB(256) && length != KB(512)) return false;
    
    return
    matchingBufferHeader(buffer, magicBytes, sizeof(magicBytes));
}

bool
KickstartRom::isKickstartRomFile(const char *path)
{
    if (!checkFileSize(path, KB(256), KB(256)) &&
        !checkFileSize(path, KB(512), KB(512))) return false;
    
    return
    matchingFileHeader(path, magicBytes, sizeof(magicBytes));
}

KickstartRom *
KickstartRom::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    KickstartRom *rom = new KickstartRom();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

KickstartRom *
KickstartRom::makeWithFile(const char *path)
{
    KickstartRom *rom = new KickstartRom();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

bool
KickstartRom::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!VAFile::readFromBuffer(buffer, length))
        return false;
    
    return isKickstartRomBuffer(buffer, length);
}
