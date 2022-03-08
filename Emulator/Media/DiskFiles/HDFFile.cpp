// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HDFFile.h"
#include "Chrono.h"
#include "HardDrive.h"
#include "IOUtils.h"
#include "Memory.h"
#include "StringUtils.h"

bool
HDFFile::isCompatible(const string &path)
{
    return util::uppercased(util::extractSuffix(path)) == "HDF";
}

bool
HDFFile::isCompatible(std::istream &stream)
{
    return util::streamLength(stream) % 512 == 0;
}

void
HDFFile::finalizeRead()
{
    geometry = getGeometryDescriptor();
    ptable = getPartitionDescriptors();

    // Check the hard drive descriptor for consistency
    geometry.checkCompatibility();

    // Check the partition table for consistency
    for (auto &p :  ptable) {
        
        p.checkCompatibility();
        if (isize(p.highCyl) > geometry.cylinders) {
            throw(ERROR_HDR_CORRUPTED_PTABLE);
        }
    }
}

void
HDFFile::init(const string &path)
{
    // Check size
    if (isOversized(util::getSizeOfFile(path))) throw VAError(ERROR_HDR_TOO_LARGE);
    
    AmigaFile::init(path);

    // TODO: Check if geometry can be derived
    // TODO: Check if this disk contains an RDB which is not supported yet
}

void
HDFFile::init(const u8 *buf, isize len)
{
    // Check size
    if (isOversized(len)) throw VAError(ERROR_HDR_TOO_LARGE);

    AmigaFile::init(buf, len);

    // TODO: Check if geometry can be derived
    // TODO: Check if this disk contains an RDB which is not supported yet
}

void
HDFFile::init(const HardDrive &drive)
{
    // TODO: THIS FUNCTION IS A PERFORMANCE KILLER FOR LARGE BUFFERS
    {   MEASURE_TIME("AmigaFile::init(const u8 *buf, isize len)")
        
        AmigaFile::init(drive.data, drive.geometry.numBytes());
    }
    
    // Overwrite the predicted geometry from the precise one
    geometry = drive.getGeometry();
}

isize
HDFFile::numCyls() const
{
    return geometry.cylinders;
}

isize
HDFFile::numHeads() const
{
    return geometry.heads;
}

isize
HDFFile::numSectors() const
{
    return geometry.sectors;
}

GeometryDescriptor
HDFFile::getGeometryDescriptor() const
{
    GeometryDescriptor result;

    if (auto rdb = seekRDB(); rdb) {

        // Read the information from the rigid disk block
        result.cylinders    = R32BE_ALIGNED(rdb + 64);
        result.sectors      = R32BE_ALIGNED(rdb + 68);
        result.heads        = R32BE_ALIGNED(rdb + 72);
        result.bsize        = R32BE_ALIGNED(rdb + 16);

    } else {
        
        // Guess the drive geometry based on the file size
        auto geometries = GeometryDescriptor::driveGeometries(size);
        if (geometries.size()) {
            result = geometries.front();
        }
    }
        
    return result;
}

PartitionDescriptor
HDFFile::getPartitionDescriptor(isize part) const
{
    PartitionDescriptor result;
    
    if (auto pb = seekPB(part); pb) {
        
        // Read the information from the partition block
        result.name           = util::createStr(pb + 37, 31);
        result.flags          = R32BE_ALIGNED(pb + 20);
        result.sizeBlock      = R32BE_ALIGNED(pb + 132);
        result.heads          = R32BE_ALIGNED(pb + 140);
        result.sectors        = R32BE_ALIGNED(pb + 148);
        result.reserved       = R32BE_ALIGNED(pb + 152);
        result.interleave     = R32BE_ALIGNED(pb + 160);
        result.lowCyl         = R32BE_ALIGNED(pb + 164);
        result.highCyl        = R32BE_ALIGNED(pb + 168);
        result.numBuffers     = R32BE_ALIGNED(pb + 172);
        result.bufMemType     = R32BE_ALIGNED(pb + 176);
        result.maxTransfer    = R32BE_ALIGNED(pb + 180);
        result.mask           = R32BE_ALIGNED(pb + 184);
        result.bootPri        = R32BE_ALIGNED(pb + 188);
        result.dosType        = R32BE_ALIGNED(pb + 192);
        
    } else {
        
        assert(part == 0);
        
        // Add a default partition spanning the whole disk
        auto geo = getGeometryDescriptor();
        result = PartitionDescriptor(geo);
        
        // Make the first partition bootable
        result.flags |= 1;
    }
    
    return result;
}

std::vector<PartitionDescriptor>
HDFFile::getPartitionDescriptors() const
{
    std::vector<PartitionDescriptor> result;
    
    // Add the first partition (which always exists)
    result.push_back(getPartitionDescriptor(0));
    
    // Add other partitions (if any)
    for (isize i = 1; i < 16; i++) {
        if (auto pb = seekPB(i); pb) {
            result.push_back(getPartitionDescriptor(i));
        }
    }
    
    return result;
}

FileSystemDescriptor
HDFFile::getFileSystemDescriptor(isize nr) const
{
    FileSystemDescriptor result;
        
    auto &part = ptable[nr];
    
    auto c = part.highCyl - part.lowCyl + 1;
    auto h = part.heads;
    auto s = part.sectors;
    
    result.numBlocks = c * h * s;

    // Determine block bounds
    auto first = part.lowCyl * h * s;
    auto dptr = data + first * 512;

    // Set the number of reserved blocks
    result.numReserved = 2;

    // Set the DOS revision
    result.dos = dos(first);

    // Only proceed if the hard drive is formatted
    if (dos(first) == FS_NODOS) return result;
    
    // Determine the location of the root block
    i64 highKey = result.numBlocks - 1;
    i64 rootKey = (result.numReserved + highKey) / 2;
    
    // Add partition
    result.rootBlock = (Block)rootKey;

    // Seek bitmap blocks
    Block ref = Block(rootKey);
    isize cnt = 25;
    isize offset = 512 - 49 * 4;
    
    while (ref && ref < (Block)result.numBlocks) {

        const u8 *p = dptr + (ref * 512) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (isize i = 0; i < cnt; i++, p += 4) {
            if (Block bmb = FSBlock::read32(p)) {
                if (isize(bmb) < result.numBlocks) {
                    result.bmBlocks.push_back(bmb);
                }
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FSBlock::read32(p)) != 0) {
            if (isize(ref) < result.numBlocks) result.bmExtBlocks.push_back(ref);
            cnt = (512 / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}

bool
HDFFile::hasRDB() const
{
    // The rigid disk block must be among the first 16 blocks
    if (size >= 16 * 512) {
        for (isize i = 0; i < 16; i++) {
            if (strcmp((const char *)data + i * 512, "RDSK") == 0) return true;
        }
    }
    return false;
}

isize
HDFFile::numPartitions() const
{
    return isize(ptable.size());
}

isize
HDFFile::partitionSize(isize nr) const
{
    auto &part = ptable[nr];
    return (part.highCyl - part.lowCyl + 1) * part.heads * part.sectors * 512;
}

isize
HDFFile::partitionOffset(isize nr) const
{
    auto &part = ptable[nr];
    return part.lowCyl * part.heads * part.sectors * 512;
}

u8 *
HDFFile::partitionData(isize nr) const
{
    return data + partitionOffset(nr);
}

u8 *
HDFFile::seekBlock(isize nr) const
{
    return nr >= 0 && 512 * (nr + 1) <= size ? data + (512 * nr) : nullptr;
}

u8 *
HDFFile::seekRDB() const
{
    // The rigid disk block must be among the first 16 blocks
    for (isize i = 0; i < 16; i++) {
        if (auto p = seekBlock(i); p) {
            if (strcmp((const char *)p, "RDSK") == 0) return p;
        }
    }
    return nullptr;
}

u8 *
HDFFile::seekPB(isize nr) const
{
    u8 *result = nullptr;
    
    // Go to the rigid disk block
    if (auto rdb = seekRDB(); rdb) {
        
        // Go to the first partition block
        result = seekBlock(R32BE_ALIGNED(rdb + 28));
        
        // Traverse the linked list
        for (isize i = 0; i < nr && result; i++) {
            result = seekBlock(R32BE_ALIGNED(result + 16));
        }

        // Make sure the reached block is a partition block
        if (result && strcmp((const char *)result, "PART")) result = nullptr;
    }
    
    return result;
}

std::optional<string>
HDFFile::rdbString(isize offset, isize len) const
{
    if (auto rdb = seekRDB(); rdb) {
        return util::createStr(rdb + offset, len);
    }
    
    return { };
}

FSVolumeType
HDFFile::dos(isize blockNr) const
{
    if (auto block = seekBlock(blockNr); block) {
        
        if (strncmp((const char *)block, "DOS", 3) || block[3] > 7) {
            return FS_NODOS;
        }
        return (FSVolumeType)block[3];
    }
    
    return FS_NODOS;
}

isize
HDFFile::writePartitionToFile(const string &path, isize nr)
{
    auto offset = partitionOffset(nr);
    auto size = partitionSize(nr);
    
    return writeToFile(path, offset, size);
}
