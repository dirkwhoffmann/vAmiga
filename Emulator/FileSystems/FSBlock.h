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

#include "Utils.h"
#include "FSTypes.h"
#include "FSName.h"

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
    
    // Reads or writes a time stamp
    static time_t readTimeStamp(u8 *p);
    static void writeTimeStamp(u8 *p, time_t t);
    
    
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
    char *assemblePath();

    
    //
    // Debugging
    //
    
    // Prints the full path of this block
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
    // Method stubs for blocks representing file items
    //
    
public:
    
    // Return true if the name of this block matches the given name
    virtual bool matches(FSName &otherName) { return false; }
        
    // Returns the size of this block
    virtual u32 getSize() { return 0; }

    
    //
    // Method stubs for blocks maintaining a linked list
    //

public:

    // Links this block with another block with the same hash
    virtual u32 getNext() { return 0; }
    virtual void setNext(u32 next) { }
    
    
    //
    // Method stubs for blocks maintaining a data block list
    //

public:

    virtual u32 blockListCapacity() { return 0; }
    virtual u32 blockListSize() { return 0; }
    virtual bool addDataBlockRef(u32 ref) { return false; }
    virtual bool addDataBlockRef(u32 first, u32 ref) { return false; }
    virtual void deleteDataBlockRefs() { }

    //
    // Working with names and comments
    //
    
    virtual FSName getName() { return FSName(""); }
    virtual void setName(FSName name) { }

    virtual FSComment getComment() { return FSComment(""); }
    virtual void setComment(FSComment name) { }

    
    //
    // Working with date and time
    //
    
    virtual time_t getCreationDate() { return 0; }
    virtual void setCreationDate(time_t t) { }

    virtual time_t getModificationDate() { return 0; }
    virtual void setModificationDate(time_t t) { }

    void dumpDate(time_t t);
    
    
    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    virtual u32 hashTableSize() { return 0; }

    // Returns a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Looks up an item in the hash table
    u32 lookup(int nr);
    FSBlock *lookup(FSName name);

    // Adds a reference to the hash table
    void addToHashTable(u32 ref);
    
    // Checks the integrity of the hash table
    bool checkHashTable(bool verbose);
    
    // Dumps the contents of the hash table for debugging
    void dumpHashTable();

    
    //
    // Chaining blocks
    //

    // Adds a reference to a the parent directory block
    virtual u32 getParentRef() { return 0; }
    virtual void setParentRef(u32 ref) { }
    FSBlock *getParentBlock();
    
    // Adds a reference to a file header block
    virtual u32 getFileHeaderRef() { return 0; }
    virtual void setFileHeaderRef(u32 ref) { }

    // Adds a reference to the next data block
    virtual u32 getNextDataBlockRef() { return 0; }
    virtual void setNextDataBlockRef(u32 ref) { }

    // Returns a reference or a pointer to the next block with the same hash
    virtual u32 getNextHashRef() { return 0; }
    virtual FSBlock *getNextHashBlock();

    // Adds a reference to the next block with the same hash
    virtual void setNextHashRef(u32 ref) { }

    // Adds a reference to the first data block
    virtual u32 getFirstDataBlockRef() { return 0; }
    virtual void setFirstDataBlockRef(u32 ref) { }

    // Returns a reference or a pointer to the next extension block
    virtual u32 getNextExtensionBlockRef() { return 0; }
    class FSFileListBlock *getNextExtensionBlock();

    // Adds a reference to the next extension block
    virtual void setNextExtensionBlockRef(u32 ref) { }

    
    //
    // Data blocks and file data
    //
    
    // Returns the number of data block references in this block
    virtual u32 numDataBlockRefs() { return 0; }

    // Returns the maximum number of storable data block references
    virtual u32 maxDataBlockRefs() { return 0; }
    
    // Returns the maximum number of storable data block references
    virtual void incDataBlockRefs() { }

    // Sets the data block number (first block is numbered 1)
    // virtual void setDataBlockNr(u32 nr) { }
    
    // Adds raw file data to this block
    virtual size_t addData(const u8 *buffer, size_t size) { return 0; }
};

typedef FSBlock* BlockPtr;

#endif
