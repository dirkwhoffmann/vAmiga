// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

IMGFile::IMGFile()
{
    setDescription("IMGFile");
}

bool
IMGFile::isIMGBuffer(const u8 *buffer, size_t length)
{
    // There are no magic bytes. We can only check the buffer size
    return
    length == IMGSIZE_35_DD;
}

bool
IMGFile::isIMGFile(const char *path)
{
    // There are no magic bytes. We can only check the file size
    return
    checkFileSize(path, IMGSIZE_35_DD);
}

IMGFile *
IMGFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromBuffer(buffer, length)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(const char *path)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(path)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(FILE *file)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(file)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithDisk(Disk *disk)
{
    assert(disk != NULL);
    
    // IMPLEMENTATION MISSING
    assert(false);
    return NULL;
}

bool
IMGFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isIMGBuffer(buffer, length);
}

u64
IMGFile::fnv()
{
    return fnv_1a_64(data, size);
}

void
IMGFile::readSector(u8 *dst, long t, long s)
{
    assert(dst != NULL);
    assert(isTrackNr(t));
    assert(isSectorNr(s));

    long offset = 512 * (9 * t + s);
    assert(offset + 512 <= size);

    for (unsigned i = 0; i < 512; i++) {
        dst[i] = data[offset + i];
    }
}
