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
  
    // Name
    FSName name = FSName("Empty");
    

    //
    // Constructing and destructing
    //
        
    FSRootBlock(FSVolume &ref, u32 nr);
    FSRootBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSRootBlock();

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;
    
    FSName getName() override;
    void setName(FSName name) override;

    time_t getCreationDate() override;
    void setCreationDate(time_t t) override;

    time_t getModificationDate() override;
    void setModificationDate(time_t t) override;

    u32 hashTableSize() override { return 72; }
};

#endif
