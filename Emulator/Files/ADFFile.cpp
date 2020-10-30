// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ADFFile::ADFFile()
{
    setDescription("ADFFile");
}

bool
ADFFile::isADFBuffer(const u8 *buffer, size_t length)
{
    // Some ADFs contain an additional byte at the end. Ignore it
    length &= ~1;
    
    // There are no magic bytes. Hence, we only check the file size
    return
    length == ADFSIZE_35_DD ||
    length == ADFSIZE_35_DD_81 ||
    length == ADFSIZE_35_DD_82 ||
    length == ADFSIZE_35_DD_83 ||
    length == ADFSIZE_35_DD_84 ||
    length == ADFSIZE_35_HD;
}

bool
ADFFile::isADFFile(const char *path)
{
    // There are no magic bytes. Hence, we only check the file size
    return
    checkFileSize(path, ADFSIZE_35_DD) ||
    checkFileSize(path, ADFSIZE_35_DD_81) ||
    checkFileSize(path, ADFSIZE_35_DD_82) ||
    checkFileSize(path, ADFSIZE_35_DD_83) ||
    checkFileSize(path, ADFSIZE_35_DD_84) ||
    checkFileSize(path, ADFSIZE_35_HD) ||
    
    checkFileSize(path, ADFSIZE_35_DD+1) ||
    checkFileSize(path, ADFSIZE_35_DD_81+1) ||
    checkFileSize(path, ADFSIZE_35_DD_82+1) ||
    checkFileSize(path, ADFSIZE_35_DD_83+1) ||
    checkFileSize(path, ADFSIZE_35_DD_84+1) ||
    checkFileSize(path, ADFSIZE_35_HD+1);
}

size_t
ADFFile::fileSize(DiskType t)
{
    assert(isDiskType(t));
    
    switch(t) {
            
        case DISK_35_DD:    return ADFSIZE_35_DD;
        case DISK_35_HD:    return ADFSIZE_35_HD;
        default:            assert(false); return 0;
    }
}

ADFFile *
ADFFile::makeWithDiskType(DiskType t)
{
    assert(isDiskType(t));
    
    ADFFile *adf = new ADFFile();
    
    if (!adf->alloc(fileSize(t))) {
        delete adf;
        return NULL;
    }
    
    memset(adf->data, 0, adf->size);
    return adf;
}

ADFFile *
ADFFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromBuffer(buffer, length)) {
        delete adf;
        return NULL;
    }
        
    adf->dumpSector(880);
    adf->dumpSector(882);
    adf->dumpSector(883);

    return adf;
}

ADFFile *
ADFFile::makeWithFile(const char *path)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromFile(path)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithFile(FILE *file)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromFile(file)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithDisk(Disk *disk)
{
    assert(disk != NULL);
    
    // We only support 3.5"DD disks at the moment
    if (disk->getType() != DISK_35_DD) { return NULL; }
    
    ADFFile *adf = makeWithDiskType(DISK_35_DD);
    
    if (adf) {
        if (!disk->decodeAmigaDisk(adf->data, 160, 11)) {
            delete adf;
            return NULL;
        }
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithVolume(FSVolume &volume)
{
    assert(volume.getBlockSize() == 512);
    assert(volume.getCapacity() <= 2 * 880);
    
    ADFFile *adf = makeWithDiskType(DISK_35_DD);
    volume.exportVolume(adf->data, adf->size);
        
    return adf;
}

bool
ADFFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isADFBuffer(buffer, length);
}

DiskType
ADFFile::getDiskType()
{
    switch(size & ~1) {
        
        case ADFSIZE_35_DD:
        case ADFSIZE_35_DD_81:
        case ADFSIZE_35_DD_82:
        case ADFSIZE_35_DD_83:
        case ADFSIZE_35_DD_84: return DISK_35_DD;
        case ADFSIZE_35_HD:    return DISK_35_HD;
    }
    assert(false);
    return (DiskType)0;
}

long
ADFFile::numSides()
{
    return 2;
}

long
ADFFile::numCyclinders()
{
    switch(size & ~1) {
            
        case ADFSIZE_35_DD:    return 80;
        case ADFSIZE_35_DD_81: return 81;
        case ADFSIZE_35_DD_82: return 82;
        case ADFSIZE_35_DD_83: return 83;
        case ADFSIZE_35_DD_84: return 84;
        case ADFSIZE_35_HD:    return 80;
            
        default:
            assert(false);
            return 0;
    }
}

long
ADFFile::numSectorsPerTrack()
{
    switch (getDiskType()) {
            
        case DISK_35_DD: return 11;
        case DISK_35_HD: return 22;
            
        default:
            assert(false);
            return 0;
    }
}

bool
ADFFile::formatDisk(EmptyDiskFormat fs)
{
    assert(isEmptyDiskFormat(fs));
    
    // Only proceed if a file system is given
    if (fs == FS_EMPTY) return false;
    
    // Right now, only 3.5" DD disks can be formatted
    if (getDiskType() != DISK_35_DD) {
        warn("Cannot format a disk of type %s with file system %s.\n",
             diskTypeName(getDiskType()), emptyDiskFormatName(fs));
        return false;
    }
    
    // Create an empty file system
    if (fs == FS_EMPTY_FFS || fs == FS_EMPTY_FFS_BOOTABLE) {

        FFSVolume vol = FFSVolume("MyDisk");
        if (fs == FS_EMPTY_FFS_BOOTABLE) vol.installBootBlock();
        vol.exportVolume(data, size);

    } else {

        OFSVolume vol = OFSVolume("MyDisk");
        if (fs == FS_EMPTY_OFS_BOOTABLE) vol.installBootBlock();
        vol.exportVolume(data, size);
    }

    return true;
}

void
ADFFile::dumpSector(int num)
{
    u8 *p = data + 512 * num;
    int cols = 32;

    printf("Sector %d\n", num);
    for (int y = 0; y < 512 / cols; y++) {
        for (int x = 0; x < cols; x++) {
            printf("%02X ", p[y*cols + x]);
            if ((x % 4) == 3) printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}
