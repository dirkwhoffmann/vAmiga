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
          
    FSRootBlock(FSVolume &ref, u32 nr);
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

    FSTime getCreationDate() override            { return FSTime(addr(-7));  }
    void setCreationDate(FSTime t) override      { t.write(addr(-7));        }

    FSName getName() override                    { return FSName(addr(-20)); }
    void setName(FSName name) override           { name.write(addr(-20));    }

    FSTime getModificationDate() override        { return FSTime(addr(-23)); }
    void setModificationDate(FSTime t) override  { t.write(addr(-23));       }

    bool addBitmapBlockRef(u32 ref) override;
    u32 hashTableSize() override { return 72; }
};

#endif
