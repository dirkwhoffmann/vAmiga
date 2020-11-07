// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

EXT1File::EXT1File()
{
    setDescription("EXT1File");
}

bool
EXT1File::isEXT1Buffer(const u8 *buffer, size_t length)
{
    u8 signature[] = { 'U', 'A', 'E', '-', '-', 'A', 'D', 'F' };

    assert(buffer != nullptr);
    
    return
    length > HEADER_SIZE &&
    matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
EXT1File::isEXT1File(const char *path)
{
    u8 signature[] = { 'U', 'A', 'E', '-', '-', 'A', 'D', 'F' };

    assert(path != nullptr);
    
    return
    checkFileSizeRange(path, HEADER_SIZE, -1) &&
    matchingFileHeader(path, signature, sizeof(signature));
}

EXT1File *
EXT1File::makeWithBuffer(const u8 *buffer, size_t length)
{
    EXT1File *result = new EXT1File();
    
    if (!result->readFromBuffer(buffer, length)) {
        delete result;
        return NULL;
    }
    
    return result;
}

EXT1File *
EXT1File::makeWithFile(const char *path)
{
    EXT1File *result = new EXT1File();
    
    if (!result->readFromFile(path)) {
        delete result;
        return NULL;
    }
    
    return result;
}

EXT1File *
EXT1File::makeWithFile(FILE *file)
{
    EXT1File *result = new EXT1File();
    
    if (!result->readFromFile(file)) {
        delete result;
        return NULL;
    }
    
    return result;
}

bool
EXT1File::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    debug("readFromBuffer\n");
    dumpHeader();
    
    return isEXT1Buffer(buffer, length);
}

void
EXT1File::dumpHeader()
{
    u8 *p = data + 8;

    for (int i = 0; i < 160; i++, p += 4) {
        u32 sync = HI_LO(p[0],p[1]);
        u32 length = HI_LO(p[2],p[3]);
        msg("Track %3d: Sync: %5d Length: %5d\n", i, sync, length);
    }
}
