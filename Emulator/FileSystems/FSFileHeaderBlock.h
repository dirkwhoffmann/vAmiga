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

struct FSFileHeaderBlock : FSBlock {
                
    FSFileHeaderBlock(FSPartition &p, Block nr, FSBlockType t);

    
    //
    // Methods from Block class
    //

    // ErrorCode exportBlock(const string &path) override;

    /*
    isize getNumDataBlockRefs() const override    { return get32(2);           }
    void setNumDataBlockRefs(u32 val) override    {        set32(2, val);      }
    void incNumDataBlockRefs() override           {        inc32(2);           }
    */
    
    Block getFirstDataBlockRef() const override   { return get32(4     );      }
    void setFirstDataBlockRef(Block ref) override {        set32(4, ref);      }
    
    Block getProtectionBits() const override      { return get32(-48     );    }
    void setProtectionBits(Block val) override    {        set32(-48, val);    }

    u32 getFileSize() const override              { return get32(-47     );    }
    void setFileSize(u32 val) override            {        set32(-47, val);    }

    /*
    FSTime getCreationDate() const override    { return FSTime(addr32(-23));  }
    void setCreationDate(FSTime t) override    { t.write(addr32(-23));        }
    */
    
    Block getNextHashRef() const override        { return get32(-4     );      }
    void setNextHashRef(Block ref) override      {        set32(-4, ref);      }

    Block getParentDirRef() const override       { return get32(-3     );      }
    void setParentDirRef(Block ref) override     {        set32(-3, ref);      }

    Block getNextListBlockRef() const override   { return get32(-2     );      }
    void setNextListBlockRef(Block ref) override {        set32(-2, ref);      }

    
    //
    // Block specific methods
    //
    
    isize writeData(FILE *file);
    // isize addData(const u8 *buffer, isize size) override;

    /*
    bool addDataBlockRef(Block ref);
    bool addDataBlockRef(Block first, Block ref) override;
    */
    
    u32 hashValue() const override { return getName().hashValue(); }
};
