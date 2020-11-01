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
IMGFile::makeWithDiskType(DiskType t)
{
    assert(t == DISK_35_DD);
    
    IMGFile *img = new IMGFile();
    
    if (!img->alloc(9 * 160 * 512)) {
        delete img;
        return NULL;
    }
    
    memset(img->data, 0, img->size);
    return img;
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
    
    // We only support 3.5"DD disks at the moment
    if (disk->getType() != DISK_35_DD) { return NULL; }
    
    IMGFile *img = makeWithDiskType(DISK_35_DD);
    
    if (img) {
        if (!disk->decodeDOSDisk(img->data, 160, 9)) {
            printf("Failed to decode DOS disk\n");
            delete img;
            return NULL;
        }
    }
    
    return img;
}

bool
IMGFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isIMGBuffer(buffer, length);
}

DiskType
IMGFile::getDiskType()
{
    return DISK_35_DD_PC;
}

long
IMGFile::numSides()
{
    return 2;
}

long
IMGFile::numCyclinders()
{
    return 80;
}

long
IMGFile::numSectorsPerTrack()
{
    return 9;
}
