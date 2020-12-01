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

FSPartition::FSPartition(u32 firstCyl, u32 lastCyl, u32 blocksPerCyl, u32 root)
{
    this->lowCyl = firstCyl;
    this->highCyl = lastCyl;
    this->firstBlock = firstCyl * blocksPerCyl;
    this->lastBlock = (lastCyl + 1) * blocksPerCyl - 1;
    this->rootBlock = root;

    assert(bmBlocks.size() == 0);
    assert(bmExtBlocks.size() == 0);
}

void
FSPartition::dump()
{
    msg("  First cylinder : %d\n", lowCyl);
    msg("   Last cylinder : %d\n", highCyl);
    msg("     First block : %d\n", firstBlock);
    msg("      Last block : %d\n", lastBlock);
    msg("      Root block : %d\n", rootBlock);
    msg("   Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("Extension blocks : ");
    for (auto& it : bmExtBlocks) { msg("%d ", it); }
    msg("\n\n");
}

/*
u32
FSPartition::rootBlockRef()
{
    u32 highKey = numCyls() * dev.heads * dev.sectors - 1;
    u32 rootKey = (dev.reserved + highKey) / 2;
    
    return rootKey;
}

FSRootBlock *
FSPartition::rootBlockPtr()
{
    return dev.rootBlock(rootBlockRef());
}
*/

/*
FSLayout::FSLayout(u32 c, u32 h, u32 s, u32 r, u32 b)
: cyls(c), heads(h), sectors(s), bsize(b)
{
    reserved = 2;
    blocks = cyls * heads * sectors;
    
    // Compute the root key if none is provided
    if (r == 0) {
        
        // Ignore extra cyclinders if a diskette layout is specified
        u32 cc = c < 44 ? 40 : c < 84 ? 80 : c;
        
        // Compute the location of the root block
        u32 highKey = cc * h * s - 1;
        r = (reserved + highKey) / 2;
    }
    
    // Create the partition
    part.push_back(FSPartition(0, c - 1, r));
}
*/

FSLayout::FSLayout(DiskType type, DiskDensity density)
{
    if (type == DISK_525 && density == DISK_DD) {
        cyls = 40; sectors = 11;

    } else if (type == DISK_35 && density == DISK_DD) {
        cyls = 80; sectors = 11;
    
    } else if (type == DISK_35 && density == DISK_HD) {
        cyls = 80; sectors = 22;

    } else {
        assert(false);
    }

    heads    = 2;
    blocks   = cyls * heads * sectors;
    reserved = 2;
    bsize    = 512;
    
    // Determine the location of the root block and the bitmap block
    u32 root = blocks / 2;
    u32 bm   = root + 1;

    part.push_back(FSPartition(0, cyls - 1, sectors * heads, root));
    part[0].bmBlocks.push_back(bm);
}

FSLayout::FSLayout(ADFFile *adf)
{
    cyls     = adf->numCylinders();
    sectors  = adf->numSectors();
    heads    = adf->numSides();
    reserved = 2;
    bsize    = 512;
    blocks   = cyls * heads * sectors;

    // Determine the location of the root block and the bitmap block
    u32 root = blocks / 2;
    u32 bm   = FSBlock::read32(adf->getData() + root * bsize + 316);
    
    // Add partition
    part.push_back(FSPartition(0, cyls - 1, sectors * heads, root));
    part[0].bmBlocks.push_back(bm);
}

FSLayout::FSLayout(HDFFile *hdf)
{
    cyls     = hdf->numCyls();
    sectors  = hdf->numSectors();
    heads    = hdf->numSides();
    reserved = hdf->numReserved();
    bsize    = hdf->bsize();
    blocks   = cyls * heads * sectors;

    // Determine the location of the root block
    u32 highKey = cyls * heads * sectors - 1;
    u32 rootKey = (reserved + highKey) / 2;

    // Add partition
    part.push_back(FSPartition(0, cyls - 1, sectors * heads, rootKey));

    // Seek bitmap blocks
    u32 ref = rootKey;
    u32 cnt = 25;
    u32 offset = bsize - 49 * 4;
    
    while (ref && ref < blocks) {

        const u8 *p = hdf->getData() + (ref * bsize) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (u32 i = 0; i < cnt; i++, p += 4) {
            if (u32 bmb = FFSDataBlock::read32(p)) {
                if (bmb < blocks) part[0].bmBlocks.push_back(bmb);
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FFSDataBlock::read32(p))) {
            if (ref < blocks) part[0].bmExtBlocks.push_back(ref);
            cnt = (bsize / 4) - 1;
            offset = 0;
        }
    }
}

void
FSLayout::dump()
{
    msg("            cyls : %d\n", cyls);
    msg("           heads : %d\n", heads);
    msg("         sectors : %d\n", sectors);
    msg("          blocks : %d\n", blocks);
    msg("        reserved : %d\n", reserved);
    msg("           bsize : %d\n", bsize);
    
    for (size_t i = 0; i < part.size(); i++) {
        msg("Partition %d:\n", i);
        part[i].dump();
    }
}
