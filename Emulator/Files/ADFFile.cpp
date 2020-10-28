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
    // There are no magic bytes. Hence, we only check the file size.
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
    // There are no magic bytes. Hence, we only check the file size.
    return
    checkFileSize(path, ADFSIZE_35_DD) ||
    checkFileSize(path, ADFSIZE_35_DD_81) ||
    checkFileSize(path, ADFSIZE_35_DD_82) ||
    checkFileSize(path, ADFSIZE_35_DD_83) ||
    checkFileSize(path, ADFSIZE_35_DD_84) ||
    checkFileSize(path, ADFSIZE_35_HD);
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
    
    // REMOVE ASAP
    int sector = 880;
    int cols = 32;
    u8 *p = adf->data + 512*sector;
    printf("Sector %d\n", sector);
    for (int y = 0; y < 512 / cols; y++) {
        for (int x = 0; x < cols; x++) {
            if (x == cols / 2) printf(" ");
            printf("%02X ", p[y*cols + x]);
        }
        printf("\n");
    }
    printf("\n");


/*
    u8 *p = adf->data + 882*512;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 32; x++) {
            printf("%02X ", *p++);
        }
        printf("\n");
    }
*/

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
    switch(size) {
        
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
    switch(size) {
            
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
ADFFile::formatDisk(FileSystemType fs)
{
    assert(isFileSystemType(fs));
    
    // Only proceed if a file system is given
    if (fs == FS_NONE) return false;
    
    // Right now, only 3.5" DD disks can be formatted
    if (getDiskType() != DISK_35_DD) {
        warn("Cannot format a disk of type %s with file system %s.\n",
             diskTypeName(getDiskType()), fileSystemTypeName(fs));
        return false;
    }
    
    // Create an empty file system
    if (fs == FS_FFS || fs == FS_FFS_BOOTABLE) {

        FFSVolume vol = FFSVolume("MyDisk");
        if (fs == FS_FFS_BOOTABLE) vol.installBootBlock();
        vol.writeAsDisk(data, size);

    } else {

        OFSVolume vol = OFSVolume("MyDisk");
        if (fs == FS_OFS_BOOTABLE) vol.installBootBlock();

        // DEBUGGING: Add some directories
        debug("Adding some example entries...\n");
        vol.makeDir("Holla");
        vol.makeFile("ABC");
        vol.makeFile("Waldfee");
        vol.changeDir("Holla");
        vol.makeDir("die");
        vol.changeDir("die");
        vol.makeFile("file");
        vol.makeDir("Waldfee");
        vol.changeDir("Waldfee");
        
        debug("Done\n");

        // Perform a file system check
        vol.check();
        
        vol.writeAsDisk(data, size);
        
        int sector = 880; // 882;
        int cols = 32;
        u8 *p = data + 512*sector;
        msg("Sector %d\n", sector);
        for (int y = 0; y < 512 / cols; y++) {
            for (int x = 0; x < cols; x++) {
                if (x == cols / 2) msg(" ");
                msg("%02X ", p[y*cols + x]);
            }
            msg("\n");
        }
        msg("\n");
    }
                
    return true;
}
