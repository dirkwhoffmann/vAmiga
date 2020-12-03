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

    partitions.push_back(FSPartitionDescriptor(dos, 0, numCyls - 1, root));
    partitions[0].bmBlocks.push_back(bitmap);
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

FSPartitionDescriptor::FSPartitionDescriptor(FSVolumeType dos,
                                             u32 firstCyl, u32 lastCyl,
                                             u32 root)
{
    this->dos = dos;
    this->lowCyl = firstCyl;
    this->highCyl = lastCyl;
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
