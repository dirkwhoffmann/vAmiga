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
    
    
    //
    // Initializing
    //
    
    FSBlock(FSVolume &ref, u32 nr) : volume(ref) { this->nr = nr; }
    
    virtual ~FSBlock() { }

    // Returns the type of this block
    virtual FSBlockType type() { return FS_EMPTY_BLOCK; }

    
    //
    // Method section 1: Implemented by blocks that maintain a hash table
    //
    
    // Returns a reference to the hash table
    virtual FSHashTable *getHashTable() { return nullptr; }

    // Adds a new item to the hash table
    virtual bool addHashBlock(FSBlock *block) { return false; }

    
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
    virtual void link(u32 other) { };

    // Returns the next element in the linked list or 0
    virtual u32 nextBlock() { return 0; }
    
    
    //
    // Method section 4: Implemented by blocks that are part of a hierachy
    //

    virtual u32 getParent() { return 0; }
    virtual void setParent(u32 parent) { }

    
    //
    // Seeking files
    //

    // Seeks for a reference to a named file inside this block
    virtual FSBlock *seek(FSName name) { return nullptr; }


    //
    // Verifying
    //

    // Checks the integrity of this block
    virtual bool check() { return true; }
    
        
    //
    // Exporting
    //
    
    // Exports this block in AmigaDOS format (512 bytes are written)
    virtual void write(u8 *p);
    
    // Writes a long word into a buffer in Big Endian format
    void write32(u8 *p, u32 value);

    
    //
    // Debugging
    //
    
    // Prints debug information
    virtual void dump() { };
    
    // Prints the path if this block represents a file system item
    virtual void printPath() { };
};

typedef FSBlock* BlockPtr;

    
#endif
