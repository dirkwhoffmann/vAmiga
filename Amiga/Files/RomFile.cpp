// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "RomFile.h"

//
// Boot Roms
//

const u8 RomFile::bootRomHeaders[1][8] = {

    // Amiga 1000 Bootstrap (1985)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x8A }
};

//
// Kickstart Roms
//

const u8 RomFile::kickRomHeaders[6][7] = {

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

RomFile::RomFile()
{
    setDescription("Rom");
}

bool
RomFile::isRomBuffer(const u8 *buffer, size_t length)
{
    // Boot Roms
    if (length == KB(8) || length == KB(16)) {

        int len = sizeof(bootRomHeaders[0]);
        int cnt = sizeof(bootRomHeaders) / len;

        for (int i = 0; i < cnt; i++)
            if (matchingBufferHeader(buffer, bootRomHeaders[i], len)) return true;

        return false;
    }

    // Kickstart Roms
    if (length == KB(256) || length == KB(512)) {

        int len = sizeof(kickRomHeaders[0]);
        int cnt = sizeof(kickRomHeaders) / len;

        for (int i = 0; i < cnt; i++)
            if (matchingBufferHeader(buffer, kickRomHeaders[i], len)) return true;

        return false;
    }

    return false;
}

bool
RomFile::isRomFile(const char *path)
{
    // Boot Roms
    if (checkFileSize(path, KB(8)) || checkFileSize(path, KB(16))) {

        int len = sizeof(bootRomHeaders[0]);
        int cnt = sizeof(bootRomHeaders) / len;

        for (int i = 0; i < cnt; i++)
            if (matchingFileHeader(path, bootRomHeaders[i], len)) return true;

        return false;
    }

    // Kickstart Roms
     if (checkFileSize(path, KB(256)) || checkFileSize(path, KB(512))) {

         int len = sizeof(kickRomHeaders[0]);
         int cnt = sizeof(kickRomHeaders) / len;

         for (int i = 0; i < cnt; i++)
             if (matchingFileHeader(path, kickRomHeaders[i], len)) return true;

         return false;
     }

    return false;
}

RomFile *
RomFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    RomFile *rom = new RomFile();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

RomFile *
RomFile::makeWithFile(const char *path)
{
    RomFile *rom = new RomFile();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

bool
RomFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isRomBuffer(buffer, length);
}
