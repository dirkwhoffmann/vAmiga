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
                    
    FSBitmapBlock(FSPartition &p, u32 nr);
    ~FSBitmapBlock();
     
    const char *getDescription() const override { return "FSBitmapBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BITMAP_BLOCK; }
    FSItemType itemType(u32 byte) const override;
    FSError check(u32 pos, u8 *expected, bool strict) const override;
    void dump() const override;
    u32 checksumLocation() const override { return 0; }

    
    //
    // Block specific methods
    //
};

#endif
