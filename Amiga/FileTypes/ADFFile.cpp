// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#include "ADFFile.h"

ADFFile::ADFFile()
{
    setDescription("ADFFile");
}

bool
ADFFile::isADFBuffer(const uint8_t *buffer, size_t length)
{
    // There are no magic bytes. We can only check the buffer size.
    return
    length == ADFSIZE_35_DD ||
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
    
    memset(adf->data, 0, t);
    return adf;
}

ADFFile *
ADFFile::makeWithBuffer(const uint8_t *buffer, size_t length)
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

/*
ADFFile *
ADFFile::makeUnformatted(DiskType type)
{
    size_t size;
    
    switch(type) {

        case DISK_35_DD:
            size = 901120;
            break;
            
        case DISK_525_SD:
            size = 450560;
            break;
    }
    
    ADFFile *adf = new ADFFile();
    
    if (!adf->alloc(size)) {
        delete adf;
        return NULL;
    }
    
    memset(adf->data, 0, size);
    return adf;
}
*/

ADFFile *
ADFFile::makeFormatted(DiskType type, FileSystemType fs)
{
    ADFFile *adf = makeWithDiskType(type);
    
    if (adf) {
        
        int numSectors;
        int rootBlockSector;
        
        switch(type) {
                
            case DISK_35_DD:
                numSectors = 2 * 880;
                rootBlockSector = 880;
                break;
                
            case DISK_525_SD:
                numSectors = 2 * 440;
                rootBlockSector = 440;
                break;
                
            default: return NULL; 
        }
        
        // Format the disk
        adf->writeBootBlock(fs);
        adf->writeRootBlock(rootBlockSector, "Empty");
        adf->writeBitmapBlock(rootBlockSector + 1, numSectors);
    }
    
    return adf;
}

bool
ADFFile::readFromBuffer(const uint8_t *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isADFBuffer(buffer, length);
}

void
ADFFile::formatDisk(FileSystemType fs, bool bootable)
{
    
}

void
ADFFile::writeBootBlock(FileSystemType fs)
{
    assert(data != NULL);

    uint8_t ofs[] = {
        
        0xc0, 0x20, 0x0f, 0x19, 0x00, 0x00, 0x03, 0x70, 0x43, 0xfa, 0x00, 0x18,
        0x4e, 0xae, 0xff, 0xa0, 0x4a, 0x80, 0x67, 0x0a, 0x20, 0x40, 0x20, 0x68,
        0x00, 0x16, 0x70, 0x00, 0x4e, 0x75, 0x70, 0xff, 0x60, 0xfa, 0x64, 0x6f,
        0x73, 0x2e, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79
    };
    
    uint8_t ffs[] = {
        
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
    uint8_t *p = data;
    
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
ADFFile::writeRootBlock(uint32_t blockNr, const char *label)
{
    uint8_t *p = data + blockNr * 512;
    
    // Type
    p[3] = 0x02;
    
    // Hashtable size (equals blocksize - 56)
    p[15] = 0x48;
    
    // Checksum
    
    // BM flag (true if bitmap on disk is valid)
    p[312] = p[313] = p[314] = p[315] = 0xFF;
    
    // BM pages (indicates the blocks containing the bitmap)
    p[318] = HI_BYTE(blockNr + 1);
    p[319] = LO_BYTE(blockNr + 1);
    
    // Last altered date and time / Creation date and time
    writeDate(420, time(NULL));
    memcpy(p + 472, p + 420, 12);
    memcpy(p + 472, p + 420, 12);
    
    // Volume name as a BCPL string (first byte is string length)
    size_t len = strlen(label);
    p[432] = (len > 30) ? 30 : len;
    strncpy((char *)(p + 433), label, len);
    p[463] = 0;
    
    // Secondary type indicates root block
    p[511] = 0x01;
    
    // Compute checksum
    uint32_t checksum = sectorChecksum(blockNr);
    p[20] = BYTE3(checksum);
    p[21] = BYTE2(checksum);
    p[22] = BYTE1(checksum);
    p[23] = BYTE0(checksum);
}

void
ADFFile::writeBitmapBlock(uint32_t blockNr, uint32_t numSectors)
{
    uint8_t *p = data + blockNr * 512;
    
    // Write allocation table
    memset(p + 4, 0xFF, numSectors / 8);
    p[4 + (blockNr / 8)] = 0x3F;
    
    // Compute checksum
    uint32_t checksum = sectorChecksum(blockNr);
    p[0] = BYTE3(checksum);
    p[1] = BYTE2(checksum);
    p[2] = BYTE1(checksum);
    p[3] = BYTE0(checksum);
}

void
ADFFile::writeDate(int offset, time_t date)
{
    /* Format used by the Amiga:
     *
     * Days  : Days since Jan 1, 1978
     * Mins  : Minutes since midnight
     * Ticks : Ticks past minute @ 50Hz
     */
    
    const uint32_t secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    date -= (8 * 365 + 2) * secPerDay;
    
    // Extract components
    uint32_t days  = date / secPerDay;
    uint32_t mins  = (date % secPerDay) / 60;
    uint32_t ticks = (date % secPerDay % 60) * 5 / 6;
    
    // Store values
    uint8_t *p = data + offset;
    
    p[0x0] = BYTE3(days);
    p[0x1] = BYTE2(days);
    p[0x2] = BYTE1(days);
    p[0x3] = BYTE0(days);
    
    p[0x4] = BYTE3(mins);
    p[0x5] = BYTE2(mins);
    p[0x6] = BYTE1(mins);
    p[0x7] = BYTE0(mins);
    
    p[0x8] = BYTE3(ticks);
    p[0x9] = BYTE2(ticks);
    p[0xA] = BYTE1(ticks);
    p[0xB] = BYTE0(ticks);
}

uint32_t
ADFFile::sectorChecksum(int sector)
{
    assert(isSectorNr(sector));
    
    uint8_t *p = data + sector * 512;
    uint32_t checksum = 0;
    
    for (unsigned i = 0; i < 512; i += 4, p += 4) {
        checksum += HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
    }
    
    return ~checksum + 1;
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
ADFFile::readSector(uint8_t *target, long t, long s)
{
    assert(target != NULL);
    assert(isTrackNr(t));
    assert(isSectorNr(s));

    seekTrackAndSector(t, s);
    for (unsigned i = 0; i < 512; i++)
        target[i] = read();
    assert(read() == EOF);
}
