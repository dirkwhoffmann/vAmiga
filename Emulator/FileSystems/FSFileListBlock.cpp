// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileListBlock::FSFileListBlock(FSVolume &ref, u32 nr) : FSFileBlock(ref, nr)
{
    
}

void
FSFileListBlock::dump()
{
    FSFileBlock::dump();
}

bool
FSFileListBlock::check(bool verbose)
{
    return FSFileBlock::check(verbose);
}

void
FSFileListBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);
    
    // Type
    write32(p, 16);
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Number of data block references
    write32(p + 8, numDataBlocks);
    
    // First data block
    write32(p + 16, firstDataBlock);
    
    // Data block list
    u8 *end = p + bsize - 51 * 4;
    for (int i = 0; i < numDataBlocks; i++) write32(end - 4 * i, dataBlocks[i]);
            
    // Block pointer to parent directory
    write32(p + bsize - 3 * 4, parent);
    
    // Block pointer to first extension block
    write32(p + bsize - 2 * 4, nextTableBlock);
    
    // Subtype
    write32(p + bsize - 1 * 4, (u32)-3);
    
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}
