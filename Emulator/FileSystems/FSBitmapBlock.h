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

struct FSBitmapBlock : FSBlock {
            
    // The allocation map
    bool *allocated = nullptr;
        
    FSBitmapBlock(FSVolume &ref, u32 nr);
    
    FSBlockType type() override { return FS_BITMAP_BLOCK; }
    void dump() override;

    // Verifying
    bool check() override;
    
    // Exporting
    void write(u8 *dst) override;

    // Allocates or deallocates a single block
    bool isAllocated(u32 block);
    void alloc(u32 block, bool value);
    void alloc(u32 block) { alloc(block, true); }
    void dealloc(u32 block) { alloc(block, false); }
    void dealloc();
};

#endif
