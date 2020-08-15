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
    // There are no magic bytes. We can only check the buffer size.
    return
    length == ADFSIZE_35_DD ||
    length == ADFSIZE_35_DD_81 ||
    length == ADFSIZE_35_DD_82 ||
    length == ADFSIZE_35_DD_83 ||
    length == ADFSIZE_35_DD_84 ||
    length == ADFSIZE_35_DD_PC ||
    length == ADFSIZE_35_HD ||
    length == ADFSIZE_35_HD_PC ||
    length == ADFSIZE_525_SD;
}

bool
ADFFile::isADFFile(const char *path)
{
    // There are no magic bytes. We can only check the file size.
    return
    checkFileSize(path, ADFSIZE_35_DD) ||
    checkFileSize(path, ADFSIZE_35_DD_81) ||
    checkFileSize(path, ADFSIZE_35_DD_82) ||
    checkFileSize(path, ADFSIZE_35_DD_83) ||
    checkFileSize(path, ADFSIZE_35_DD_84) ||
    checkFileSize(path, ADFSIZE_35_DD_PC) ||
    checkFileSize(path, ADFSIZE_35_HD) ||
    checkFileSize(path, ADFSIZE_35_HD_PC) ||
    checkFileSize(path, ADFSIZE_525_SD);
}

size_t
ADFFile::fileSize(DiskType t)
{
    assert(isDiskType(t));
    
    switch(t) {
            
        case DISK_35_DD:    return ADFSIZE_35_DD;
        case DISK_35_DD_PC: return ADFSIZE_35_DD_PC;
        case DISK_35_HD:    return ADFSIZE_35_HD;
        case DISK_35_HD_PC: return ADFSIZE_35_HD_PC;
        case DISK_525_SD:   return ADFSIZE_525_SD;
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
        if (!disk->decodeDisk(adf->data, 160, 11)) {
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
        
        case ADFSIZE_35_DD:    return DISK_35_DD;
        case ADFSIZE_35_DD_81: return DISK_35_DD;
        case ADFSIZE_35_DD_82: return DISK_35_DD;
        case ADFSIZE_35_DD_83: return DISK_35_DD;
        case ADFSIZE_35_DD_84: return DISK_35_DD;
        case ADFSIZE_35_DD_PC: return DISK_35_DD_PC;
        case ADFSIZE_35_HD:    return DISK_35_HD;
        case ADFSIZE_35_HD_PC: return DISK_35_HD_PC;
        case ADFSIZE_525_SD:   return DISK_525_SD;
    }
    assert(false);
    return (DiskType)0;
}

const char *
ADFFile::sha()
{
    static char result[41];

    int error = sha_1(NULL, result, data, size);
    
    if (error) {
        warn("sha_1 returned with error code %d\n", error);
        result[0] = 0;
    }

    return result;
}

u64
ADFFile::fnv()
{
    return fnv_1a_64(data, size);
}

long
ADFFile::numSectorsPerTrack()
{
    switch (getDiskType()) {
        
        case DISK_35_DD:
        case DISK_35_HD:
        return 11;
        
        case DISK_35_DD_PC:
        case DISK_35_HD_PC:
        case DISK_525_SD:
        return 9;
    }
}

long
ADFFile::rootBlockNr()
{
    DiskType type = getDiskType();
    
    if (type == DISK_35_DD) return 880;
    if (type == DISK_35_HD) return 1760;

    // Don't call this method with an incompatible disk type
    assert(false);
    return 0;
}

bool
ADFFile::formatDisk(FileSystemType fs)
{
    assert(isFileSystemType(fs));
    
    // Only proceed if a file system is given
    if (fs == FS_NONE) return false;
    
    // Check if the disk is compatible with the provided file system
    DiskType type = getDiskType();
    if (type != DISK_35_DD && type != DISK_35_HD) {
        warn("Cannot format a disk of type %s with file system %s.\n",
             diskTypeName(type), fileSystemTypeName(fs));
        return false;
    }
    
    // Format the disk
    debug("Formatting a %d sector disk...\n", numSectorsTotal());
    debug("Sector %d is the root block.\n", rootBlockNr());
    
    // Wipe out all existing data
    memset(data, 0, size);
    
    // Write the boot block, the root block, and the bitmap block
    writeBootBlock(fs);
    writeRootBlock("Empty");
    writeBitmapBlock();
    return true;
}

void
ADFFile::writeBootBlock(FileSystemType fs)
{
    assert(data != NULL);
    assert(fs != FS_NONE);
    
    u8 ofs[] = {
        
        0xc0, 0x20, 0x0f, 0x19, 0x00, 0x00, 0x03, 0x70, 0x43, 0xfa, 0x00, 0x18,
        0x4e, 0xae, 0xff, 0xa0, 0x4a, 0x80, 0x67, 0x0a, 0x20, 0x40, 0x20, 0x68,
        0x00, 0x16, 0x70, 0x00, 0x4e, 0x75, 0x70, 0xff, 0x60, 0xfa, 0x64, 0x6f,
        0x73, 0x2e, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79
    };
    
    u8 ffs[] = {
        
        0xE3, 0x3D, 0x0E, 0x72, 0x00, 0x00, 0x03, 0x70, 0x43, 0xFA, 0x00, 0x3E,
        0x70, 0x25, 0x4E, 0xAE, 0xFD, 0xD8, 0x4A, 0x80, 0x67, 0x0C, 0x22, 0x40,
        0x08, 0xE9, 0x00, 0x06, 0x00, 0x22, 0x4E, 0xAE, 0xFE, 0x62, 0x43, 0xFA,
        0x00, 0x18, 0x4E, 0xAE, 0xFF, 0xA0, 0x4A, 0x80, 0x67, 0x0A, 0x20, 0x40,
        0x20, 0x68, 0x00, 0x16, 0x70, 0x00, 0x4E, 0x75, 0x70, 0xFF, 0x4E, 0x75,
        0x64, 0x6F, 0x73, 0x2E, 0x6C, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x00,
        0x65, 0x78, 0x70, 0x61, 0x6E, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x6C, 0x69,
        0x62, 0x72, 0x61, 0x72, 0x79, 0x00, 0x00, 0x00
    };
    
    // The boot block is located in sector 0 and 1
    u8 *p = data;
    
    // Write header
    p[0] = 'D';
    p[1] = 'O';
    p[2] = 'S';
    p[3] = (fs == FS_OFS || fs == FS_OFS_BOOTABLE) ? 0 : 1;
    
    // Make disk bootable
    if (fs == FS_OFS_BOOTABLE) memcpy(p + 4, ofs, sizeof(ofs));
    if (fs == FS_FFS_BOOTABLE) memcpy(p + 4, ffs, sizeof(ffs));
}

void
ADFFile::writeRootBlock(const char *label)
{
    u8 *p = data + rootBlockNr() * 512;
    
    // Type
    p[3] = 0x02;
    
    // Hashtable size (equals blocksize - 56)
    p[15] = 0x48;
    
    // Checksum
    
    // BM flag (true if bitmap on disk is valid)
    p[312] = p[313] = p[314] = p[315] = 0xFF;
    
    // BM pages (indicates the blocks containing the bitmap)
    p[318] = HI_BYTE(bitmapBlockNr());
    p[319] = LO_BYTE(bitmapBlockNr());
    
    // Last recent change of the root directory of this volume
    writeDate(p + 420, time(NULL));
    
    // Date and time when this volume was formatted
    writeDate(p + 484, time(NULL));
    
    // Volume name as a BCPL string (first byte is string length)
    size_t len = strlen(label);
    p[432] = (len > 30) ? 30 : len;
    strncpy((char *)(p + 433), label, len);
    p[463] = 0;
    
    // Secondary type indicates root block
    p[511] = 0x01;
    
    // Compute checksum
    u32 checksum = sectorChecksum(rootBlockNr());
    p[20] = BYTE3(checksum);
    p[21] = BYTE2(checksum);
    p[22] = BYTE1(checksum);
    p[23] = BYTE0(checksum);
}

void
ADFFile::writeBitmapBlock()
{
    u8 *p = data + bitmapBlockNr() * 512;
    
    // Write allocation table
    memset(p + 4, 0xFF, numSectorsTotal() / 8);
    
    // Mark the root block and the bitmap block as used
    p[4 + (rootBlockNr() / 8)] = 0x3F;
    
    // Compute checksum
    u32 checksum = sectorChecksum(bitmapBlockNr());
    p[0] = BYTE3(checksum);
    p[1] = BYTE2(checksum);
    p[2] = BYTE1(checksum);
    p[3] = BYTE0(checksum);
}

void
ADFFile::writeDate(u8 *dst, time_t date)
{
    /* Format used by the Amiga:
     *
     * Days  : Days since Jan 1, 1978
     * Mins  : Minutes since midnight
     * Ticks : Ticks past minute @ 50Hz
     */
    
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    date -= (8 * 365 + 2) * secPerDay;
    
    // Extract components
    u32 days  = date / secPerDay;
    u32 mins  = (date % secPerDay) / 60;
    u32 ticks = (date % secPerDay % 60) * 5 / 6;
    
    // Store value
    dst[0x0] = BYTE3(days);
    dst[0x1] = BYTE2(days);
    dst[0x2] = BYTE1(days);
    dst[0x3] = BYTE0(days);
    
    dst[0x4] = BYTE3(mins);
    dst[0x5] = BYTE2(mins);
    dst[0x6] = BYTE1(mins);
    dst[0x7] = BYTE0(mins);
    
    dst[0x8] = BYTE3(ticks);
    dst[0x9] = BYTE2(ticks);
    dst[0xA] = BYTE1(ticks);
    dst[0xB] = BYTE0(ticks);
}

u32
ADFFile::sectorChecksum(int sector)
{
    assert(isSectorNr(sector));

    u32 result = 0;

    u8 *p = data + sector * 512;
    for (unsigned i = 0; i < 512; i += 4, p += 4) {
        result += HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
    }
    
    return ~result + 1;
}

void
ADFFile::seekTrack(long nr)
{
    assert(isTrackNr(nr));
    
    fp = nr * (11 * 512);
    eof = (nr + 1) + (11 * 512);
}

void
ADFFile::seekSector(long nr)
{
    assert(isSectorNr(nr));
    
    fp = nr * 512;
    eof = (nr + 1) * 512;
}

void
ADFFile::readSector(u8 *target, long t, long s)
{
    assert(target != NULL);
    assert(isTrackNr(t));
    assert(isSectorNr(s));

    seekTrackAndSector(t, s);
    for (unsigned i = 0; i < 512; i++)
        target[i] = read();
    assert(read() == EOF);
}
