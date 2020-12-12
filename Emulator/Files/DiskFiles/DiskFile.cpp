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
    ADFFile *adf = ADFFile::makeWithFile(path);
    if (adf) return adf;
    
    IMGFile *img = IMGFile::makeWithFile(path);
    if (img) return img;

    DMSFile *dms = DMSFile::makeWithFile(path);
    if (dms) return dms;

    EXEFile *exe = EXEFile::makeWithFile(path);
    if (exe) return exe;
    
    DIRFile *dir = DIRFile::makeWithFile(path);
    if (dir) return dir;

    return NULL;
}

u8
DiskFile::readByte(long t, long s, long offset)
{
    return readByte(t * numSectors() + s, offset);
}

u8
DiskFile::readByte(long b, long offset)
{
    assert(offset < 512);
    return data[b * 512 + offset];
}

void
DiskFile::readSector(u8 *dst, long t, long s)
{
    readSector(dst, t * numSectors() + s);
}

void
DiskFile::readSector(u8 *dst, long s)
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
DiskFile::readSectorHex(char *dst, long t, long s, size_t count)
{
    readSectorHex(dst, t * numSectors() + s, count);
}

void
DiskFile::readSectorHex(char *dst, long s, size_t count)
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
