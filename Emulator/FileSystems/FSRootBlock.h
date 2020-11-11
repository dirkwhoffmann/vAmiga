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
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();
    
    // Most recent change
    FSTimeStamp modified = FSTimeStamp();
    
    
    //
    // Constructing and destructing
    //
        
    FSRootBlock(FSVolume &ref, u32 nr);
    FSRootBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSRootBlock();

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    const char *getName() override { return name.name; }
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;
    
    u32 hashTableSize() override { return 72; }
};

#endif
