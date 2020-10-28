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
    FSTimeStamp lastModified = FSTimeStamp();

    // Hash table storing references to other blocks
    FSHashTable hashTable = FSHashTable(volume);
    
    
    //
    // Methods
    //
    
    FSRootBlock(FSVolume &ref, u32 nr);
    FSRootBlock(FSVolume &ref, u32 nr, const char *name);

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    void dump() override;
    bool check() override;

    // Hash tables
    FSHashTable *getHashTable() override { return &hashTable; }
    bool addHashBlock(FSBlock *block) override { return hashTable.link(block); }
    
    FSBlock *seek(FSName name) override { return hashTable.seek(name); }
    
    // Exporting
    void write(u8 *dst) override;

    // Debugging
    void printPath() override;
};

#endif
