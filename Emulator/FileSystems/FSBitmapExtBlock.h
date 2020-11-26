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

    u32 getNextBmExtBlockRef() override          { return get32(-1);         }
    void setNextBmExtBlockRef(u32 ref) override  {        set32(-1, ref);    }

    u32 getBmBlockRef(int nr)                    { return get32(nr     );    }
    void setBmBlockRef(int nr, u32 ref)          {        set32(nr, ref);    }

    void addBitmapBlockRefs(vector<u32> &refs, std::vector<u32>::iterator &it);

    
    //
    // Block specific methods
    //
};

#endif
