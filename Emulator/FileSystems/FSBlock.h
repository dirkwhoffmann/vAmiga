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
    // Methods
    //
    
    FSBlock(FSVolume &ref); 
    FSBlock(FSVolume &ref, u32 nr) : FSBlock(ref) { this->nr = nr; }
    
    virtual ~FSBlock() { }

    // Returns the type of this block
    virtual FSBlockType type() { return FS_BLOCK; }

    // Prints debug information
    virtual void dump() { };

    // Checks the integrity of this block
    virtual bool check() { return true; }
    
    // Exports this block in AmigaDOS format (512 bytes are written)
    virtual void write(u8 *p);
    
protected:
    
    // Writes a long word into a buffer in Big Endian format
    void write32(u8 *p, u32 value);
};

typedef FSBlock* BlockPtr;


struct HashableBlock : FSBlock {

    // Reference to the next block with the same hash
    HashableBlock *next = nullptr;

    //
    // Methods
    //
    
    HashableBlock(FSVolume &ref) : FSBlock(ref) { }
    
    // Return a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Links this block with another block with the same hash
    virtual void link(HashableBlock *block);

    // Return true if the name of this block matches the given name
    virtual bool matches(FSName &otherName) { return false; }
};
    
typedef HashableBlock* HashableBlockPtr;

#endif
