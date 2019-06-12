// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "ExtRom.h"

// AROS Extended ROM
const uint8_t ExtRom::magicBytes1[] = { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };

ExtRom::ExtRom()
{
    setDescription("ExtRom");
}

bool
ExtRom::isExtRomBuffer(const uint8_t *buffer, size_t length)
{
    printf("length = %zu %d\n", length, KB(512));

    if (length != KB(512)) return false;

    printf("length = %zu %d\n", length, KB(512));

    return matchingBufferHeader(buffer, magicBytes1, sizeof(magicBytes1));
}

bool
ExtRom::isExtRomFile(const char *path)
{
    if (!checkFileSize(path, KB(512))) return false;

    return matchingFileHeader(path, magicBytes1, sizeof(magicBytes1));
}

ExtRom *
ExtRom::makeWithBuffer(const uint8_t *buffer, size_t length)
{
    ExtRom *rom = new ExtRom();

    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }

    return rom;
}

ExtRom *
ExtRom::makeWithFile(const char *path)
{
    ExtRom *rom = new ExtRom();

    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }

    return rom;
}

bool
ExtRom::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;

    return isExtRomBuffer(buffer, length);
}
