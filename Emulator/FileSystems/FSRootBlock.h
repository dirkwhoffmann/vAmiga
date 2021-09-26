//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"
#include <vector>

struct FSRootBlock : FSBlock {
          
    FSRootBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSRootBlock();

    
    //
    // Methods from Block class
    //

    /*
    Block getNextBmExtBlockRef() const override   { return get32(-24);         }
    void setNextBmExtBlockRef(Block ref) override {        set32(-24, ref);    }
    */
    /*
    FSTime getModificationDate() const override   { return FSTime(addr32(-23));}
    void setModificationDate(FSTime t) override   { t.write(addr32(-23));      }
    */
    /*
    FSTime getCreationDate() const override       { return FSTime(addr32(-7)); }
    void setCreationDate(FSTime t) override       { t.write(addr32(-7));       }
    */
    
    isize hashTableSize() const override          { return 72;                 }

    bool addBitmapBlockRefs(std::vector<Block> &refs);
};
