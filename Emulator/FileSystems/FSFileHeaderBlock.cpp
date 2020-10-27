// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref) : FSBlock(ref)
{
    memset(dataBlocks, 0, sizeof(dataBlocks));
}

void
FSFileHeaderBlock::dump()
{
    
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
    if (dataBlocks[0]) write32(p + 16, dataBlocks[0]->nr);
    
    // Data block list
    for (int i = 0; i < numBlocks; i++) write32(p+308-4*i, dataBlocks[i]->nr);
    
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
    assert(parent != NULL);
    write32(p + 500, parent->nr);
    
    // Subtype
    p[508] = -3;
        
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
