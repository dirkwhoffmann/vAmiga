// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSDevice.h"

FSDeviceDescriptor::FSDeviceDescriptor(DiskType type, DiskDensity density, FSVolumeType dos)
{
    if (type == DISK_525 && density == DISK_DD) {
        numCyls = 40; numSectors = 11;

    } else if (type == DISK_35 && density == DISK_DD) {
        numCyls = 80; numSectors = 11;
    
    } else if (type == DISK_35 && density == DISK_HD) {
        numCyls = 80; numSectors = 22;

    } else {
        assert(false);
    }

    numHeads    = 2;
    numBlocks   = numCyls * numHeads * numSectors;
    numReserved = 2;
    bsize       = 512;
    
    // Determine the location of the root block and the bitmap block
    u32 root   = numBlocks / 2;
    u32 bitmap = root + 1;

    partitions.push_back(FSPartitionDescriptor(*this, dos, 0, numCyls - 1, root));
    partitions[0].bmBlocks.push_back(bitmap);
}

FSDeviceDescriptor::FSDeviceDescriptor(ADFFile *adf)
{
    numCyls     = adf->numCylinders();
    numHeads    = adf->numSides();
    numSectors  = adf->numSectors();
    numReserved = 2;
    bsize       = 512;
    numBlocks   = numCyls * numHeads * numSectors;

    // Determine the location of the root block and the bitmap block
    u32 root = adf->rootBlock();
    u32 bitmap = adf->bitmapBlock();
    
    // Add partition
    partitions.push_back(FSPartitionDescriptor(*this, adf->dos(), 0, numCyls - 1, root));
    partitions[0].bmBlocks.push_back(bitmap);
}

FSDeviceDescriptor::FSDeviceDescriptor(HDFFile *hdf)
{
    numCyls     = hdf->numCyls();
    numHeads    = hdf->numSides();
    numSectors  = hdf->numSectors();
    numReserved = hdf->numReserved();
    bsize       = hdf->bsize();
    numBlocks   = numCyls * numHeads * numSectors;

    // Determine the location of the root block
    u32 highKey = numCyls * numHeads * numSectors - 1;
    u32 rootKey = (numReserved + highKey) / 2;

    // Determine the file system type
    FSVolumeType dos = hdf->dos(0); 
    
    // Add partition
    partitions.push_back(FSPartitionDescriptor(*this, dos, 0, numCyls - 1, rootKey));

    // Seek bitmap blocks
    u32 ref = rootKey;
    u32 cnt = 25;
    u32 offset = bsize - 49 * 4;
    
    while (ref && ref < numBlocks) {

        const u8 *p = hdf->getData() + (ref * bsize) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (u32 i = 0; i < cnt; i++, p += 4) {
            if (u32 bmb = FFSDataBlock::read32(p)) {
                if (bmb < numBlocks) partitions[0].bmBlocks.push_back(bmb);
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FFSDataBlock::read32(p))) {
            if (ref < numBlocks) partitions[0].bmExtBlocks.push_back(ref);
            cnt = (bsize / 4) - 1;
            offset = 0;
        }
    }
}

void
FSDeviceDescriptor::dump()
{
    msg("            cyls : %d\n", numCyls);
    msg("           heads : %d\n", numHeads);
    msg("         sectors : %d\n", numSectors);
    msg("          blocks : %d\n", numBlocks);
    msg("        reserved : %d\n", numReserved);
    msg("           bsize : %d\n", bsize);
    
    for (size_t i = 0; i < partitions.size(); i++) {
        partitions[i].dump();
    }
}

FSPartitionDescriptor::FSPartitionDescriptor(FSDeviceDescriptor &layout,
                         FSVolumeType dos, u32 firstCyl, u32 lastCyl, u32 root)
{
    this->dos = dos;
    this->lowCyl = firstCyl;
    this->highCyl = lastCyl;
    // this->firstBlock = firstCyl * blocksPerCyl;
    // this->lastBlock = (lastCyl + 1) * blocksPerCyl - 1;
    this->rootBlock = root;

    assert(bmBlocks.size() == 0);
    assert(bmExtBlocks.size() == 0);
}

void
FSPartitionDescriptor::dump()
{
    msg("       Partition : %d - %d\n", lowCyl, highCyl);
    msg("      Root block : %d\n", rootBlock);
    msg("   Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("Extension blocks : ");
    for (auto& it : bmExtBlocks) { msg("%d ", it); }
    msg("\n\n");
}
