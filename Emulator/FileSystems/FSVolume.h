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

#include "FSBlock.h"
#include "FSEmptyBlock.h"
#include "FSBootBlock.h"
#include "FSRootBlock.h"
#include "FSBitmapBlock.h"
#include "FSUserDirBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSDataBlock.h"
#include "ADFFile.h"

#include <dirent.h>

/* This class provides the basic functionality of the Amiga File Systems OFS
 * and FFS. Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

class FSVolume : AmigaObject {
    
    friend class FSBlock;
    friend class FSEmptyBlock;
    friend class FSBootBlock;
    friend class FSRootBlock;
    friend class FSBitmapBlock;
    friend class FSUserDirBlock;
    friend class FSFileHeaderBlock;
    friend class FSFileListBlock;
    friend class FSDataBlock;
    friend class OFSDataBlock;
    friend class FFSDataBlock;
    friend class FSHashTable;

protected:
        
    // The type of this volume
    FSVolumeType type;

    // Total capacity of this volume in blocks
    u32 capacity;
    
    // Size of a single block in bytes
    u32 bsize;
        
    // Number of data bytes in a single data block
    u32 dsize;
    
    // Block storage
    BlockPtr *blocks;
    
    // The directory where new files and subdirectories are added
    u32 currentDir = 0;

    // Root directory used by the file exporter
    string exportDir = "";
    

    //
    // Factory methods
    //
    
public:

    // Creates a file system from a buffer (usually the data of an ADF)
    static FSVolume *makeWithADF(class ADFFile *adf, FSError *error);

    // Creates a file system from a buffer (DEPRECATED)
    // static FSVolume *make(const u8 *buffer, size_t size, size_t bsize, FSError *error);
    
    // Creates a file system with the contents of a host file system directory
    static FSVolume *make(FSVolumeType type, const char *name, const char *path, u32 capacity);
    static FSVolume *make(FSVolumeType type, const char *name, const char *path);

    
    //
    // Initializing
    //
    
public:

    FSVolume(FSVolumeType type, const char *name, u32 capacity, u32 bsize = 512);
    FSVolume(FSVolumeType type, u32 capacity, u32 bsize = 512);
    ~FSVolume();
    
    // Prints information about this volume
    void info();
    
    // Prints debug information about this volume
    virtual void dump();
        
    // Guesses the type of a block by analyzing its number and data
    FSBlockType guessBlockType(u32 nr, const u8 *buffer);

    
    //
    // Querying file system properties
    //
    
    FSName getName() { return rootBlock()->getName(); }
    FSVolumeType getType() { return type; }
    bool isOFS();
    bool isFFS();
    
    // Returns the volume size in blocks
    u32 getCapacity() { return capacity; }

    // Returns the block size in bytes
    u32 getBlockSize() { return bsize; }

    // Returns the number of bytes that can be stored in a single data block
    u32 getDataBlockCapacity();
    
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
    // Accessing blocks
    //
    
    // Returns the type of a certain block
    FSBlockType blockType(u32 nr);

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(u32 nr, u32 pos);
    
    // Returns the location of the root block and the bitmap block
    u32 rootBlockNr() { return capacity / 2; }
    u32 bitmapBlockNr() { return capacity / 2 + 1; }
    
    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *block(u32 nr);
    FSBootBlock *bootBlock(u32 nr);
    FSRootBlock *rootBlock(u32 nr);
    FSRootBlock *rootBlock() { return rootBlock(rootBlockNr()); }
    FSBitmapBlock *bitmapBlock(u32 nr);
    FSBitmapBlock *bitmapBlock() { return bitmapBlock(bitmapBlockNr()); }
    FSUserDirBlock *userDirBlock(u32 nr);
    FSFileHeaderBlock *fileHeaderBlock(u32 nr);
    FSFileListBlock *fileListBlock(u32 nr);
    FSDataBlock *dataBlock(u32 nr);
    FSBlock *hashableBlock(u32 nr);
    
    
    //
    // Creating and deleting blocks
    //
    
    // Seeks and free block and marks it as allocated
    u32 allocateBlock();

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
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    FSBlock *changeDir(const char *name);

    // Returns the path of a file system item
    string getPath(FSBlock *block);
    string getPath(u32 ref) { return getPath(block(ref)); }
    string getPath() { return getPath(currentDirBlock()); }

    // Seeks an item inside the current directory
    FSBlock *seek(const char *name);
    FSBlock *seekDir(const char *name);
    FSBlock *seekFile(const char *name);

    // Creates a new directory entry
    FSBlock *makeDir(const char *name);
    FSBlock *makeFile(const char *name);
    FSBlock *makeFile(const char *name, const u8 *buffer, size_t size);
    FSBlock *makeFile(const char *name, const char *str);

    // Prints a directory listing
    void printDirectory(bool recursive);
        
    
    //
    // Traversing the file system
    //

    // Collects all references of blocks with the same hash value
    FSError collectHashBlocks(FSBlock *b, list<u32> &list);
    
    // Returns a collections of nodes for all items in the current directory
    FSError collect(u32 ref, list<u32> &list);
    FSError collectRecursive(u32 ref, list<u32> &list);
    
    // Signature of a walker callback pointer
    typedef FSError (FSVolume::*WalkerPtr)(FSBlock *, void *);
        
    // Walks through all files in the current directory or a given directory
    FSError walk(FSBlock *dir, WalkerPtr walker, void *payload, bool recursive);

    // Walker callbacks
    FSError listWalker(FSBlock *block, void *payload);
    FSError exportWalker(FSBlock *block, void *payload);

    
    //
    // Importing and exporting
    //
    
    // Exports the volume to a buffer compatible with the ADF format
    bool importVolume(const u8 *src, size_t size);
    bool importVolume(const u8 *src, size_t size, FSError *error);

    // Imports the volume from a buffer compatible with the ADF format
    bool exportVolume(u8 *dst, size_t size);
    bool exportVolume(u8 *dst, size_t size, FSError *error);

    // Imports a directory from the host file system
    bool importDirectory(const char *path, bool recursive = true);
    bool importDirectory(const char *path, DIR *dir, bool recursive = true);

    // Exports the volume to a directory of the host file system
    FSError exportDirectory(const char *path, bool recursive = true);
};

#endif
