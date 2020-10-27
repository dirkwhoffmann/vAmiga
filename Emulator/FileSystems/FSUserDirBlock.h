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
            
    // Name
    FSName name = FSName("");
    
    // Comment
    FSName comment = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();

    // Hash table storing references to other blocks
    FSHashTable hashTable = FSHashTable(volume);

    // Reference to the parent block
    FSBlock *parent = nullptr;

    // Reference to the next block with the same hash
    u32 next = 0;

    
    //
    // Methods
    //
    
    FSUserDirBlock(FSVolume &ref);
    FSUserDirBlock(FSVolume &ref, const char *name);

    // Methods from Block class
    FSBlockType type() override { return FS_USERDIR_BLOCK; }
    virtual void dump() override;
    void write(u8 *dst) override;
    virtual u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }

    void link(u32 ref) override;
    u32 nextBlock() override { return next; }

};

#endif
