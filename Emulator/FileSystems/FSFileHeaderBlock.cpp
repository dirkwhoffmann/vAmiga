// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    memset(dataBlocks, 0, sizeof(dataBlocks));
}

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name) :
FSFileHeaderBlock(ref, nr)
{
    this->name = FSName(name);
}

void
FSFileHeaderBlock::dump()
{
    
}

bool
FSFileHeaderBlock::check()
{
    bool result = true;
    FSBlock *block;
    FSBlockType type;
    
    if (!parent) {
        printf("Reference to parent block is missing.\n");
        result = false;
        goto exit;
    }
    
    block = volume.block(parent);
    if (block == nullptr) {
        printf("Reference to parent block is invalid (%d).\n", parent);
        result = false;
        goto exit;
    }
    
    type = block->type();
    if (type != FS_ROOT_BLOCK && type != FS_USERDIR_BLOCK) {
        printf("Parent block %d has invalid type %d.\n", parent, type);
        result = false;
        goto exit;
    }
    
exit:
    
    return result;
}

void
FSFileHeaderBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    write32(p, 2);
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Number of blocks in file header
    write32(p + 8, numBlocks);

    // First data block
    if (dataBlocks[0]) write32(p + 16, dataBlocks[0]);
    
    // Data block list
    for (int i = 0; i < numBlocks; i++) write32(p+308-4*i, dataBlocks[i]);
    
    // Protection status bits
    write32(p + 320, 0);
    
    // File size
    write32(p + 324, fileSize);
    
    // Comment as BCPL string
    comment.write(p + 328);
    
    // Creation date
    created.write(p + 420);
    
    // Directory name as BCPL string
    name.write(p + 432);
    
    // Next block with same hash
    write32(p + 496, next);

    // Block pointer to parent directory
    write32(p + 500, parent);
    
    // Subtype
    write32(p + 508, (u32)-3);
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

void
FSFileHeaderBlock::link(u32 ref)
{
    // Only proceed if a valid block number is given
    if (!volume.isBlockNumber(ref)) return;
    
    if (next) {
        volume.block(next)->link(ref);
    } else {
        next = ref;
    }
}

void
FSFileHeaderBlock::setParent(u32 ref)
{
    if (volume.isBlockNumber(ref)) parent = ref;
}

void
FSFileHeaderBlock::printPath()
{
    FSBlock *ref = volume.block(getParent());
    if (ref) ref->printPath();
    printf("%s", name.name);
}
