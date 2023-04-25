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
#include "Amiga.h"
#include "BootBlockImage.h"
#include "Checksum.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "IOUtils.h"
#include "MemUtils.h"
#include "MutableFileSystem.h"

namespace vamiga {

bool
ADFFile::isCompatible(const string &path)
{
    auto suffix = util::uppercased(util::extractSuffix(path));
    return suffix == "ADF";
}

bool
ADFFile::isCompatible(std::istream &stream)
{
    isize length = util::streamLength(stream);

    // Some ADFs contain an additional byte at the end. Ignore it.
    length &= ~1;

    // The size must be a multiple of the cylinder size
    if (length % 11264) return false;

    // Check some more limits
    return length <= ADFSIZE_35_DD_84 || length == ADFSIZE_35_HD;
}

isize
ADFFile::fileSize(Diameter diameter, Density density)
{
    assert_enum(Diameter, diameter);
    assert_enum(Density, density);

    if (diameter != INCH_35) throw VAError(ERROR_DISK_INVALID_DIAMETER);
    
    if (density == DENSITY_DD) return ADFSIZE_35_DD;
    if (density == DENSITY_HD) return ADFSIZE_35_HD;

    throw VAError(ERROR_DISK_INVALID_DENSITY);
}

void
ADFFile::init(Diameter diameter, Density density)
{
    assert_enum(Diameter, diameter);
    assert(data.empty());
    
    data.init(fileSize(diameter, density));
}

void
ADFFile::init(const FloppyDiskDescriptor &descr)
{
    if (descr.diameter != INCH_35) throw VAError(ERROR_DISK_INVALID_DIAMETER);

    switch (descr.density) {

        case DENSITY_DD:

            switch (descr.cylinders) {

                case 80: init(ADFSIZE_35_DD); break;
                case 81: init(ADFSIZE_35_DD_81); break;
                case 82: init(ADFSIZE_35_DD_82); break;
                case 83: init(ADFSIZE_35_DD_83); break;
                case 84: init(ADFSIZE_35_DD_84); break;

                default:
                    throw VAError(ERROR_DISK_INVALID_LAYOUT);
            }
            break;

        case DENSITY_HD:

            init(ADFSIZE_35_HD);
            break;

        default:
            throw VAError(ERROR_DISK_INVALID_DENSITY);
    }
}

void
ADFFile::init(FloppyDisk &disk)
{
    init(disk.getDiameter(), disk.getDensity());
    
    assert(numTracks() == 160);
    assert(numSectors() == 11 || numSectors() == 22);
    
    decodeDisk(disk);
}

void
ADFFile::init(FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw VAError(ERROR_DISK_MISSING);
    init(*drive.disk);
}

void
ADFFile::init(MutableFileSystem &volume)
{
    switch (volume.numBlocks()) {
            
        case 2 * 880:
            init(INCH_35, DENSITY_DD);
            break;
            
        case 4 * 880:
            init(INCH_35, DENSITY_HD);
            break;
            
        default:
            throw VAError(ERROR_FS_WRONG_CAPACITY);
    }

    volume.exportVolume(data.ptr, data.size);
}

void
ADFFile::finalizeRead()
{
    // Add some empty cylinders if the file contains less than 80
    if (data.size < ADFSIZE_35_DD) data.resize(ADFSIZE_35_DD, 0);
}

isize
ADFFile::numCyls() const
{
    switch(data.size & ~1) {
            
        case ADFSIZE_35_DD:    return 80;
        case ADFSIZE_35_DD_81: return 81;
        case ADFSIZE_35_DD_82: return 82;
        case ADFSIZE_35_DD_83: return 83;
        case ADFSIZE_35_DD_84: return 84;
        case ADFSIZE_35_HD:    return 80;
            
        default:
            fatalError;
    }
}

isize
ADFFile::numHeads() const
{
    return 2;
}

isize
ADFFile::numSectors() const
{
    switch (getDensity()) {
            
        case DENSITY_DD: return 11;
        case DENSITY_HD: return 22;
            
        default:
            fatalError;
    }
}

FSVolumeType
ADFFile::getDos() const
{
    if (strncmp((const char *)data.ptr, "DOS", 3) || data[3] > 7) {
        return FS_NODOS;
    }

    return (FSVolumeType)data[3];
}

void
ADFFile::setDos(FSVolumeType dos)
{
    if (dos == FS_NODOS) {
        std::memset(data.ptr, 0, 4);
    } else {
        std::memcpy(data.ptr, "DOS", 3);
        data[3] = (u8)dos;
    }
}

Diameter
ADFFile::getDiameter() const
{
    return INCH_35;
}

Density
ADFFile::getDensity() const
{
    return (data.size & ~1) == ADFSIZE_35_HD ? DENSITY_HD : DENSITY_DD;
}

FileSystemDescriptor
ADFFile::getFileSystemDescriptor() const
{
    FileSystemDescriptor result;
    
    // Determine the root block location
    Block root = data.size < ADFSIZE_35_HD ? 880 : 1760;

    // Determine the bitmap block location
    Block bitmap = FSBlock::read32(data.ptr + root * 512 + 316);
    
    // Assign a default location if the bitmap block reference is invalid
    if (bitmap == 0 || bitmap >= (Block)numBlocks()) bitmap = root + 1;

    // Setup the descriptor
    result.numBlocks = numCyls() * numHeads() * numSectors(); // TODO: REPLACE BY numBlocks()
    assert(result.numBlocks == numBlocks());
    result.bsize = 512;
    result.numReserved = 2;
    result.dos = getDos();
    result.rootBlock = root;
    result.bmBlocks.push_back(bitmap);
    
    return result;
}

BootBlockType
ADFFile::bootBlockType() const
{
    return BootBlockImage(data.ptr).type;
}

const char *
ADFFile::bootBlockName() const
{
    return BootBlockImage(data.ptr).name;
}

void
ADFFile::killVirus()
{
    debug(ADF_DEBUG, "Overwriting boot block virus with ");
    
    if (isOFSVolumeType(getDos())) {

        plain(ADF_DEBUG, "a standard OFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BB_AMIGADOS_13);
        bb.write(data.ptr + 4, 4, 1023);

    } else if (isFFSVolumeType(getDos())) {

        plain(ADF_DEBUG, "a standard FFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BB_AMIGADOS_20);
        bb.write(data.ptr + 4, 4, 1023);

    } else {

        plain(ADF_DEBUG, "zeroes\n");
        std::memset(data.ptr + 4, 0, 1020);
    }
}

void
ADFFile::formatDisk(FSVolumeType fs, BootBlockId id, string name)
{
    assert_enum(FSVolumeType, fs);

    debug(ADF_DEBUG,
          "Formatting disk (%ld, %s)\n", numBlocks(), FSVolumeTypeEnum::key(fs));

    // Only proceed if a file system is given
    if (fs == FS_NODOS) return;
    
    // Get a device descriptor for this ADF
    auto descriptor = getFileSystemDescriptor();
    descriptor.dos = fs;
    
    // Create an empty file system
    MutableFileSystem volume(descriptor);
    volume.setName(FSName(name));
    
    // Write boot code
    volume.makeBootable(id);
    
    // Export the file system to the ADF
    if (!volume.exportVolume(data.ptr, data.size)) throw VAError(ERROR_FS_UNKNOWN);
}

void
ADFFile::encodeDisk(FloppyDisk &disk) const
{
    if (disk.getDiameter() != getDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }

    isize tracks = numTracks();
    debug(ADF_DEBUG, "Encoding Amiga disk with %ld tracks\n", tracks);

    // Start with an unformatted disk
    disk.clearDisk();

    // Encode all tracks
    for (Track t = 0; t < tracks; t++) encodeTrack(disk, t);

    // In debug mode, also run the decoder
    if constexpr (ADF_DEBUG) {
        
        ADFFile adf(disk);
        auto tmp = Amiga::tmp("debug.adf").string();
        debug(ADF_DEBUG, "Saving image to %s for debugging\n", tmp.c_str());
        adf.writeToFile(tmp);
    }
}

void
ADFFile::encodeTrack(FloppyDisk &disk, Track t) const
{
    isize sectors = numSectors();
    debug(ADF_DEBUG, "Encoding Amiga track %ld with %ld sectors\n", t, sectors);

    // Format track
    disk.clearTrack(t, 0xAA);

    // Encode all sectors
    for (Sector s = 0; s < sectors; s++) encodeSector(disk, t, s);
    
    // TODO: Remove after while
    assert((disk.data.track[t][disk.length.track[t] - 1] & 1) == disk.readBit(t, disk.length.track[t] * 8 - 1));

    // Rectify the first clock bit (where the buffer wraps over)
    if (disk.readBit(t, disk.length.track[t] * 8 - 1)) {
        disk.writeBit(t, 0, 0);
    }
    /*
    if (disk.data.track[t][disk.length.track[t] - 1] & 1) {
        disk.data.track[t][0] &= 0x7F;
    }
    */

    // Compute a debug checksum
    debug(ADF_DEBUG, "Track %ld checksum = %x\n",
          t, util::fnv32(disk.data.track[t], disk.length.track[t]));
}

void
ADFFile::encodeSector(FloppyDisk &disk, Track t, Sector s) const
{
    assert(t < disk.numTracks());
    
    debug(ADF_DEBUG, "Encoding sector %ld\n", s);
    
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
    u8 *p = disk.data.track[t] + (s * 1088);

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
    FloppyDisk::encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (isize i = 16; i < 48; i++)
        p[i] = 0xAA;
    
    // Data
    u8 bytes[512];
    readSector(bytes, t, s);
    FloppyDisk::encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    FloppyDisk::encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(isize i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    FloppyDisk::encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(isize i = 8; i < 1088; i++) {
        p[i] = FloppyDisk::addClockBits(p[i], p[i-1]);
    }
}

void
ADFFile::dumpSector(Sector s) const
{
    util::hexdump(data.ptr + 512 * s, 512);
}

void
ADFFile::decodeDisk(FloppyDisk &disk)
{
    long tracks = numTracks();
    
    debug(ADF_DEBUG, "Decoding Amiga disk with %ld tracks\n", tracks);
    
    if (disk.getDiameter() != getDiameter()) {
        throw VAError(ERROR_DISK_INVALID_DIAMETER);
    }
    if (disk.getDensity() != getDensity()) {
        throw VAError(ERROR_DISK_INVALID_DENSITY);
    }

    // Make the MFM stream scannable beyond the track end
    disk.repeatTracks();

    // Decode all tracks
    for (Track t = 0; t < tracks; t++) decodeTrack(disk, t);
}

void
ADFFile::decodeTrack(FloppyDisk &disk, Track t)
{ 
    long sectors = numSectors();

    debug(ADF_DEBUG, "Decoding track %ld\n", t);
    
    u8 *src = disk.data.track[t];
    u8 *dst = data.ptr + t * sectors * 512;
    
    // Seek all sync marks
    std::vector<isize> sectorStart(sectors);
    isize nr = 0; isize index = 0;
    
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
    
    debug(ADF_DEBUG, "Found %ld sectors (expected %ld)\n", nr, sectors);

    if (nr != sectors) {
        
        warn("Found %ld sectors, expected %ld. Aborting.\n", nr, sectors);
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
    FloppyDisk::decodeOddEven(info, src, 4);
    
    // Only proceed if the sector number is valid
    u8 sector = info[2];
    if (sector >= numSectors()) {
        warn("Invalid sector number %d. Aborting.\n", sector);
        throw VAError(ERROR_DISK_INVALID_SECTOR_NUMBER);
    }
    
    // Skip sector header
    src += 56;
    
    // Decode sector data
    FloppyDisk::decodeOddEven(dst + sector * 512, src, 512);
}

}
