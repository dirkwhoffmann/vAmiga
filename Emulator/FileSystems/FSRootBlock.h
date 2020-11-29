//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_ROOT_BLOCK_H
#define _FS_ROOT_BLOCK_H

#include "FSBlock.h"

struct FSRootBlock : FSBlock {
          
    FSRootBlock(FSDevice &ref, u32 nr);
    // FSRootBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSRootBlock();

    const char *getDescription() override { return "FSRootBlock"; }

    
    //
    // Methods from Block class
    //

    void dump() override;

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    u32 checksumLocation() override { return 5; }
 
    u32 getBmBlockRef(int nr)                    { return get32(nr-49);      }
    void setBmBlockRef(int nr, u32 ref)          {        set32(nr-49, ref); }

    u32 getNextBmExtBlockRef() override          { return get32(-24);        }
    void setNextBmExtBlockRef(u32 ref) override  {        set32(-24, ref);   }
    
    FSTime getModificationDate() override        { return FSTime(addr32(-23)); }
    void setModificationDate(FSTime t) override  { t.write(addr32(-23));       }

    FSName getName() override                    { return FSName(addr32(-20)); }
    void setName(FSName name) override           { name.write(addr32(-20));    }

    FSTime getCreationDate() override            { return FSTime(addr32(-7));  }
    void setCreationDate(FSTime t) override      { t.write(addr32(-7));        }

    bool addBitmapBlockRefs(std::vector<u32> &refs);
    u32 hashTableSize() override { return 72; }
};

#endif
