
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSPartition.h"
#include "FSDevice.h"
#include <algorithm>
#include <vector>

FSPartition::FSPartition(FSDevice &dev, FSPartitionDescriptor &layout) : FSPartition(dev)
{
    dos         = layout.dos;
    lowCyl      = layout.lowCyl;
    highCyl     = layout.highCyl;
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
    
    firstBlock  = (Block)(lowCyl * dev.numHeads * dev.numSectors);
    lastBlock   = (Block)((highCyl + 1) * dev.numHeads * dev.numSectors - 1);
    
    // Do some consistency checking
    for (Block i = firstBlock; i <= lastBlock; i++) assert(dev.blocks[i] == nullptr);
    
    // Create boot blocks
    dev.blocks[firstBlock]     = new FSBlock(*this, firstBlock, FS_BOOT_BLOCK);
    dev.blocks[firstBlock + 1] = new FSBlock(*this, firstBlock + 1, FS_BOOT_BLOCK);

    // Create the root block
    FSBlock *rb = new FSBlock(*this, rootBlock, FS_ROOT_BLOCK);
    dev.blocks[layout.rootBlock] = rb;
    
    // Create the bitmap blocks
    for (auto& ref : layout.bmBlocks) {
        
        dev.blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_BLOCK);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : layout.bmExtBlocks) {
        
        dev.blocks[ref] = new FSBlock(*this, ref, FS_BITMAP_EXT_BLOCK);
        pred->setNextBmExtBlockRef(ref);
        pred = dev.blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(layout.bmBlocks);
    
    // Add free blocks
    for (Block i = firstBlock; i <= lastBlock; i++) {
        
        if (dev.blocks[i] == nullptr) {
            dev.blocks[i] = new FSBlock(*this, i, FS_EMPTY_BLOCK);
            markAsFree(i);
        }
    }
}

void
FSPartition::info() const
{
    msg("DOS%ld  ",        dos);
    msg("%6ld (x %3ld) ",  numBlocks(), bsize());
    msg("%6ld  ",          usedBlocks());
    msg("%6ld   ",         freeBlocks());
    msg("%3ld%%   ",       (isize)(100.0 * usedBlocks() / numBlocks()));
    msg("%s\n",            getName().c_str());
    msg("\n");
}

void
FSPartition::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("First cylinder");
        os << dec(lowCyl) << std::endl;
        os << tab("Last cylinder");
        os << dec(highCyl) << std::endl;
        os << tab("First block");
        os << dec(firstBlock) << std::endl;
        os << tab("Last block");
        os << dec(lastBlock) << std::endl;
        os << tab("Root block");
        os << dec(rootBlock) << std::endl;
        os << tab("Bitmap blocks");
        for (auto& it : bmBlocks) { os << dec(it) << " "; }
        os << util::tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
    }
}

FSBlockType
FSPartition::predictBlockType(Block nr, const u8 *buffer) const
{
    assert(buffer != nullptr);

    // Only proceed if the block belongs to this partition
    if (nr < firstBlock || nr > lastBlock) return FS_UNKNOWN_BLOCK;
    
    // Is it a boot block?
    if (nr == firstBlock + 0) return FS_BOOT_BLOCK;
    if (nr == firstBlock + 1) return FS_BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FS_BITMAP_BLOCK;
    
    // is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FS_BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize() - 4);

    if (type == 2  && subtype == 1)       return FS_ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FS_USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FS_DATA_BLOCK_OFS;
    } else {
        for (isize i = 0; i < bsize(); i++) if (buffer[i]) return FS_DATA_BLOCK_FFS;
    }
    
    return FS_EMPTY_BLOCK;
}

FSName
FSPartition::getName() const
{
    FSBlock *rb = dev.rootBlockPtr(rootBlock);
    return rb ? rb->getName() : FSName("");
}

void
FSPartition::setName(FSName name)
{
    FSBlock *rb = dev.rootBlockPtr(rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
}

isize
FSPartition::bsize() const
{
    return dev.bsize;
}

isize
FSPartition::numBlocks() const
{
    return numCyls() * dev.numHeads * dev.numSectors;
}

isize
FSPartition::numBytes() const
{
    return numBlocks() * bsize();
}

isize
FSPartition::freeBlocks() const
{
    isize result = 0;
    
    for (isize i = (isize)firstBlock; i <= (isize)lastBlock; i++) {
        if (isFree((Block)i)) result++;
    }

    return result;
}

isize
FSPartition::usedBlocks() const
{
    return numBlocks() - freeBlocks();
}

isize
FSPartition::freeBytes() const
{
    return freeBlocks() * bsize();
}

isize
FSPartition::usedBytes() const
{
    return usedBlocks() * bsize();
}

isize
FSPartition::requiredDataBlocks(isize fileSize) const
{
    // Compute the capacity of a single data block
    isize numBytes = bsize() - (isOFS() ? 24 : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
FSPartition::requiredFileListBlocks(isize fileSize) const
{
    // Compute the required number of data blocks
    isize numBlocks = requiredDataBlocks(fileSize);
    
    // Compute the number of data block references in a single block
    isize numRefs = (bsize() / 4) - 56;

    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;

    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

isize
FSPartition::requiredBlocks(isize fileSize) const
{
    isize numDataBlocks = requiredDataBlocks(fileSize);
    isize numFileListBlocks = requiredFileListBlocks(fileSize);
    
    debug(FS_DEBUG, "Required file header blocks : %d\n",  1);
    debug(FS_DEBUG, "       Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "  Required file list blocks : %ld\n", numFileListBlocks);
    debug(FS_DEBUG, "                Free blocks : %ld\n", freeBlocks());
    
    return 1 + numDataBlocks + numFileListBlocks;
}
 
Block
FSPartition::allocateBlock()
{
    if (Block nr = allocateBlockAbove(rootBlock)) return nr;
    if (Block nr = allocateBlockBelow(rootBlock)) return nr;

    return 0;
}

Block
FSPartition::allocateBlockAbove(Block nr)
{
    assert(nr >= firstBlock && nr <= lastBlock);
    
    for (i64 i = (i64)nr + 1; i <= lastBlock; i++) {
        if (dev.blocks[i]->type == FS_EMPTY_BLOCK) {
            markAsAllocated((Block)i);
            return (Block)i;
        }
    }
    return 0;
}

Block
FSPartition::allocateBlockBelow(Block nr)
{
    assert(nr >= firstBlock && nr <= lastBlock);
    
    for (i64 i = (i64)nr - 1; i >= firstBlock; i--) {
        if (dev.blocks[i]->type == FS_EMPTY_BLOCK) {
            markAsAllocated((Block)i);
            return (Block)i;
        }
    }
    return 0;
}

void
FSPartition::deallocateBlock(Block nr)
{
    assert(nr >= firstBlock && nr <= lastBlock);
    assert(dev.blocks[nr]);
    
    delete dev.blocks[nr];
    dev.blocks[nr] = new FSBlock(*this, nr, FS_EMPTY_BLOCK);
    markAsFree(nr);
}

Block
FSPartition::addFileListBlock(Block head, Block prev)
{
    FSBlock *prevBlock = dev.blockPtr(prev);
    if (!prevBlock) return 0;
    
    Block nr = allocateBlock();
    if (!nr) return 0;
    
    dev.blocks[nr] = new FSBlock(*this, nr, FS_FILELIST_BLOCK);
    dev.blocks[nr]->setFileHeaderRef(head);
    prevBlock->setNextListBlockRef(nr);
    
    return nr;
}

Block
FSPartition::addDataBlock(isize count, Block head, Block prev)
{
    FSBlock *prevBlock = dev.blockPtr(prev);
    if (!prevBlock) return 0;

    Block nr = allocateBlock();
    if (!nr) return 0;

    FSBlock *newBlock;
    if (isOFS()) {
        newBlock = new FSBlock(*this, nr, FS_DATA_BLOCK_OFS);
    } else {
        newBlock = new FSBlock(*this, nr, FS_DATA_BLOCK_FFS);
    }
    
    dev.blocks[nr] = newBlock;
    newBlock->setDataBlockNr((Block)count);
    newBlock->setFileHeaderRef(head);
    prevBlock->setNextDataBlockRef(nr);
    
    return nr;
}


FSBlock *
FSPartition::newUserDirBlock(const string &name)
{
    FSBlock *block = nullptr;
    
    if (Block nr = allocateBlock()) {
    
        block = new FSBlock(*this, nr, FS_USERDIR_BLOCK);
        block->setName(FSName(name));
        dev.blocks[nr] = block;
    }
    
    return block;
}

FSBlock *
FSPartition::newFileHeaderBlock(const string &name)
{
    FSBlock *block = nullptr;
    
    if (Block nr = allocateBlock()) {

        block = new FSBlock(*this, nr, FS_FILEHEADER_BLOCK);
        block->setName(FSName(name));
        dev.blocks[nr] = block;
    }
    
    return block;
}

FSBlock *
FSPartition::bmBlockForBlock(Block nr)
{
    assert(nr >= 2 && (isize)nr < numBlocks());
        
    // Locate the bitmap block
    isize bitsPerBlock = (bsize() - 4) * 8;
    isize bmNr = (nr - 2) / bitsPerBlock;

    if (bmNr >= (isize)bmBlocks.size()) {
        warn("Allocation bit is located in non-existent bitmap block %ld\n", bmNr);
        return nullptr;
    }

    return dev.bitmapBlockPtr(bmBlocks[bmNr]);
}

bool
FSPartition::isFree(Block nr) const
{
    assert(nr >= firstBlock && nr <= lastBlock);
    
    // Translate rel to a relative block index
    nr -= lowCyl;
    
    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    FSBlock *bm = locateAllocationBit(nr, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

void
FSPartition::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;
    
    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data[byte], bit, value);
    }
}

FSBlock *
FSPartition::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    assert(nr >= firstBlock && nr <= lastBlock);

    // Make ref a relative offset
    nr -= firstBlock;

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize() - 4) * 8;
    isize bmNr = nr / bitsPerBlock;
    nr = nr % bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm;
    bm = (bmNr < (isize)bmBlocks.size()) ? dev.bitmapBlockPtr(bmBlocks[bmNr]) : nullptr;
    if (bm == nullptr) {
        warn("Allocation bit is located in non-existent bitmap block %ld\n", bmNr);
        return nullptr;
    }
    
    // Locate the byte position (note: the long word ordering will be reversed)
    isize rByte = nr / 8;
    
    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < bsize());
    
    *byte = rByte;
    *bit = nr % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}

void
FSPartition::makeBootable(BootBlockId id)
{
    assert(dev.blocks[firstBlock + 0]->type == FS_BOOT_BLOCK);
    assert(dev.blocks[firstBlock + 1]->type == FS_BOOT_BLOCK);

    dev.blocks[firstBlock + 0]->writeBootBlock(id, 0);
    dev.blocks[firstBlock + 1]->writeBootBlock(id, 1);
}

void
FSPartition::killVirus()
{
    assert(dev.blocks[firstBlock + 0]->type == FS_BOOT_BLOCK);
    assert(dev.blocks[firstBlock + 1]->type == FS_BOOT_BLOCK);

    auto id = isOFS() ? BB_AMIGADOS_13 : isFFS() ? BB_AMIGADOS_20 : BB_NONE;

    if (id != BB_NONE) {
        dev.blocks[firstBlock + 0]->writeBootBlock(id, 0);
        dev.blocks[firstBlock + 1]->writeBootBlock(id, 1);
    } else {
        std::memset(dev.blocks[firstBlock + 0]->data + 4, 0, bsize() - 4);
        std::memset(dev.blocks[firstBlock + 1]->data, 0, bsize());
    }
}

bool
FSPartition::check(bool strict, FSErrorReport &report) const
{
    assert(firstBlock <= lastBlock);
    
    report.bitmapErrors = 0;
    
    for (Block i = firstBlock; i <= lastBlock; i++) {

        FSBlock *block = dev.blocks[i];
        if (block->type == FS_EMPTY_BLOCK && !isFree((Block)i)) {
            report.bitmapErrors++;
            debug(FS_DEBUG, "Empty block %d is marked as allocated\n", i);
        }
        if (block->type != FS_EMPTY_BLOCK && isFree((Block)i)) {
            report.bitmapErrors++;
            debug(FS_DEBUG, "Non-empty block %d is marked as free\n", i);
        }
    }
 
    return report.bitmapErrors == 0;
}
