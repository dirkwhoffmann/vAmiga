// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "ExtFile.h"

// AROS Extended ROM
const u8 ExtFile::magicBytes1[] = { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };
const u8 ExtFile::magicBytes2[] = { 0x4E, 0x71, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };

ExtFile::ExtFile()
{
    setDescription("ExtRom");
}

bool
ExtFile::isExtBuffer(const u8 *buffer, size_t length)
{
    if (length != KB(512)) return false;

    return
    matchingBufferHeader(buffer, magicBytes1, sizeof(magicBytes1)) ||
    matchingBufferHeader(buffer, magicBytes2, sizeof(magicBytes2));
}

bool
ExtFile::isExtFile(const char *path)
{
    if (!checkFileSize(path, KB(512))) return false;

    return
    matchingFileHeader(path, magicBytes1, sizeof(magicBytes1)) ||
    matchingFileHeader(path, magicBytes2, sizeof(magicBytes2));
}

ExtFile *
ExtFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    ExtFile *rom = new ExtFile();

    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }

    return rom;
}

ExtFile *
ExtFile::makeWithFile(const char *path)
{
    ExtFile *rom = new ExtFile();

    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }

    return rom;
}

bool
ExtFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;

    return isExtBuffer(buffer, length);
}
