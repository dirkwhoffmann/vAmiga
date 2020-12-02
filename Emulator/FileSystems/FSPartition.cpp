
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSPartition::FSPartition(FSDevice &ref, FSPartitionDescriptor layout) : dev(ref)
{
    lowCyl      = layout.lowCyl;
    highCyl     = layout.highCyl;
    firstBlock  = layout.firstBlock;
    lastBlock   = layout.lastBlock;
    rootBlock   = layout.rootBlock;
    
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
    
    // Do some consistency checking
    for (u32 i = firstBlock; i <= lastBlock; i++) assert(dev.blocks[i] == nullptr);
    
    // Create boot blocks
    dev.blocks[firstBlock]     = new FSBootBlock(*this, firstBlock, layout.dos);
    dev.blocks[firstBlock + 1] = new FSBootBlock(*this, firstBlock + 1, FS_NONE);

    // Create the root block
    FSRootBlock *rb = new FSRootBlock(*this, layout.rootBlock);
    dev.blocks[layout.rootBlock] = rb;
    
    // Create the bitmap blocks
    for (auto& ref : layout.bmBlocks) {
        
        debug("Creating bitmap block %d\n", ref);
        dev.blocks[ref] = new FSBitmapBlock(*this, ref);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : layout.bmExtBlocks) {
        
        dev.blocks[ref] = new FSBitmapExtBlock(*this, ref);
        pred->setNextBmExtBlockRef(ref);
        pred = dev.blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(layout.bmBlocks);
    
    // Add free blocks
    for (u32 i = firstBlock; i <= lastBlock; i++) {
        
        if (dev.blocks[i] == nullptr) {
            dev.blocks[i] = new FSEmptyBlock(*this, i);
            dev.markAsFree(i); // TODO: MUST BE PARTITION SPECIFIC
        }
    }
}

void
FSPartition::info()
{
    msg("DOS%ld  ",     dos());
    msg("%5d (x %3d) ", numBlocks(), dev.bsize);
    msg("%5d  ",        usedBlocks());
    msg("%5d   ",       freeBlocks());
    msg("%3d%%   ",     (int)(100.0 * usedBlocks() / numBlocks()));
    msg("%s\n",         getName().c_str());
    msg("\n");
}

FSName
FSPartition::getName()
{
    FSRootBlock *rb = dev.rootBlock(rootBlock);
    assert(rb != nullptr);
    
    return rb->getName();
}

void
FSPartition::setName(FSName name)
{
    FSRootBlock *rb = dev.rootBlock(rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
}

FSVolumeType
FSPartition::dos()
{
    FSBlock *b = dev.block(firstBlock);
    return b ? b->dos() : FS_NONE;
}

u32
FSPartition::numBlocks()
{
    return numCyls() * dev.numHeads * dev.numSectors;
}

u32
FSPartition::numBytes()
{
    return numBlocks() * dev.bsize;
}

u32
FSPartition::freeBlocks()
{
    u32 result = 0;
    
    for (size_t i = firstBlock; i <= lastBlock; i++) {
        if (isFree(i)) result++;
    }

    return result;
}

u32
FSPartition::usedBlocks()
{
    return highCyl - lowCyl + 1 - freeBlocks();
}

u32
FSPartition::freeBytes()
{
    return freeBlocks() * dev.bsize;
}

u32
FSPartition::usedBytes()
{
    return usedBlocks() * dev.bsize;
}

FSBitmapBlock *
FSPartition::bmBlockForBlock(u32 relRef)
{
    assert(relRef >= 2 && relRef < numBlocks());
        
    // Locate the bitmap block
    u32 bitsPerBlock = (dev.bsize - 4) * 8;
    u32 nr = (relRef - 2) / bitsPerBlock;

    if (nr >= bmBlocks.size()) {
        warn("Allocation bit is located in non-existent bitmap block %d\n", nr);
        return nullptr;
    }

    return dev.bitmapBlock(bmBlocks[nr]);
}

bool
FSPartition::isFree(u32 ref)
{
    assert(ref >= lowCyl && ref <= highCyl);
    
    // Translate rel to a relative block index
    ref -= lowCyl;
    
    // The first two blocks are always allocated and not part of the bitmap
    if (ref < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    u32 byte, bit;
    FSBitmapBlock *bm = locateAllocationBit(ref, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

FSBitmapBlock *
FSPartition::locateAllocationBit(u32 ref, u32 *byte, u32 *bit)
{
    FSBitmapBlock *bm;
    
    assert(ref >= firstBlock && ref <= lastBlock);
    
    // The first two blocks are not part the map
    if (ref < 2) return nullptr;
    
    // Locate the bitmap block
    if (!(bm = bmBlockForBlock(ref))) return nullptr;
    
    // Locate the byte position (note: the long word ordering will be reversed)
    u32 rByte = ((ref - 2) / 8);
    
    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < dev.bsize);
    
    *byte = rByte;
    *bit = (ref - 2) % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}
