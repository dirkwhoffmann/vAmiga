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
    
    // The volume this partition belongs to
    class FSDevice &dev;
    
    // File system type
    // FSVolumeType dos;
    
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
    // Initializing
    //
    
    FSPartition(FSDevice &ref, FSPartitionDescriptor layout);

    const char *getDescription() override { return "FSPartition"; }

    // Prints a summary about this partition (called by FSDevice::info)
    void info();

    // Gets or sets the name of this partition
    FSName getName();
    void setName(FSName name);

    
    //
    // Querying partition properties
    //
    
    // Returns the DOS version (OFS, FFS, etc.)
    FSVolumeType dos();

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
    // Working with bitmap blocks
    //

    // Returns the bitmap block storing the allocation bit for a certain block
    FSBitmapBlock *bmBlockForBlock(u32 relRef);

    // Checks if a block is marked as free in the allocation bitmap
    bool isFree(u32 ref);

private:
    
    // Locates the allocation bit for a certain block
    FSBitmapBlock *locateAllocationBit(u32 ref, u32 *byte, u32 *bit);
};

#endif
