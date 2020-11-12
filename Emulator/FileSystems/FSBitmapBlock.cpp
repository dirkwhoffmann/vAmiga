// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSBitmapBlock::FSBitmapBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
    
    allocated = new bool[volume.capacity]();
    
    // Mark all blocks except the first two as free
    for (int i = 2; i < volume.capacity; i++) alloc(i);
    
    // The first two blocks are always allocated
    allocated[0] = true;
    allocated[1] = true;    
}

FSBitmapBlock::~FSBitmapBlock()
{
    delete [] data;
}

void
FSBitmapBlock::dump()
{
    printf("   Allocated: ");

    for (int i = 0; i < volume.capacity; i++) {
        if (allocated[i]) printf("%d ", i);
    }
    
    printf("\n");
}

bool
FSBitmapBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    for (u32 i = 2; i < volume.capacity; i++) {
                
        FSBlockType type = volume.blocks[i]->type();

        if (type == FS_EMPTY_BLOCK && isAllocated(i)) {
            if (verbose) printf("Empty block %d is marked as allocated.\n", i);
            result = false;
        }
        if (type != FS_EMPTY_BLOCK && !isAllocated(i)) {
            if (verbose) printf("Non-empty block %d is marked as free.\n", i);
            result = false;
        }
    }

    return result;
}

void
FSBitmapBlock::locateBlockBit(u32 nr, int *byte, int *bit)
{
    // The first two blocks are not part the map (they are always allocated)
    assert(nr >= 2);
    nr -= 2;
    
    // Compute the location (the long word ordering of 'byte' is inversed)
    *bit = nr % 8;
    *byte = nr / 8;

    // Rectifiy the ordering
    switch (*byte % 4) {
        case 0: *byte += 3; break;
        case 1: *byte += 1; break;
        case 2: *byte -= 1; break;
        case 3: *byte -= 3; break;
    }
}

void
FSBitmapBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(p);
    assert(volume.bsize == bsize);

    // Start from scratch
    memset(p, 0, bsize);

    // Write allocation map
    for (long i = 2; i < volume.capacity; i++) {

        if (allocated[i]) continue;
        
        // Determine bit position for this block inside the bitmap. Layout:
        //
        //     Position: p[00] p[01] p[02] ... p[31] p[32] p[33] ... p[63]
        //       Sector:   29    28    27         2    61    60        34
        //
        // Remember: The first two sectors are always allocated and not part
        // the map.

        long bit = (i - 2) % 8;
        long byte = (i - 2) / 8;
        switch (byte % 4) {
            case 0: byte += 3; break;
            case 1: byte += 1; break;
            case 2: byte -= 1; break;
            case 3: byte -= 3; break;
        }
        
        SET_BIT(p[4 + byte], bit);
    }
    
    for (int i = 0; i < bsize; i++) {
        assert(p[i] == data[i]);
    }
    
    // Compute checksum
    write32(p, FSBlock::checksum(p));
}

bool
FSBitmapBlock::isAllocated(u32 block)
{
    // The first two blocks are always allocated
    if (block < 2) return true;
    
    // Consider non-existing blocks as allocated, too
    if (!volume.isBlockNumber(block)) return true;

    int byte, bit;
    locateBlockBit(block, &byte, &bit);
    assert(byte <= bsize() - 4);
    assert(bit <= 7);

    assert(GET_BIT(data[byte + 4], bit) != allocated[block]);
    return allocated[block];
}

void
FSBitmapBlock::alloc(u32 block, bool value)
{
    if (!volume.isBlockNumber(block)) return;

    int byte, bit;
    locateBlockBit(block, &byte, &bit);
    assert(byte <= bsize() - 4);
    assert(bit <= 7);
    
    // 0 = allocated, 1 = not allocated
    value ? CLR_BIT(data[4 + byte], bit) : SET_BIT(data[4 + byte], bit);
    
    allocated[block] = value;
}

void
FSBitmapBlock::dealloc()
{
    memset(allocated, 0, sizeof(bool) * volume.capacity);
}
