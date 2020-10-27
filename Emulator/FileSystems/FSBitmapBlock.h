// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BITMAP_BLOCK_H
#define _FS_BITMAP_BLOCK_H

#include "FSBlock.h"

struct BitmapBlock : Block {
    
    // Total number of blocks
    long capacity = 0;
        
    // The allocation map
    bool *allocated = nullptr;
        
    BitmapBlock(FSVolume &ref, long cap);
    
    FSBlockType type() override { return FS_BITMAP_BLOCK; }
    virtual void dump() override;
    void write(u8 *dst) override;

    // Allocates or deallocates a single block
    bool isAllocated(int block) { return block >= capacity || allocated[block]; }
    void alloc(int block) { if (block < capacity) allocated[block] = true; }
    void dealloc(int block) { if (block < capacity) allocated[block] = false; }
    void dealloc() { memset(allocated, 0, sizeof(bool) * capacity); }
};

#endif
