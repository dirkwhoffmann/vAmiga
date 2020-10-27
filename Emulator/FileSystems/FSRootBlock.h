//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_ROOT_BLOCK_H
#define _FS_ROOT_BLOCK_H

#include "FSBlock.h"

struct RootBlock : Block {
  
    // Name
    FSName name = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();
    
    // Most recent change
    FSTimeStamp lastModified = FSTimeStamp();

    // Hash table storing references to other blocks
    FSHashTable hashTable = FSHashTable();
    
    //
    // Methods
    //
    RootBlock(FSVolume &ref);
    RootBlock(FSVolume &ref, const char *name);

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    void write(u8 *dst) override;
};

#endif
