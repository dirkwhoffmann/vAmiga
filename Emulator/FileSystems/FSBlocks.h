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

#include "FSName.h"
#include "FSTimeStamp.h"
#include "FSHashTable.h"

struct Block {
    
    long nr;
    
    // Computes a checksum for the sector in the provided buffer
    static u32 checksum(u8 *p);

    virtual ~Block() { printf("~Block()\n"); }
    
    virtual bool isBootBlock() { return false; }
    virtual bool isRootBlock() { return false; }
    virtual bool isBitmapBlock() { return false; }
    virtual bool isUserDirBlock() { return false; }
    virtual bool isFileHeaderBlock() { return false; }
    virtual bool isFileListBlock() { return false; }
    virtual bool isDataBlock() { return false; }

    virtual void dump() { };

    virtual void write(u8 *ptr) { };
};

typedef Block* BlockPtr;

struct HashableBlock : Block {

    // Reference to the next block with the same hash
    HashableBlock *next = nullptr;

    // Links this block with another block with the same hash
    void link(HashableBlock *block);
    
    // Return the hash value for this block
    virtual u32 hashValue() = 0;
};
    
typedef HashableBlock* HashableBlockPtr;

struct BootBlock : Block {
  
    bool ffs;
    
    BootBlock(bool ffs = false) { this->ffs = ffs; }
    
    virtual bool isBootBlock() override { return true; }
    void write(u8 *dst) override;
};

struct RootBlock : Block {
  
    // Volume name
    FSName name = FSName("");

    // Creation date
    FSTimeStamp created = FSTimeStamp();

    // Most recent change
    FSTimeStamp lastModified = FSTimeStamp();

    // Hash table storing references to other blocks
    FSHashTable hashTable = FSHashTable();
    
    RootBlock(const char *str) { name = FSName(str); }

    virtual bool isRootBlock() override { return true; }
    void write(u8 *dst) override;
};

struct BitmapBlock : Block {
    
    // Total number of blocks
    long capacity = 0;
        
    // The allocation map
    bool *allocated = nullptr;
        
    BitmapBlock(long cap);
    virtual bool isBitmapBlock() override { return true; }
    virtual void dump() override;
    void write(u8 *dst) override;

    // Allocates or deallocates a single block
    bool isAllocated(int block) { return block >= capacity || allocated[block]; }
    void alloc(int block) { if (block < capacity) allocated[block] = true; }
    void dealloc(int block) { if (block < capacity) allocated[block] = false; }
    void dealloc() { memset(allocated, 0, sizeof(bool) * capacity); }
};

struct UserDirBlock : HashableBlock {
    
    FSName name = FSName("");
    FSTimeStamp date = FSTimeStamp();
        
    UserDirBlock(FSName name);
    
    virtual bool isUserDirBlock() override { return true; }
    virtual void dump() override;
    void write(u8 *dst) override;
    virtual u32 hashValue() override { return name.hash(); }
};

#endif
