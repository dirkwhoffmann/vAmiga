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

HDFFile *
HDFFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    HDFFile *hdf = new HDFFile();
    
    if (!hdf->readFromBuffer(buffer, length)) {
        delete hdf;
        return nullptr;
    }
    
    printf("HDF loaded\n");
    u8 *p = hdf->getData();
    for (int i = 0; i < 32; i++) {
        printf("Block %d: %c%c%c%c\n", i, p[0], p[1], p[2], p[3]);
        p += 256;
    }

    return hdf;
}

bool
HDFFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isHDFBuffer(buffer, length);
}

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

/*
std::vector<FSPartitionDescriptor>
HDFFile::pTable()
{
    std::vector<FSPartitionDescriptor> result;
    
    if (hasRDB()) {
        
        // TODO: Extract the partition information from the disk image
        warn("HDF RDB images are not supported");
        
    } else {
        
        // Locate the root block
        u32 highKey = numCyls() * numSides() *  numSectors() - 1;
        u32 ref = (numReserved() + highKey) / 2;
                
        // Create a single partition
        result.push_back(FSPartitionDescriptor(0, numCyls(), ref));
        
        // Locate the bitmap blocks
        u32 cnt = 25;
        u32 offset = bsize() - 49 * 4;

        while (ref && ref < numBlocks()) {

            const u8 *p = data + (ref * bsize()) + offset;
        
            // Collect all references stored in this block
            for (u32 i = 0; i < cnt; i++, p += 4) {
                if (u32 bmb = FFSDataBlock::read32(p)) {
                    if (bmb < numBlocks()) result[0].bmBlocks.push_back(bmb);
                }
            }
            
            // Continue collecting in the next extension bitmap block
            if ((ref = FFSDataBlock::read32(p))) {
                if (ref < numBlocks()) result[0].bmExtBlocks.push_back(ref);
                cnt = (bsize() / 4) - 1;
                offset = 0;
            }
        }
    }
    return result;
}
*/
