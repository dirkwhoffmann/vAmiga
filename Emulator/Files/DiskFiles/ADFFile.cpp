// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADFFile.h"
#include "BootBlockImage.h"
#include "Checksum.h"
#include "Disk.h"
#include "Drive.h"
#include "IO.h"
#include "MemUtils.h"

bool
ADFFile::isCompatiblePath(const string &path)
{
    auto suffix = util::extractSuffix(path);
    return suffix == "adf" || suffix == "ADF";
}

bool
ADFFile::isCompatibleStream(std::istream &stream)
{
    isize length = util::streamLength(stream);
    
    // Some ADFs contain an additional byte at the end. Ignore it.
    length &= ~1;
    
    // There are no magic bytes. Hence, we only check the file size.
    return
    length == ADFSIZE_35_DD ||
    length == ADFSIZE_35_DD_81 ||
    length == ADFSIZE_35_DD_82 ||
    length == ADFSIZE_35_DD_83 ||
    length == ADFSIZE_35_DD_84 ||
    length == ADFSIZE_35_HD;
}

ADFFile *
ADFFile::make(DiskDiameter diameter, DiskDensity density)
{
    assert_enum(DiskDiameter, diameter);
    
    ADFFile *adf = new ADFFile();
    
    adf->size = fileSize(diameter, density);
    adf->data = new u8[adf->size]();

    return adf;
}

ADFFile *
ADFFile::make(Disk &disk)
{
    DiskDiameter type = disk.getDiameter();
    DiskDensity density = disk.getDensity();

    // Create an empty ADF
    ADFFile *adf = make(type, density);
    
    // Export disk
    assert(adf->numTracks() == 160);
    assert(adf->numSectors() == 11 || adf->numSectors() == 22);
    try { adf->decodeDisk(disk); }
    catch (VAError &exception) { delete adf; throw exception; }
    
    return adf;
}

ADFFile *
ADFFile::make(Drive &drive)
{
    return drive.disk ? make(*drive.disk) : nullptr;
}

ADFFile *
ADFFile::make(FSDevice &volume)
{
    ADFFile *adf = nullptr;
    
    switch (volume.getCapacity()) {
            
        case 2 * 880:
            adf = make(INCH_35, DISK_DD);
            break;
            
        case 4 * 880:
            adf = make(INCH_35, DISK_HD);
            break;
            
        default:
            throw VAError(ERROR_FS_WRONG_CAPACITY);
    }

    ErrorCode ec;
    volume.exportVolume(adf->data, adf->size, &ec);
    if (ec != ERROR_OK) throw VAError(ec);
    
    return adf;
}

isize
ADFFile::fileSize(DiskDiameter diameter, DiskDensity density)
{
    assert_enum(DiskDiameter, density);
    
    if (diameter != INCH_35) throw VAError(ERROR_DISK_INVALID_DIAMETER);
    
    if (density == DISK_DD) return ADFSIZE_35_DD;
    if (density == DISK_HD) return ADFSIZE_35_HD;

    throw VAError(ERROR_DISK_INVALID_DENSITY);
}

FSVolumeType
ADFFile::getDos() const
{
    if (strncmp((const char *)data, "DOS", 3) || data[3] > 7) {
        return FS_NODOS;
    }

    return (FSVolumeType)data[3];
}

void
ADFFile::setDos(FSVolumeType dos)
{
    if (dos == FS_NODOS) {
        memset(data, 0, 4);
    } else {
        memcpy(data, "DOS", 3);
        data[3] = (u8)dos;
    }
}

DiskDiameter
ADFFile::getDiskDiameter() const
{
    return INCH_35;
}

DiskDensity
ADFFile::getDiskDensity() const
{
    return (size & ~1) == ADFSIZE_35_HD ? DISK_HD : DISK_DD;
}

isize
ADFFile::numSides() const
{
    return 2;
}

isize
ADFFile::numCyls() const
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

isize
ADFFile::numSectors() const
{
    switch (getDiskDensity()) {
            
        case DISK_DD: return 11;
        case DISK_HD: return 22;
            
        default:
            assert(false);
            return 0;
    }
}

FSDeviceDescriptor
ADFFile::layout()
{
    FSDeviceDescriptor result;
    
    result.numCyls     = numCyls();
    result.numHeads    = numSides();
    result.numSectors  = numSectors();
    result.numReserved = 2;
    result.bsize       = 512;
    result.numBlocks   = result.numCyls * result.numHeads * result.numSectors;

    // Determine the root block location
    Block root = size < ADFSIZE_35_HD ? 880 : 1760;

    // Determine the bitmap block location
    Block bitmap = FSBlock::read32(data + root * 512 + 316);
    
    // Assign a default location if the bitmap block reference is invalid
    if (bitmap == 0 || bitmap >= (Block)numBlocks()) bitmap = root + 1;
    
    // Add partition
    result.partitions.push_back(FSPartitionDescriptor(getDos(), 0, result.numCyls - 1, root));
    result.partitions[0].bmBlocks.push_back(bitmap);
    
    return result;
}

BootBlockType
ADFFile::bootBlockType() const
{
    return BootBlockImage(data).type;
}

const char *
ADFFile::bootBlockName() const
{
    return BootBlockImage(data).name;
}

void
ADFFile::killVirus()
{
    msg("Overwriting boot block virus with ");
    
    if (isOFSVolumeType(getDos())) {

        msg("a standard OFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BB_AMIGADOS_13);
        bb.write(data + 4, 4, 1023);

    } else if (isFFSVolumeType(getDos())) {

        msg("a standard FFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BB_AMIGADOS_20);
        bb.write(data + 4, 4, 1023);

    } else {

        msg("zeroes\n");
        memset(data + 4, 0, 1020);
    }
}

bool
ADFFile::formatDisk(FSVolumeType fs, BootBlockId id)
{
    assert_enum(FSVolumeType, fs);

    ErrorCode error;

    msg("Formatting disk with %lld blocks (%s)\n", numBlocks(), FSVolumeTypeEnum::key(fs));

    // Only proceed if a file system is given
    if (fs == FS_NODOS) return false;
    
    // Get a device descriptor for this ADF
    FSDeviceDescriptor descriptor = layout();
    descriptor.partitions[0].dos = fs;
    
    // Create an empty file system
    FSDevice *volume = FSDevice::make(descriptor);
    volume->setName(FSName("Disk"));
    
    // Write boot code
    volume->makeBootable(id);
    
    // Export the file system to the ADF
    volume->exportVolume(data, size, &error);
    delete(volume);

    if (error == ERROR_OK) {
        return true;
    } else {
        warn("Failed to export file system from ADF: %s\n", ErrorCodeEnum::key(error));
        return false;
    }
}

void
ADFFile::encodeDisk(Disk &disk)
{
    if (disk.getDiameter() != getDiskDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDiskDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }

    isize tracks = numTracks();
    debug(MFM_DEBUG, "Encoding Amiga disk with %zd tracks\n", tracks);

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(disk, t);

    // In debug mode, also run the decoder
    if (MFM_DEBUG) {
        msg("Amiga disk fully encoded\n");
        ADFFile *tmp = ADFFile::make(disk);
        if (tmp) {
            msg("Decoded image written to /tmp/debug.adf\n");
            tmp->writeToFile("/tmp/tmp.adf");
        }
    }
}

void
ADFFile::encodeTrack(Disk &disk, Track t)
{
    isize sectors = numSectors();
    debug(MFM_DEBUG, "Encoding Amiga track %d with %ld sectors\n", t, sectors);

    // Format track
    disk.clearTrack(t, 0xAA);

    // Encode all sectors
    for (Sector s = 0; s < sectors; s++) encodeSector(disk, t, s);
    
    // Rectify the first clock bit (where buffer wraps over)
    if (disk.data.track[t][disk.length.track[t] - 1] & 1) {
        disk.data.track[t][0] &= 0x7F;
    }
    
    // Compute a debug checksum
    debug(MFM_DEBUG, "Track %d checksum = %x\n",
          t, util::fnv_1a_32(disk.data.track[t], disk.length.track[t]));
}

void
ADFFile::encodeSector(Disk &disk, Track t, Sector s)
{
    assert(t < disk.numTracks());
    
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
    
    // Determine the start of this sector
    u8 *p = disk.data.track[t] + 700 + (s * 1088);
    
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
    Disk::encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (isize i = 16; i < 48; i++)
    p[i] = 0xAA;
    
    // Data
    u8 bytes[512];
    readSector(bytes, t, s);
    Disk::encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    Disk::encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    Disk::encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(isize i = 8; i < 1088; i++) {
        p[i] = Disk::addClockBits(p[i], p[i-1]);
    }    
}

void
ADFFile::dumpSector(Sector s) const
{
    util::hexdump(data + 512 * s, 512);
}

void
ADFFile::decodeDisk(Disk &disk)
{
    long tracks = numTracks();
    
    debug(MFM_DEBUG, "Decoding Amiga disk with %ld tracks\n", tracks);
    
    if (disk.getDiameter() != getDiskDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDiskDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }
        
    // Make the MFM stream scannable beyond the track end
    disk.repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(disk, t);
}

void
ADFFile::decodeTrack(Disk &disk, Track t)
{ 
    long sectors = numSectors();

    trace(MFM_DEBUG, "Decoding track %d\n", t);
    
    u8 *src = disk.data.track[t];
    u8 *dst = data + t * sectors * 512;
    
    // Seek all sync marks
    isize sectorStart[sectors], nr = 0; isize index = 0;
    while (index < isizeof(disk.data.track[t]) && nr < sectors) {

        // Scan MFM stream for $4489 $4489
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;

        // Make sure it's not a DOS track
        if (src[index + 1] == 0x89) continue;

        sectorStart[nr++] = index;
    }
    
    trace(MFM_DEBUG, "Found %zd sectors (expected %ld)\n", nr, sectors);

    if (nr != sectors) {
        
        warn("Found %zd sectors, expected %ld. Aborting.\n", nr, sectors);
        throw VAError(ERROR_DISK_WRONG_SECTOR_COUNT);
    }
    
    // Decode all sectors
    for (Sector s = 0; s < sectors; s++) {
        decodeSector(dst, src + sectorStart[s]);
    }
}

void
ADFFile::decodeSector(u8 *dst, u8 *src)
{
    assert(dst != nullptr);
    assert(src != nullptr);
    
    // Decode sector info
    u8 info[4];
    Disk::decodeOddEven(info, src, 4);
    
    // Only proceed if the sector number is valid
    u8 sector = info[2];
    if (sector >= numSectors()) {
        warn("Invalid sector number %d. Aborting.\n", sector);
        throw VAError(ERROR_DISK_INVALID_SECTOR_NUMBER);
    }
    
    // Skip sector header
    src += 56;
    
    // Decode sector data
    Disk::decodeOddEven(dst + sector * 512, src, 512);
}
