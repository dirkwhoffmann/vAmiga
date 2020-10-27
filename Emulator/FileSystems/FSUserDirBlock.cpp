// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSUserDirBlock::FSUserDirBlock(FSVolume &ref) : FSBlock(ref)
{
    
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, const char *name) : FSBlock(ref)
{
    
}

void
FSUserDirBlock::dump()
{
    
}

void
FSUserDirBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    p[3] = 0x02;
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Hashtable
    hashTable.write(p + 24);

    // Protection status bits
    u32 protection = 0;
    write32(p + 320, protection);
    
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
    p[508] = 2;
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}

void
FSUserDirBlock::link(u32 ref)
{
    // Only proceed if a valid block number is given
    if (!volume.isBlockNumber(ref)) return;
    
    if (next) {
        volume.block(next)->link(ref);
    } else {
        next = ref;
    }
}
