// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_USERDIR_BLOCK_H
#define _FS_USERDIR_BLOCK_H

#include "FSBlock.h"

struct FSUserDirBlock : FSBlock {
                
    FSUserDirBlock(FSVolume &ref, u32 nr);
    FSUserDirBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSUserDirBlock();
    
    //
    // Methods from Block class
    //
    
    FSBlockType type() override  { return FS_USERDIR_BLOCK; }
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, bool strict) override;
    void dump() override;
    u32 checksumLocation() override { return 5; }

    
    //
    // Accessing block items
    //

    u32 getProtectionBits() override           { return get32(-48     );     }
    void setProtectionBits(u32 val) override   {        set32(-48, val);     }

    FSComment getComment() override            { return FSComment(addr(-46)); }
    void setComment(FSComment name) override   { name.write(addr(-46));       }

    FSTime getCreationDate() override          { return FSTime(addr(-23));    }
    void setCreationDate(FSTime t) override    { t.write(addr(-23));          }

    FSName getName() override                  { return FSName(addr(-20));    }
    void setName(FSName name) override         { name.write(addr(-20));       }
    bool isNamed(FSName &other) override       { return getName() == other;   }

    u32 getNextHashRef() override              { return get32(-4     );      }
    void setNextHashRef(u32 ref) override      {        set32(-4, ref);      }

    u32 getParentDirRef() override             { return get32(-3      );     }
    void setParentDirRef(u32 ref) override     {        set32(-3,  ref);     }

    u32 hashTableSize() override { return 72; }
    u32 hashValue() override { return getName().hashValue(); }
};

#endif
