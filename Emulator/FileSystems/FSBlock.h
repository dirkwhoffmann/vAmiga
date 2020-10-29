// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BLOCKS_H
#define _FS_BLOCKS_H

#include <stdio.h>
#include "FSTypes.h"
#include "FSName.h"
#include "FSTimeStamp.h"
#include "FSHashTable.h"

struct FSBlock {
    
    // The volume this block belongs to
    class FSVolume &volume;
    
    // The sector number of this block
    u32 nr;
    

    //
    // Static methods
    //

    // Computes a checksum for the sector in the provided buffer
    static u32 checksum(u8 *p);
    
    // Writes a long word into a buffer in Big Endian format
    static void write32(u8 *p, u32 value);

    
    //
    // Methods
    //
    
    FSBlock(FSVolume &ref, u32 nr) : volume(ref) { this->nr = nr; }
    virtual ~FSBlock() { }

    // Returns the type of this block
    virtual FSBlockType type() { return FS_EMPTY_BLOCK; }

    // Prints the name or path of this block
    virtual void printName() { };
    virtual void printPath() { };

    // Prints a debug summary for this block
    virtual void dump() { };
    
    // Checks the integrity of this block
    virtual bool check(bool verbose);

    // Exports this block in AmigaDOS format
    virtual void write(u8 *p);
    
    
    //
    // Method section 1: Implemented by blocks that maintain a hash table
    //
    
    // Returns a reference to the hash table
    virtual FSHashTable *getHashTable() { return nullptr; }

    // Adds a new item to the hash table
    virtual bool addHashBlock(FSBlock *block) { return false; }

    // Looks for a matching item inside the hash table
    virtual FSBlock *seek(FSName name) { return nullptr; }

    
    //
    // Method section 2: Implemented by blocks that are hashable
    //

    // Return a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Return true if the name of this block matches the given name
    virtual bool matches(FSName &otherName) { return false; }

    
    //
    // Method section 3: Implemented by blocks that maintain a linked list
    //

    // Links this block with another block with the same hash
    virtual u32 getNext() { return 0; }
    virtual void setNext(u32 next) { }

    
    //
    // Method section 4: Implemented by blocks that are vertically grouped
    //

    virtual u32 getParent() { return 0; }
    virtual void setParent(u32 parent) { }

    
    //
    // Method section 5: Implemented by blocks that maintain a data block list
    //

    virtual u32 blockListCapacity() { return 0; }
    virtual u32 blockListSize() { return 0; }
    virtual bool addDataBlockRef(u32 ref) { return false; }
    virtual void deleteDataBlockRefs() { }

protected:
    
    // Performs a certain integrity check on a block reference
    bool assertNotNull(u32 ref, bool verbose);
    bool assertInRange(u32 ref, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, FSBlockType optType, bool verbose);
    bool assertSelfRef(u32 ref, bool verbose);
};

typedef FSBlock* BlockPtr;

#endif
