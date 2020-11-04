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
    FSHashTable *hashTable;

    // Protection status bits
    u32 protection = 0;
    
    // Reference to the parent block
    u32 parent = 0;

    // Reference to the next block with the same hash
    u32 next = 0;

    
    //
    // Methods
    //
    
    FSUserDirBlock(FSVolume &ref, u32 nr);
    FSUserDirBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSUserDirBlock();

    
    //
    // Methods from Block class
    //
    
    FSBlockType type() override { return FS_USERDIR_BLOCK; }
    void printName() override;
    void printPath() override;
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;

    FSHashTable *getHashTable() override { return hashTable; }
    bool addHashBlock(FSBlock *block) override { return hashTable->link(block); }
    FSBlock *seek(FSName name) override { return hashTable->seek(name); }

    u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }
    char *getName() override { return name.name; }
    time_t getCreationDate() override { return created.get(); }

    void setNext(u32 ref) override;
    u32 getNext() override { return next; }

    u32 getParent() override { return parent; }
    void setParent(u32 ref) override;
};

#endif
