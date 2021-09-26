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

struct FSUserDirBlock : FSBlock {
                
    FSUserDirBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSUserDirBlock();
    
    const char *getDescription() const override { return "FSUserDirBlock"; }

    
    //
    // Methods from Block class
    //
    
    // ErrorCode exportBlock(const string &path) override;
    
    /*
    u32 getProtectionBits() const override     { return get32(-48     );       }
    void setProtectionBits(u32 val) override   {        set32(-48, val);       }
    */
    /*
    FSTime getCreationDate() const override    { return FSTime(addr32(-23));   }
    void setCreationDate(FSTime t) override    { t.write(addr32(-23));         }
    */
    
    Block getNextHashRef() const override      { return get32(-4     );        }
    void setNextHashRef(Block ref) override    {        set32(-4, ref);        }

    /*
    Block getParentDirRef() const override     { return get32(-3      );       }
    void setParentDirRef(Block ref) override   {        set32(-3,  ref);       }
    */
    
    isize hashTableSize() const override       { return 72; }
    u32 hashValue() const override             { return getName().hashValue(); }
};
