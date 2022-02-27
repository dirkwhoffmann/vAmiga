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
//    return suffix == "HDF";
}

bool
HDFFile::isCompatible(std::istream &stream)
{
    return util::streamLength(stream) % 512 == 0;
}

void
HDFFile::finalizeRead()
{
    deriveGeomentry(); // DEPRECATED

    scanDisk();
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
        
        AmigaFile::init(drive.data, drive.driveSpec.geometry.numBytes());
    }
    
    // Overwrite the predicted geometry from the precise one
    driveSpec.geometry = drive.getGeometry();
}

const Geometry
HDFFile::getGeometry() const
{
    return driveSpec.geometry;
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
    return isize(driveSpec.partitions.size());
}


isize
HDFFile::numCyls() const
{
    return driveSpec.geometry.cylinders;
}

isize
HDFFile::numSides() const
{
    return driveSpec.geometry.heads;
}

isize
HDFFile::numSectors() const
{
    return driveSpec.geometry.sectors;
}

isize
HDFFile::numReserved() const
{
    return 2;
}

isize
HDFFile::numBlocks() const
{
    return size / bsize();
}

isize
HDFFile::bsize() const
{
    return driveSpec.geometry.bsize;
}

/*
FSDeviceDescriptor
HDFFile::layout()
{
    FSDeviceDescriptor result;
    
    // Copy the drive geometry
    result.geometry = driveSpec.geometry;
    result.numBlocks = result.geometry.numBlocks();
    
    // Set the number of reserved blocks
    result.numReserved = 2; // numReserved();

    // Only proceed if the hard drive is formatted
    if (dos(0) == FS_NODOS) return result;
    
    // Determine the location of the root block
    i64 highKey = result.numBlocks - 1;
    i64 rootKey = (result.numReserved + highKey) / 2;
    
    // Add partition
    result.partition = FSPartitionDescriptor(dos(0),
                                             0,
                                             result.geometry.upperCyl(),
                                             (Block)rootKey);

    // Seek bitmap blocks
    Block ref = (Block)rootKey;
    isize cnt = 25;
    isize offset = 512 - 49 * 4;
    
    while (ref && ref < (Block)result.numBlocks) {

        const u8 *p = data + (ref * 512) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (isize i = 0; i < cnt; i++, p += 4) {
            if (Block bmb = FSBlock::read32(p)) {
                if (bmb < result.numBlocks) {
                    result.partition.bmBlocks.push_back(bmb);
                }
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FSBlock::read32(p)) != 0) {
            if (ref < result.numBlocks) result.partition.bmExtBlocks.push_back(ref);
            cnt = (512 / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}
*/

FileSystemDescriptor
HDFFile::getFileSystemDescriptor(isize nr) const
{
    FileSystemDescriptor result;
        
    auto &part = driveSpec.partitions[nr];
    
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
    Block ref = (Block)rootKey;
    isize cnt = 25;
    isize offset = 512 - 49 * 4;
    
    while (ref && ref < (Block)result.numBlocks) {

        const u8 *p = dptr + (ref * 512) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (isize i = 0; i < cnt; i++, p += 4) {
            if (Block bmb = FSBlock::read32(p)) {
                if (bmb < result.numBlocks) {
                    result.bmBlocks.push_back(bmb);
                }
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FSBlock::read32(p)) != 0) {
            if (ref < result.numBlocks) result.bmExtBlocks.push_back(ref);
            cnt = (512 / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}

FSDeviceDescriptor
HDFFile::layoutOfPartition(isize nr) const
{
    FSDeviceDescriptor result;
        
    auto &part = driveSpec.partitions[nr];
    
    auto c = part.highCyl - part.lowCyl + 1;
    auto h = part.heads;
    auto s = part.sectors;
    
    result.geometry.cylinders = c;
    result.geometry.heads = h;
    result.geometry.sectors = s;
    result.geometry.bsize = 512;
    result.numBlocks = c * h * s;
    assert(c * h * s == result.geometry.numBlocks());

    // Determine block bounds
    auto first = part.lowCyl * h * s;
    auto dptr = data + first * 512;

    // Set the number of reserved blocks
    result.numReserved = 2;

    // Only proceed if the hard drive is formatted
    // if (dos(0) == FS_NODOS) return result;
    assert(dos(first) != FS_NODOS);
    
    // Determine the location of the root block
    i64 highKey = result.numBlocks - 1;
    i64 rootKey = (result.numReserved + highKey) / 2;
    
    // Add partition
    result.dos = dos(first);
    result.rootBlock = (Block)rootKey;
    /*
    result.partition = FSPartitionDescriptor(dos(first),
                                             0,
                                             c - 1,
                                             (Block)rootKey);
    */

    // Seek bitmap blocks
    Block ref = (Block)rootKey;
    isize cnt = 25;
    isize offset = 512 - 49 * 4;
    
    while (ref && ref < (Block)result.numBlocks) {

        const u8 *p = dptr + (ref * 512) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (isize i = 0; i < cnt; i++, p += 4) {
            if (Block bmb = FSBlock::read32(p)) {
                if (bmb < result.numBlocks) {
                    result.bmBlocks.push_back(bmb);
                }
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FSBlock::read32(p)) != 0) {
            if (ref < result.numBlocks) result.bmExtBlocks.push_back(ref);
            cnt = (512 / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}

u8 *
HDFFile::dataForPartition(isize nr) const
{
    auto &part = driveSpec.partitions[nr];
    return data + part.lowCyl * part.heads * part.sectors * 512;
}

void
HDFFile::deriveGeomentry()
{
    if (hasRDB()) {
        
        msg("RDB detected\n");
        predictGeometry();
        // TODO: GET GEOMETRY FROM THE RDB
        
    } else {
            
        msg("No RDB found. Geometry can only be predicted.\n");
        predictGeometry();
    }
}

void
HDFFile::predictGeometry()
{
    debug(true, "predictGeometry()\n");
    
    // Get all possible geometries
    auto geometries = Geometry::driveGeometries(size);
    
    // REMOVE ASAP
    for (const auto &g : geometries) {
        debug(true, "c: %ld h: %ld s: %ld\n", g.cylinders, g.heads, g.sectors);
    }

    // Use the first entry as the drive's geometry
    if (geometries.size()) {
        
        driveSpec.geometry = geometries.front();
    }
}

void
HDFFile::scanDisk()
{
    auto rdb = seekRDB();
        
    if (rdb) {

        // Read the information from the rigid disk block
        
        driveSpec.geometry.cylinders    = R32BE_ALIGNED(rdb + 64);
        driveSpec.geometry.sectors      = R32BE_ALIGNED(rdb + 68);
        driveSpec.geometry.heads        = R32BE_ALIGNED(rdb + 72);
        driveSpec.geometry.bsize        = R32BE_ALIGNED(rdb + 16);

        driveSpec.diskVendor            = util::createStr(rdb + 160, 8);
        driveSpec.diskProduct           = util::createStr(rdb + 168, 16);
        driveSpec.diskRevision          = util::createStr(rdb + 184, 4);
        driveSpec.controllerVendor      = util::createStr(rdb + 188, 8);
        driveSpec.controllerProduct     = util::createStr(rdb + 196, 16);
        driveSpec.controllerRevision    = util::createStr(rdb + 212, 4);
        
        scanPartitions();
    
    } else {
        
        // Predict the drive geometry by analyzing the file size
        predictGeometry();
        
        // Fill in default values
        driveSpec.diskVendor = "VAMIGA";
        driveSpec.diskProduct = "HARD DRIVE";
        driveSpec.diskRevision = "R1.0";
        driveSpec.controllerVendor = "VAMIGA";
        driveSpec.controllerProduct = "HDR CONTROLLER";
        driveSpec.controllerRevision = "R1.0";
        
        addDefaultPartition();
    }
}

void
HDFFile::scanPartitions()
{
    for (isize i = 0; i < 16; i++) {

        if (auto part = seekPB(i); part) {
        
            PartitionSpec partSpec;
            
            partSpec.name           = util::createStr(part + 37, 31);
            partSpec.flags          = R32BE_ALIGNED(part + 20);
            partSpec.sizeBlock      = R32BE_ALIGNED(part + 132);
            partSpec.heads          = R32BE_ALIGNED(part + 140);
            partSpec.sectors        = R32BE_ALIGNED(part + 148);
            partSpec.reserved       = R32BE_ALIGNED(part + 152);
            partSpec.interleave     = R32BE_ALIGNED(part + 160);
            partSpec.lowCyl         = R32BE_ALIGNED(part + 164);
            partSpec.highCyl        = R32BE_ALIGNED(part + 168);
            partSpec.numBuffers     = R32BE_ALIGNED(part + 172);
            partSpec.bufMemType     = R32BE_ALIGNED(part + 176);
            partSpec.maxTransfer    = R32BE_ALIGNED(part + 180);
            partSpec.mask           = R32BE_ALIGNED(part + 184);
            partSpec.bootPri        = R32BE_ALIGNED(part + 188);
            partSpec.dosType        = R32BE_ALIGNED(part + 192);

            driveSpec.partitions.push_back(partSpec);
        }
    }
}

void
HDFFile::addDefaultPartition()
{
    PartitionSpec partSpec;
    
    auto &geometry = getGeometry();
    
    partSpec.name           = "Default";
    partSpec.flags          = 1;
    partSpec.sizeBlock      = u32(geometry.bsize / 4);
    partSpec.heads          = u32(geometry.heads);
    partSpec.sectors        = u32(geometry.sectors);
    partSpec.reserved       = 2;
    partSpec.interleave     = 0;
    partSpec.lowCyl         = 0;
    partSpec.highCyl        = u32(geometry.cylinders - 1);
    partSpec.numBuffers     = 1;
    partSpec.bufMemType     = 0;
    partSpec.maxTransfer    = 0x7FFFFFFF;
    partSpec.mask           = 0xFFFFFFFE;
    partSpec.bootPri        = 0;
    partSpec.dosType        = 0x444f5300; // DOS0

    driveSpec.partitions.push_back(partSpec);
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
