// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IOUtils.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"
#include <climits>
#include <set>
#include <stack>

void
FileSystem::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;

    if (category & dump::Summary) {
        
        auto total = numBlocks();
        auto used = usedBlocks();
        auto free = freeBlocks();
        auto fill = (isize)(100.0 * used / total);
        
        os << "DOS" << dec(dos);
        os << "   ";
        os << std::setw(6) << std::left << std::setfill(' ') << total;
        os << " (x ";
        os << std::setw(3) << std::left << std::setfill(' ') << bsize;
        os << ")  ";
        os << std::setw(6) << std::left << std::setfill(' ') << used;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << free;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << fill;
        os << "%  ";
        os << getName().c_str() << std::endl;
    }
    
    if (category & dump::Partitions) {
        
        os << tab("Root block");
        os << dec(rootBlock) << std::endl;
        os << tab("Bitmap blocks");
        for (auto& it : bmBlocks) { os << dec(it) << " "; }
        os << std::endl;
        os << util::tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
        os << std::endl;
    }

    if (category & dump::Blocks) {
                
        for (isize i = 0; i < numBlocks(); i++)  {
            
            if (blocks[i]->type == FS_EMPTY_BLOCK) continue;
            
            msg("\nBlock %ld (%d):", i, blocks[i]->nr);
            msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type));
            
            blocks[i]->dump();
        }
    }
}

isize
FileSystem::freeBlocks() const
{
    isize result = 0;
    
    for (isize i = 0; i < numBlocks(); i++) {
        if (isFree((Block)i)) result++;
    }

    return result;
}

isize
FileSystem::usedBlocks() const
{
    return numBlocks() - freeBlocks();
}

isize
FileSystem::freeBytes() const
{
    return freeBlocks() * bsize;
}

isize
FileSystem::usedBytes() const
{
    return usedBlocks() * bsize;
}

FSName
FileSystem::getName() const
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getName() : FSName("");
}

FSBlockType
FileSystem::blockType(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
}

FSItemType
FileSystem::itemType(Block nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FileSystem::blockPtr(Block nr) const
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBlock *
FileSystem::bootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BOOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::rootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_ROOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapExtBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_EXT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::userDirBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_USERDIR_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileHeaderBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileListBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILELIST_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::dataBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;

    if (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::hashableBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
    
    if (t == FS_USERDIR_BLOCK || t == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

u8
FileSystem::readByte(Block nr, isize offset) const
{
    assert(offset < bsize);

    if (isize(nr) < numBlocks()) {
        return blocks[nr]->data ? blocks[nr]->data[offset] : 0;
    }
    
    return 0;
}

bool
FileSystem::isFree(Block nr) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    FSBlock *bm = locateAllocationBit(nr, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm;
    bm = (bmNr < (isize)bmBlocks.size()) ? bitmapBlockPtr(bmBlocks[bmNr]) : nullptr;
    if (bm == nullptr) {
        warn("Failed to lookup allocation bit for block %d\n", nr);
        warn("bmNr = %ld\n", bmNr);
        return nullptr;
    }
    
    // Locate the byte position (note: the long word ordering will be reversed)
    nr = nr % bitsPerBlock;
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
    assert(rByte >= 4 && rByte < bsize);
    
    *byte = rByte;
    *bit = nr % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}
