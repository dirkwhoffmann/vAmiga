// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskFile *
DiskFile::makeWithFile(const char *path)
{
    ADFFile *adf = AmigaFile::make <ADFFile> (path);
    if (adf) return adf;
    
    IMGFile *img = AmigaFile::make <IMGFile> (path);
    if (img) return img;

    DMSFile *dms = AmigaFile::make <DMSFile> (path);
    if (dms) return dms;

    EXEFile *exe = AmigaFile::make <EXEFile> (path);
    if (exe) return exe;
    
    Folder *dir = AmigaFile::make <Folder> (path);
    if (dir) return dir;

    return nullptr;
}

u8
DiskFile::readByte(long t, long s, long offset) const
{
    return readByte(t * numSectors() + s, offset);
}

u8
DiskFile::readByte(long b, long offset) const
{
    assert(offset < 512);
    return data[b * 512 + offset];
}

void
DiskFile::readSector(u8 *dst, long t, long s) const
{
    readSector(dst, t * numSectors() + s);
}

void
DiskFile::readSector(u8 *dst, long s) const
{
    size_t sectorSize = 512;
    size_t offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= size);

    for (unsigned i = 0; i < sectorSize; i++) {
        dst[i] = data[offset + i];
    }
}

void
DiskFile::readSectorHex(char *dst, long t, long s, size_t count) const
{
    readSectorHex(dst, t * numSectors() + s, count);
}

void
DiskFile::readSectorHex(char *dst, long s, size_t count) const
{
    size_t sectorSize = 512;
    size_t offset = s * sectorSize;

    assert(dst != nullptr);

    for (unsigned i = 0; i < count; i++) {
        sprintf(dst + 3*i, "%02X ", data[offset + i]);
    }
}

bool
DiskFile::encodeDisk(class Disk *disk)
{
    assert(false);
    return false;
}

bool
DiskFile::decodeDisk(class Disk *disk)
{
    assert(false);
    return false;
}
