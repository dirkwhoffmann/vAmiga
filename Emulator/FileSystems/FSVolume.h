// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _OFS_H
#define _OFS_H

#include "FSBlock.h"
#include "FSBootBlock.h"
#include "FSRootBlock.h"
#include "FSBitmapBlock.h"
#include "FSUserDirBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSDataBlock.h"
#include "ADFFile.h"

/* This class provides the basic functionality of the Amiga's Original File
 * System (OFS). Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

// THIS IS EXPERIMENTAL CODE AND NOT FULLY FUNCTIONAL YET. STAY TUNED...

class FSVolume : AmigaObject {
    
protected:
    
    friend class FSBlock;
    friend class FSBitmapBlock;
    
    // The type of this volume
    FSVolumeType type = OFS;

    // Total capacity of this volume in blocks
    u32 capacity;
    
    // Size of a single block in bytes
    u32 bsize = 512;
        
    // Block storage
    BlockPtr *blocks;
    
    // The directory where new files and subdirectories are added
    u32 currentDir = 0;

    
    //
    // Class methods
    //
    
public:
    
    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(u32 nr) { return nr < capacity; }

    
    //
    // Initializing
    //
    
public:

    FSVolume(const char *name, u32 capacity, u32 bsize);
    FSVolume(const char *name) : FSVolume(name, 2 * 880, 512) { };
    ~FSVolume();

    // Prints a debug summary for this volume
    virtual void dump();
    
    // Checks the integrity of this volume
    virtual bool check(bool verbose);
    
    // Exports the volume into a buffer compatible with the ADF format
    void writeAsDisk(u8 *dst, size_t length);

    
    //
    // Querying file system properties
    //
    
    FSVolumeType getType() { return type; }
    bool isOFS() { return type == OFS; }
    bool isFFF() { return type == FFS; }
    u32 getCapacity() { return capacity; }
    u32 getBSize() { return bsize; }
    u32 bytesInDataBlock() { return bsize - (isOFS() ? 24 : 0); }
    u32 freeBlocks();

    
    //
    // Accessing blocks
    //
        
    // Returns the location of the root block and the bitmap block
    u32 rootBlockNr() { return 880; }
    u32 bitmapBlockNr() { return 881; }
    
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

    
    //
    // Creating and deleting blocks
    //
    // Allocates a new block (returns 0 if the volume is full)
    u32 allocateBlock();
    u32 allocateBlock(u32 start, int increment);
    u32 allocateAbove(u32 ref) { return allocateBlock(ref, 1); }
    u32 allocateBelow(u32 ref) { return allocateBlock(ref, -1); }

    // Deallocates a block
    void deallocateBlock(u32 ref);
    
    // Creates a new block of a certain kind
    FSUserDirBlock *newUserDirBlock(const char *name);
    FSFileHeaderBlock *newFileHeaderBlock(const char *name);
    FSFileListBlock *newFileListBlock();
    FSDataBlock *newDataBlock();

    // Replaces a block
    // void replaceBlock(long nr, FSBlock *block);

    // Adds ot removes a block (DEPRECATED)
    // void addBlock(long nr, FSBlock *block);
    // void removeBlock(long nr);
            
    // Installs a boot block
    void installBootBlock();

    
    //
    // Managing directories and files
    //
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    bool changeDir(const char *name);
    
    // Creates a new directory entry
    FSUserDirBlock *makeDir(const char *name);
    FSFileHeaderBlock *makeFile(const char *name);
    
    // Seeks an item inside the current directory
    FSBlock *seek(const char *name);
    FSUserDirBlock *seekDir(const char *name);
    FSFileHeaderBlock *seekFile(const char *name);
};

class OFSVolume : public FSVolume {
    
public:
    
    OFSVolume(const char *name);
};

class FFSVolume : public FSVolume {

public:
    
    FFSVolume(const char *name);
};

#endif
