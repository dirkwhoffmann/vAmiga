// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BITMAP_EXT_BLOCK_H
#define _FS_BITMAP_EXT_BLOCK_H

#include "FSBlock.h"

struct FSBitmapExtBlock : FSBlock {
                    
    FSBitmapExtBlock(FSVolume &ref, u32 nr);
    ~FSBitmapExtBlock();
     
    const char *getDescription() override { return "FSBitmapExtBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() override { return FS_BITMAP_EXT_BLOCK; }
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    void dump() override;

    
    //
    // Block specific methods
    //
};

#endif
