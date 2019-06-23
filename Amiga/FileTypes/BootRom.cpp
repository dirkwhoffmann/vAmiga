// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "BootRom.h"

// Amiga 1000 Bootstrap (1985)
const uint8_t BootRom::magicBytes[] = { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x8A };


BootRom::BootRom()
{
    setDescription("BootRom");
}

bool
BootRom::isBootRomBuffer(const uint8_t *buffer, size_t length)
{
    if (length == KB(64)) {
        return
        matchingBufferHeader(buffer, magicBytes, sizeof(magicBytes));
    }
    if (length == KB(8)) {
        return
        matchingBufferHeader(buffer, magicBytes, sizeof(magicBytes));
    }

    return false;
}

bool
BootRom::isBootRomFile(const char *path)
{
    if (checkFileSize(path, KB(64))) {
        return
        matchingFileHeader(path, magicBytes, sizeof(magicBytes));
    }
    if (checkFileSize(path, KB(8))) {
        return
        matchingFileHeader(path, magicBytes, sizeof(magicBytes));
    }

    return false;
}

BootRom *
BootRom::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    BootRom *rom = new BootRom();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

BootRom *
BootRom::makeWithFile(const char *path)
{
    BootRom *rom = new BootRom();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

bool
BootRom::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isBootRomBuffer(buffer, length);
}
