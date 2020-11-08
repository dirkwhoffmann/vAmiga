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
    
    /*
    adf->dumpSector(880);
    adf->dumpSector(882);
    adf->dumpSector(883);
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


    // Create empty ADF
    ADFFile *adf = nullptr;
    if (disk->getType() == DISK_35_DD) adf = makeWithDiskType(DISK_35_DD);
    if (disk->getType() == DISK_35_HD) adf = makeWithDiskType(DISK_35_HD);
    if (!adf) return nullptr;
    
    // Export disk
    assert(adf->numTracks() == 160);
    assert(adf->numSectorsPerTrack() == 11 || adf->numSectorsPerTrack() == 22);
    if (!disk->decodeAmigaDisk(adf->data, adf->numTracks(), adf->numSectorsPerTrack())) {
        delete adf;
        return nullptr;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithVolume(FSVolume &volume)
{
    ADFFile *adf = nullptr;
    assert(volume.getBlockSize() == 512);
    
    switch (volume.getCapacity()) {
            
        case 2 * 880:
            adf = makeWithDiskType(DISK_35_DD);
            break;
            
        case 4 * 880:
            adf = makeWithDiskType(DISK_35_HD);
            break;
            
        default:
            assert(false);
    }

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

        FSVolume vol = FSVolume(FFS, "MyDisk", 2 * 880);
        if (fs == FS_EMPTY_FFS_BOOTABLE) vol.installBootBlock();
        vol.exportVolume(data, size);

    } else {

        FSVolume vol = FSVolume(OFS, "MyDisk", 2 * 880);
        if (fs == FS_EMPTY_OFS_BOOTABLE) vol.installBootBlock();
        vol.exportVolume(data, size);
    }

    return true;
}

bool
ADFFile::encodeMFM(Disk *disk)
{
    assert(disk != NULL);
    assert(disk->getType() == getDiskType());

    // Start with an unformatted disk
    disk->clearDisk();

    // Start the encoding process
    bool result = true;
    long tracks = numTracks();
    
    debug(MFM_DEBUG, "Encoding Amiga disk with %d tracks\n", tracks);

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) result &= encodeMFM(disk, t);

    // In debug mode, also run the decoder
    if (MFM_DEBUG) {
        debug("Amiga disk fully encoded (success = %d)\n", result);
        ADFFile *tmp = ADFFile::makeWithDisk(disk);
        if (tmp) {
            msg("Decoded image written to /tmp/debug.adf\n");
            tmp->writeToFile("/tmp/tmp.adf");
        }
    }

    return result;
}

bool
ADFFile::encodeMFM(Disk *disk, Track t)
{
    long sectors = numSectorsPerTrack();
    assert(disk->geometry.sectors == sectors);
    
    trace(MFM_DEBUG, "Encoding Amiga track %d (%d sectors)\n", t, sectors);

    // Format track
    disk->clearTrack(t, 0xAA);

    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) result &= encodeMFM(disk, t, s);
    
    // Rectify the first clock bit (where buffer wraps over)
    u8 *strt = disk->ptr(t);
    u8 *stop = disk->ptr(t) + disk->geometry.trackSize - 1;
    if (*stop & 0x01) *strt &= 0x7F;
    // if (data.track[t][trackSize - 1] & 1) data.track[t][0] &= 0x7F;

    // Compute a debug checksum
    if (MFM_DEBUG) {
        u64 check = fnv_1a_32(disk->ptr(t), disk->geometry.trackSize);
        debug("Track %d checksum = %x\n", t, check);
    }

    return result;
}

bool
ADFFile::encodeMFM(Disk *disk, Track t, Sector s)
{
    assert(t < disk->geometry.tracks);
    assert(s < disk->geometry.sectors);
    
    debug(MFM_DEBUG, "Encoding sector %d\n", s);
    
    // Block header layout:
    //
    //                         Start  Size   Value
    //     Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
    //     SYNC mark           04      4     0x44 0x89 0x44 0x89
    //     Track & sector info 08      8     Odd/Even encoded
    //     Unused area         16     32     0xAA
    //     Block checksum      48      8     Odd/Even encoded
    //     Data checksum       56      8     Odd/Even encoded
    
    u8 *p = disk->ptr(t, s);
    
    // Bytes before SYNC
    p[0] = (p[-1] & 1) ? 0x2A : 0xAA;
    p[1] = 0xAA;
    p[2] = 0xAA;
    p[3] = 0xAA;
    
    // SYNC mark
    u16 sync = 0x4489;
    p[4] = HI_BYTE(sync);
    p[5] = LO_BYTE(sync);
    p[6] = HI_BYTE(sync);
    p[7] = LO_BYTE(sync);
    
    // Track and sector information
    u8 info[4] = { 0xFF, (u8)t, (u8)s, (u8)(11 - s) };
    disk->encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (unsigned i = 16; i < 48; i++)
    p[i] = 0xAA;
    
    // Data
    u8 bytes[512];
    readSector(bytes, t, s);
    disk->encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    disk->encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    disk->encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(unsigned i = 8; i < 1088; i++) {
        p[i] = disk->addClockBits(p[i], p[i-1]);
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
