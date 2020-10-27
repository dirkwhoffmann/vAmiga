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
#include "ADFFile.h"

/* This class provides the basic functionality of the Amiga's Original File
 * System (OFS). Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

// THIS IS EXPERIMENTAL CODE AND NOT FULLY FUNCTIONAL YET. STAY TUNED...

class FSVolume : AmigaObject {
    
protected:
    
    friend class Block;
    
    // The type of this volume (only OFS is supported, yet)
    FSVolumeType type = OFS;

    // Total number of blocks in this volume
    u32 capacity;
    
    // Size of a single block in bytes
    u32 bsize = 512;
        
    // The block storage
    BlockPtr *blocks;
    
public:

    // Constructing and destructing
    FSVolume(const char *name, u32 capacity, u32 bsize);
    FSVolume(const char *name) : FSVolume(name, 2 * 880, 512) { };
    ~FSVolume();
    
    // Prints debug information
    void dump();
    
    
    //
    // Querying file system properties
    //
    
    FSVolumeType getType() { return type; }
    bool isOFS() { return type == OFS; }
    bool isFFF() { return type == FFS; }
    u32 getCapacity() { return capacity; }
    u32 getBSize() { return bsize; }
    
    
    //
    // Working with blocks
    //
    
    // Returns a pointer to one of the special blocks
    RootBlock *rootBlock();
    BitmapBlock *bitmapBlock();
    
    // Adds ot removes a block
    void addBlock(long nr, Block *block);
    void removeBlock(long nr);
    
    // Returns the number of a free block (or -1 if no free blocks exist)
    long freeBlock();
    
    // Installs a boot block
    void installBootBlock();

    
    //
    // Managing directories
    //
    
    bool addTopLevelDir(const char *name);
    bool addSubDir(const char *name, UserDirBlock *dir);
    UserDirBlock *seekDirectory(const char *path);
    
    //
    // Exporting
    //
    
    // Write files system to a disk
    void writeAsDisk(u8 *dst, size_t length);
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
