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

struct FSFileListBlock : FSBlock {
        
    FSFileListBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSFileListBlock();

    
    //
    // Methods from Block class
    //
    
    FSBlockType getType() const override { assert(type == FS_FILELIST_BLOCK); return FS_FILELIST_BLOCK;   }
    
    isize getNumDataBlockRefs() const override    { return get32(2);           }
    void setNumDataBlockRefs(u32 val) override    {           set32(2, val);   }
    void incNumDataBlockRefs() override           {        inc32(2);           }

    Block getFirstDataBlockRef() const override   { return get32(4);           }
    void setFirstDataBlockRef(Block ref) override {        set32(4, ref);      }
    
    Block getFileHeaderRef() const override       { return get32(-3);          }
    void setFileHeaderRef(Block ref) override     {        set32(-3, ref);     }
    
    Block getNextListBlockRef() const override    { return get32(-2);          }
    void setNextListBlockRef(Block ref) override  {        set32(-2, ref);     }
    
    bool addDataBlockRef(Block first, Block ref) override;
};
