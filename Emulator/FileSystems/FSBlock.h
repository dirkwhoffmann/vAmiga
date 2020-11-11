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

    // The block data
    u8 *data = nullptr;
    
    
    //
    // Static variables and methods
    //

    // Search limit for avoiding infinite loops in list walks
    static const long searchLimit = 255;
    
    // Computes a checksum for the sector in the provided buffer
    static u32 checksum(u8 *p);
    
    // Reads or writes a long word in Big Endian format
    static u32 read32(u8 *p);
    static void write32(u8 *p, u32 value);
    
    
    //
    // Constructing and destructing
    //
    
    FSBlock(FSVolume &ref, u32 nr) : volume(ref) { this->nr = nr; }
    virtual ~FSBlock() { }

    
    //
    // Accessing block properties
    //

    // Returns the type of this block
    virtual FSBlockType type() = 0; 

    // Returns the size of this block
    u32 bsize();
    
    // Returns the name or path of this block
    virtual const char *getName() { return ""; }
    char *assemblePath();

    
    //
    // Debugging
    //
    
    // Prints the name or path of this block
    void printName();
    void printPath();

    // Prints a debug summary for this block
    virtual void dump() { };
    
    
    //
    // Verifying
    //
    
    // Checks the integrity of this block
    virtual bool check(bool verbose);

protected:
    
    // Performs a certain integrity check on a block reference
    bool assertNotNull(u32 ref, bool verbose);
    bool assertInRange(u32 ref, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, FSBlockType optType, bool verbose);
    bool assertSelfRef(u32 ref, bool verbose);

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    virtual void importBlock(u8 *p, size_t bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    virtual void exportBlock(u8 *p, size_t bsize);

    
    //
    // Method stubs for blocks maintaining a hash table
    //
    
public:
    
    // Returns a reference to the hash table
    virtual FSHashTable *getHashTable() { return nullptr; }

    // Adds a new item to the hash table
    virtual bool addHashBlock(FSBlock *block) { return false; }

    // Looks for a matching item inside the hash table
    // virtual FSBlock *seek(FSName name) { return nullptr; }

    
    //
    // Method stubs for blocks representing file items
    //
    
public:
    
    // Return true if the name of this block matches the given name
    virtual bool matches(FSName &otherName) { return false; }
        
    // Returns the size of this block
    virtual u32 getSize() { return 0; }

    // Returns the creation date of this block
    virtual time_t getCreationDate() { return 0; }

    
    //
    // Method stubs for blocks maintaining a linked list
    //

public:

    // Links this block with another block with the same hash
    virtual u32 getNext() { return 0; }
    virtual void setNext(u32 next) { }

    
    //
    // Method stubs for blocks that are vertically grouped
    //

public:

    virtual u32 getParent() { return 0; }
    virtual void setParent(u32 parent) { }

    
    //
    // Method stubs for blocks maintaining a data block list
    //

public:

    virtual u32 blockListCapacity() { return 0; }
    virtual u32 blockListSize() { return 0; }
    virtual void setNextFileListBlock(u32 next) { }
    virtual bool addDataBlockRef(u32 ref) { return false; }
    virtual void deleteDataBlockRefs() { }

    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    virtual u32 hashTableSize() { return 0; }

    // Returns a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Looks up an item in the hash table
    virtual FSBlock *lookup(FSName name);
    
    // Adds a reference to the hash table
    virtual void addToHashTable(u32 ref);
    
    // Dumps the contents of the hash table for debugging
    virtual void dumpHashTable();

    
    //
    // Chaining blocks
    //
    
    // Adds a reference to a file header block
    virtual void setFileHeaderRef(u32 ref) { }

    // Adds a reference to the next data block
    virtual void setNextDataBlockRef(u32 ref) { }

    // Returns a reference to the next block with the same hash
    virtual u32 getNextHashRef() { return 0; }
    virtual FSBlock *getNextHashBlock();
    
    // Adds a reference to the next block with the same hash
    virtual void setNextHashRef(u32 ref) { }

    
    //
    // Data blocks and file data
    //
    
    // Sets the data block number (first block is numbered 1)
    // virtual void setDataBlockNr(u32 nr) { }
    
    // Adds raw file data to this block
    virtual size_t addData(const u8 *buffer, size_t size) { return 0; }
};

typedef FSBlock* BlockPtr;

#endif
