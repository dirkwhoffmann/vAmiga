// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"
#include <vector>

struct FSBitmapExtBlock : FSBlock {
                    
    FSBitmapExtBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSBitmapExtBlock();
     
    
    //
    // Methods from Block class
    //

    FSBlockType getType() const override { assert(type == FS_BITMAP_EXT_BLOCK); return FS_BITMAP_EXT_BLOCK; }

    Block getNextBmExtBlockRef() const override   { return get32(-1);         }
    void setNextBmExtBlockRef(Block ref) override {        set32(-1, ref);    }
    
    void addBitmapBlockRefs(std::vector<Block> &refs, std::vector<Block>::iterator &it);
};
