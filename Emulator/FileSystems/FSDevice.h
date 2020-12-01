// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FSVOLUME_H
#define _FSVOLUME_H

#include "AmigaObject.h"
#include "FSLayout.h"
#include "ADFFile.h"
#include "HDFFile.h"

#include <dirent.h>

/* This class provides the basic functionality of the Amiga File Systems OFS
 * and FFS. Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

class FSDevice : AmigaObject {
    
    friend class FSPartition;
    friend class FSBlock;
    friend class FSEmptyBlock;
    friend class FSBootBlock;
    friend class FSRootBlock;
    friend class FSBitmapBlock;
    friend class FSBitmapExtBlock;
    friend class FSUserDirBlock;
    friend class FSFileHeaderBlock;
    friend class FSFileListBlock;
    friend class FSDataBlock;
    friend class OFSDataBlock;
    friend class FFSDataBlock;
    friend class FSHashTable;

protected:
        
    // The layout of this device
    FSLayout layout;
    
    // The type of this volume (DEPRECATED: MOVE TO PARTITION)
    FSVolumeType type;

    //
    // Physical device properties
    //
    
    // Total capacity of this device in blocks (TODO: Replace by blocks() )
    u32 capacity;
    
    // Number of physical cylinders (TODO: MOVE TO PARTITION TABLE)
    u32 cylinders = 80;

    // Number of physical heads (TODO: MOVE TO PARTITION TABLE)
    u32 heads = 2;
    
    //
    // Logical device properties
    //
    
    // Number of blocks per track (TODO: MOVE TO PARTITION TABLE)
    u32 sectors = 11;
    
    // Number of reserved blocks (TODO: MOVE TO PARTITION TABLE)
    u32 reserved = 2;
    
    // Size of a single block in bytes (TODO: MOVE TO PARTITION TABLE) 
    u32 bsize;
        
    // Number of data bytes in a single data block
    u32 dsize;
    
    // The block storage
    BlockPtr *blocks;
    
    // The partition table
    std::vector<FSPartition> part;
        
    // The currently selected partition
    u32 cp = 0;

    // The currently selected directly
    u32 cd = 0;
    

    //
    // Factory methods
    //
    
public:

    // Creates a file system from an ADF or HDF file
    static FSDevice *makeWithADF(class ADFFile *adf, FSError *error);
    static FSDevice *makeWithHDF(class HDFFile *hdf, FSError *error);

    // Creates a file system from a format description
    static FSDevice *makeWithFormat(DiskType type, DiskDensity density);

    // Creates a file system from a partition table (DEPRECATED)
    static FSDevice *make(PTable &ptable, FSError *error);

    // Creates a file system with a single partition
    static FSDevice *make(FSVolumeType type, u32 cyls, u32 heads, u32 sectors, u32 bsize = 512);

    // Creates a file system with the contents of a host file system directory
    static FSDevice *make(FSVolumeType type, const char *name, const char *path, u32 cylinders, u32 heads, u32 sectors);
    static FSDevice *make(FSVolumeType type, const char *name, const char *path);

    
    //
    // Initializing
    //
    
public:

    // FSDevice(FSVolumeType type, u32 capacity, u32 bsize = 512);
    FSDevice() { }
    // FSDevice(FSVolumeType type, u32 cylinders, u32 heads, u32 sectors, u32 bsize = 512);
    FSDevice(FSLayout &layout);
    ~FSDevice();
    
    const char *getDescription() override { return "FSVolume"; }
    
    // Gets or sets the name of this volume
    FSName getName() { return currentRootBlockPtr()->getName(); }
    void setName(FSName name) { currentRootBlockPtr()->setName(name); }
    
    // Prints information about this volume
    void info();
    
    // Prints debug information about this volume
    virtual void dump();
        
    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(u32 nr, const u8 *buffer);

    
    //
    // Querying file system properties
    //
    
public:
        
    // Returns the type of this volume
    FSVolumeType getType() { return type; } // DEPRECATED. MAKE PART OF PARTITION
    bool isOFS(); // DEPRECATED. MAKE PART OF PARTITION
    bool isFFS(); // DEPRECATED. MAKE PART OF PARTITION
    
    // Returns the device capacity in blocks
    u32 getCapacity() { return capacity; }

    // Returns the number of reserved blocks
    u32 getReserved() { return reserved; }

    // Returns the block size in bytes
    u32 getBlockSize() { return bsize; }

    // Returns storage capacity information about certain blocks
    u32 getDataBlockCapacity();
    u32 bitmapRefsInRootBlock();
    u32 bitmapRefsInBitmapExtensionBlock();

    // Returns certain bitmap block parameters
    u32 getAllocBitsInBitmapBlock();
    u32 requiredBitmapBlocks();
    u32 requiredBitmapExtensionBlocks();
    
    // Reports usage information
    u32 numBlocks() { return getCapacity(); }
    u32 freeBlocks();
    u32 usedBlocks() { return numBlocks() - freeBlocks(); }
    u32 totalBytes() { return numBlocks() * bsize; }
    u32 freeBytes() { return freeBlocks() * bsize; }
    u32 usedBytes() { return usedBlocks() * bsize; }

    
    //
    // Integrity checking
    //

public:
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict);

    // Checks a single byte in a certain block
    FSError check(u32 blockNr, u32 pos, u8 *expected, bool strict);

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(u32 nr) { return nr < capacity; }

    // Checks if the type of a block matches one of the provides types
    FSError checkBlockType(u32, FSBlockType type);
    FSError checkBlockType(u32, FSBlockType type, FSBlockType altType);

    // Checks if a certain block is corrupted
    bool isCorrupted(u32 blockNr) { return getCorrupted(blockNr) != 0; }

    // Returns the position in the corrupted block list (0 = OK)
    u32 getCorrupted(u32 blockNr);

    // Returns the number of the next or previous corrupted block
    u32 nextCorrupted(u32 blockNr);
    u32 prevCorrupted(u32 blockNr);

    // Checks if a certain block is the n-th corrupted block
    bool isCorrupted(u32 blockNr, u32 n);

    // Returns the number of the the n-th corrupted block
    u32 seekCorruptedBlock(u32 n);
    
    
    //
    // Working with partitions
    //
    
    // Returns the number of partitions
    u32 numPartitions() { return layout.part.size(); }
    
    // Returns the partition a certain block belongs to
    u32 partitionForBlock(u32 ref);

    
    //
    // Allocating blocks
    //
    
public:
    
    // Checks if a certain block is marked as allocated or free
    bool isAllocated(u32 ref) { return !isFree(ref); }
    bool isFree(u32 ref);
    
    // Marks a block as allocated or free
    void markAsAllocated(u32 ref) { mark(ref, true); }
    void markAsFree(u32 ref) { mark(ref, false); }
    void mark(u32 ref, bool alloc);
    
private:
    
    // Seeks all bitmap and bitmap extension blocks inside a buffer
    // void locateBitmapBlocks(const u8 *buffer);
    
    // Computes the position of a certain block allocation bit
    bool locateAllocationBit(u32 ref, u32 *block, u32 *byte, u32 *bit);

    
    //
    // Accessing blocks
    //
    
public:
    
    // Returns the type of a certain block
    FSBlockType blockType(u32 nr);

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(u32 nr, u32 pos);
    
    // Returns the location of the root block in the current partition
    u32 currentRootBlockRef() { return layout.part[cp].rootBlock; }
    FSBlock *currentRootBlockPtr() { return block(currentRootBlockRef()); }

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *block(u32 nr);
    FSBootBlock *bootBlock(u32 nr);
    FSRootBlock *rootBlock(u32 nr);
    FSBitmapBlock *bitmapBlock(u32 nr);
    // FSBitmapBlock *bitmapBlock() { return bitmapBlock(bitmapBlockNr()); }
    FSBitmapExtBlock *bitmapExtBlock(u32 nr);
    FSUserDirBlock *userDirBlock(u32 nr);
    FSFileHeaderBlock *fileHeaderBlock(u32 nr);
    FSFileListBlock *fileListBlock(u32 nr);
    FSDataBlock *dataBlock(u32 nr);
    FSBlock *hashableBlock(u32 nr);
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Seeks a free block in the current partition and marks it as allocated
    u32 allocateBlock() { return allocateBlock(layout.part[cp]); }

    // Seeks a free block in a specific partition and marks it as allocated
    u32 allocateBlock(FSPartition &part);
    u32 allocateBlockAbove(FSPartition &part, u32 ref);
    u32 allocateBlockBelow(FSPartition &part, u32 ref);

    // Deallocates a block
    void deallocateBlock(u32 ref);

    // Adds a new block of a certain kind
    u32 addFileListBlock(u32 head, u32 prev);
    u32 addDataBlock(u32 count, u32 head, u32 prev);
    
    // Creates a new block of a certain kind
    FSUserDirBlock *newUserDirBlock(const char *name);
    FSFileHeaderBlock *newFileHeaderBlock(const char *name);
            
    // Installs a boot block
    void installBootBlock();

    // Updates the checksums in all blocks
    void updateChecksums();
    
    
    //
    // Managing directories and files
    //
    
public:
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    FSBlock *changeDir(const char *name);

    // Returns the path of a file system item
    string getPath(FSBlock *block);
    string getPath(u32 ref) { return getPath(block(ref)); }
    string getPath() { return getPath(currentDirBlock()); }

    // Seeks an item inside the current directory
    u32 seekRef(FSName name);
    u32 seekRef(const char *name) { return seekRef(FSName(name)); }
    FSBlock *seek(const char *name) { return block(seekRef(name)); }
    FSBlock *seekDir(const char *name) { return userDirBlock(seekRef(name)); }
    FSBlock *seekFile(const char *name) { return fileHeaderBlock(seekRef(name)); }

    // Adds a reference to the current directory
    void addHashRef(u32 ref);
    void addHashRef(FSBlock *block);
    
    // Creates a new directory entry
    FSBlock *makeDir(const char *name);
    FSBlock *makeFile(const char *name);
    FSBlock *makeFile(const char *name, const u8 *buffer, size_t size);
    FSBlock *makeFile(const char *name, const char *str);

    // Prints a directory listing
    void printDirectory(bool recursive);
        
    
    //
    // Traversing linked lists
    //
    
    // Returns the last element in the list of extension blocks
    FSBlock *lastFileListBlockInChain(u32 start);
    FSBlock *lastFileListBlockInChain(FSBlock *block);

    // Returns the last element in the list of blocks with the same hash
    FSBlock *lastHashBlockInChain(u32 start);
    FSBlock *lastHashBlockInChain(FSBlock *block);

    
    //
    // Traversing the file system
    //
    
public:
    
    // Returns a collections of nodes for all items in the current directory
    FSError collect(u32 ref, std::vector<u32> &list, bool recursive = true);

private:
    
    // Collects all references stored in a hash table
    FSError collectHashedRefs(u32 ref, std::stack<u32> &list, std::set<u32> &visited);

    // Collects all references with the same hash value
    FSError collectRefsWithSameHashValue(u32 ref, std::stack<u32> &list, std::set<u32> &visited);

 
    //
    // Importing and exporting
    //
    
public:
    
    // Reads a single byte from a block
    u8 readByte(u32 block, u32 offset);

    // Imports the volume from a buffer compatible with the ADF format
    bool importVolume(const u8 *src, size_t size);
    bool importVolume(const u8 *src, size_t size, FSError *error);

    // Imports a directory from the host file system
    bool importDirectory(const char *path, bool recursive = true);
    bool importDirectory(const char *path, DIR *dir, bool recursive = true);

    // Exports the volume to a buffer compatible with the ADF format
    bool exportVolume(u8 *dst, size_t size);
    bool exportVolume(u8 *dst, size_t size, FSError *error);

    // Exports a single block or a range of blocks
    bool exportBlock(u32 nr, u8 *dst, size_t size);
    bool exportBlock(u32 nr, u8 *dst, size_t size, FSError *error);
    bool exportBlocks(u32 first, u32 last, u8 *dst, size_t size);
    bool exportBlocks(u32 first, u32 last, u8 *dst, size_t size, FSError *error);

    // Exports the volume to a directory of the host file system
    FSError exportDirectory(const char *path);
};

#endif
