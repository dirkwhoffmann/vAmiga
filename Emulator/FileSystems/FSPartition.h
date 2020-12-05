// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_PARTITION_H
#define _FS_PARTITION_H

#include "FSTypes.h"
#include "FSDescriptors.h"

struct FSPartition : AmigaObject {
    
    // The device this partition is part of
    class FSDevice &dev;
    
    // File system format
    FSVolumeType dos = FS_NODOS;
    
    // Cylinder boundaries
    u32 lowCyl = 0;
    u32 highCyl = 0;
    
    // Block boundaries
    u32 firstBlock = 0;
    u32 lastBlock = 0;
    
    // Location of the root block
    u32 rootBlock = 0;
    
    // Location of the bitmap blocks and extended bitmap blocks
    vector<u32> bmBlocks;
    vector<u32> bmExtBlocks;

    
    //
    // Factory methods
    //
    
public:

    // Creates a file system with a custom device descriptor
    static FSPartition *makeWithFormat(FSDevice &ref, FSPartitionDescriptor &layout);

    
    //
    // Initializing
    //
    
    FSPartition(FSDevice &ref);
    
    const char *getDescription() override { return "FSPartition"; }

    // Prints a summary about this partition (called by FSDevice::info)
    void info();

    // Prints debug information about this partition
    void dump();

    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(u32 nr, const u8 *buffer);
    
    // Gets or sets the name of this partition
    FSName getName();
    void setName(FSName name);

    
    //
    // Querying partition properties
    //
    
    // Returns the file system category
    bool isOFS() { return isOFSVolumeType(dos); }
    bool isFFS() { return isFFSVolumeType(dos); }

    // Returns the size of a single block in bytes (usually 512)
    u32 bsize();

    // Reports layout information about this partition
    u32 numCyls() { return highCyl - lowCyl + 1; }
    u32 numBlocks();
    u32 numBytes();
    
    // Reports usage information about this partition
    u32 freeBlocks();
    u32 usedBlocks();
    u32 freeBytes();
    u32 usedBytes();
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Returns the number of required blocks to store a file of certain size
    u32 requiredDataBlocks(size_t fileSize);
    u32 requiredFileListBlocks(size_t fileSize);
    u32 requiredBlocks(size_t fileSize);

    // Seeks a free block and marks it as allocated
    u32 allocateBlock();
    u32 allocateBlockAbove(u32 ref);
    u32 allocateBlockBelow(u32 ref);

    // Deallocates a block
    void deallocateBlock(u32 ref);

    // Adds a new block of a certain kind
    u32 addFileListBlock(u32 head, u32 prev);
    u32 addDataBlock(u32 count, u32 head, u32 prev);
    
    // Creates a new block of a certain kind
    FSUserDirBlock *newUserDirBlock(const char *name);
    FSFileHeaderBlock *newFileHeaderBlock(const char *name);
    
    
    //
    // Working with the block allocation bitmap
    //

    // Returns the bitmap block storing the allocation bit for a certain block
    FSBitmapBlock *bmBlockForBlock(u32 relRef);

    // Checks if a block is marked as free in the allocation bitmap
    bool isFree(u32 ref);
    
    // Marks a block as allocated or free
    void markAsAllocated(u32 ref) { setAllocationBit(ref, 0); }
    void markAsFree(u32 ref) { setAllocationBit(ref, 1); }
    void setAllocationBit(u32 ref, bool value);

    
private:
    
    // Locates the allocation bit for a certain block
    FSBitmapBlock *locateAllocationBit(u32 ref, u32 *byte, u32 *bit);
    
    
    //
    // Working with boot blocks
    //
    
public:
    
    // Installs a boot block
    void makeBootable(long id);

    
    //
    // Integrity checking
    //

public:
    
    // Performs several partition checks
    bool check(bool strict, FSErrorReport &report);

    // Checks if the block with the given number is part of this partition
    bool inRange(u32 nr) { return nr >= firstBlock && nr <= lastBlock; }
    
    
    //
    // Importing and exporting
    //
    
public:
    
    // Provides block information that is needed during the import process
    /*
    bool predictBlock(u32 nr, const u8 *buffer,
                      FSPartition **p, FSVolumeType *dos, FSBlockType *type);
    */
};

typedef FSPartition* FSPartitionPtr;

#endif
