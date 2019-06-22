// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "BootRom.h"

// Amiga 1000 Bootstrap - 252180-01 (USA, Europe)
// BROKEN. NO VALID JUMP VECTOR
// const uint8_t BootRom::magicBytes1[] = { 0x11, 0xF9, 0xF8, 0x8A, 0x00 };

// Amiga 1000 Bootstrap - 252179-01 (USA, Europe)
// BROKEN. NO VALID JUMP VECTOR
// const uint8_t BootRom::magicBytes2[] = { 0x11, 0x4E, 0x00, 0x00, 0x00 };

// Amiga 1000 Bootstrap (1985)
const uint8_t BootRom::magicBytes3[] = { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x8A };

// Amiga 1000 Bootstrap - amiga-boot-a1000
// const uint8_t BootRom::magicBytes4[] = { 0x41, 0x4D, 0x49, 0x52, 0x4F };

BootRom::BootRom()
{
    setDescription("BootRom");
}

bool
BootRom::isBootRomBuffer(const uint8_t *buffer, size_t length)
{
    /*
    if (length == KB(32)) {
        return
        matchingBufferHeader(buffer, magicBytes1, sizeof(magicBytes1)) ||
        matchingBufferHeader(buffer, magicBytes2, sizeof(magicBytes2));
    }
    if (length == KB(8)) {
        return
        matchingBufferHeader(buffer, magicBytes3, sizeof(magicBytes3));
    }
    if (length == 8203) {
        return
        matchingBufferHeader(buffer, magicBytes4, sizeof(magicBytes3));
    }
    */
    if (length == KB(64)) {
        return
        matchingBufferHeader(buffer, magicBytes3, sizeof(magicBytes3));
    }
    if (length == KB(8)) {
        return
        matchingBufferHeader(buffer, magicBytes3, sizeof(magicBytes3));
    }

    return false;
}

bool
BootRom::isBootRomFile(const char *path)
{
    /*
    if (checkFileSize(path, KB(32))) {
        return
        matchingFileHeader(path, magicBytes1, sizeof(magicBytes1)) ||
        matchingFileHeader(path, magicBytes2, sizeof(magicBytes2));
    }
    if (checkFileSize(path, KB(8))) {
        return
        matchingFileHeader(path, magicBytes3, sizeof(magicBytes3));
    }
    if (checkFileSize(path, 8203)) {
        return
        matchingFileHeader(path, magicBytes4, sizeof(magicBytes3));
    }
    */

    if (checkFileSize(path, KB(64))) {
        return
        matchingFileHeader(path, magicBytes3, sizeof(magicBytes3));
    }
    if (checkFileSize(path, KB(8))) {
        return
        matchingFileHeader(path, magicBytes3, sizeof(magicBytes3));
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
