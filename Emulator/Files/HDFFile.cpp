// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include <string.h>

HDFFile::HDFFile()
{
}

bool
HDFFile::isHDFBuffer(const u8 *buffer, size_t length)
{
    // HDFs contain no magic bytes. We can only check the buffer size
    return length % 512 == 0;
}

bool
HDFFile::isHDFFile(const char *path)
{
    if (!checkFileSuffix(path, "hdf") &&
        !checkFileSuffix(path, "HDF")) {
        return false;
    }
    
    return getSizeOfFile(path) % 512 == 0;
}

HDFFile *
HDFFile::makeWithFile(const char *path)
{
    HDFFile *hdf = new HDFFile();
    
    if (!hdf->readFromFile(path)) {
        delete hdf;
        return nullptr;
    }
        
    return hdf;
}

/*
HDFFile *
HDFFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    HDFFile *hdf = new HDFFile();
    
    if (!hdf->readFromBuffer(buffer, length)) {
        delete hdf;
        return nullptr;
    }
    
    return hdf;
}
*/

bool
HDFFile::hasRDB()
{
    // The rigid disk block must be among the first 16 blocks
    if (size >= 16 * 512) {
        for (int i = 0; i < 16; i++) {
            if (strcmp((const char *)data + i * 512, "RDSK") == 0) return true;
        }
    }
    return false;
}

long
HDFFile::numCyls()
{
    assert(size % bsize() == 0);
    
    if (hasRDB()) warn("HDF RDB images are not supported");

    return size / bsize() / numSectors() / numSides();
}

long
HDFFile::numSides()
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 1;
}

long
HDFFile::numSectors()
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 32;
}

long
HDFFile::numReserved()
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 2;
}

long
HDFFile::numBlocks()
{
    assert((long)size / bsize() == numCyls() * numSides() * numSectors());
    return size / bsize();
}

long
HDFFile::bsize()
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 512;
}

FSDeviceDescriptor
HDFFile::layout()
{
    FSDeviceDescriptor result;
    
    result.numCyls     = numCyls();
    result.numHeads    = numSides();
    result.numSectors  = numSectors();
    result.numReserved = numReserved();
    result.bsize       = bsize();
    result.numBlocks   = result.numCyls * result.numHeads * result.numSectors;

    // Determine the location of the root block
    u32 highKey = result.numBlocks - 1;
    u32 rootKey = (result.numReserved + highKey) / 2;
    
    // Add partition
    result.partitions.push_back(FSPartitionDescriptor(dos(0), 0, result.numCyls - 1, rootKey));

    // Seek bitmap blocks
    u32 ref = rootKey;
    u32 cnt = 25;
    u32 offset = bsize() - 49 * 4;
    
    while (ref && ref < result.numBlocks) {

        const u8 *p = data + (ref * bsize()) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (u32 i = 0; i < cnt; i++, p += 4) {
            if (u32 bmb = FFSDataBlock::read32(p)) {
                if (bmb < result.numBlocks) result.partitions[0].bmBlocks.push_back(bmb);
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FFSDataBlock::read32(p))) {
            if (ref < result.numBlocks) result.partitions[0].bmExtBlocks.push_back(ref);
            cnt = (bsize() / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}

FSVolumeType
HDFFile::dos(int i)
{
    const char *p = (const char *)data + i * 512;
    
    if (strncmp(p, "DOS", 3) || data[3] > 7) {
        return FS_NODOS;
    }

    return (FSVolumeType)p[3];
}
