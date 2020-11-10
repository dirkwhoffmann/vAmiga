// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_FILE_BLOCK_H
#define _FS_FILE_BLOCK_H

#include "FSBlock.h"

struct FSFileBlock : FSBlock {

    // Maximum number of references stored in this block
    u32 maxDataBlocks = 0;

    // Number of references stored in this block
    u32 numDataBlocks = 0;

    // Reference to the very first data block
    u32 firstDataBlock = 0;

    // Reference to the parent block
    u32 parent = 0;

    // References stored in this block
    u32 *dataBlocks = nullptr;

    // Reference to the next ref table block
    u32 nextTableBlock = 0;

    //
    // Initializing
    //
    
    FSFileBlock(FSVolume &ref, u32 nr);
    ~FSFileBlock();

    void dump() override;
    bool check(bool verbose) override;
    
    u32 getParent() override { return parent; }
    void setParent(u32 parent) override { this->parent = parent; }

    u32 blockListCapacity() override { return maxDataBlocks; }
    u32 blockListSize() override { return numDataBlocks; }
        
    void setNextFileListBlock(u32 next) override { this->nextTableBlock = next; }
    
    class FSDataBlock *addDataBlockDeprecated(); 
    bool addDataBlockRef(u32 ref) override;
    void deleteDataBlockRefs() override;
};

#endif
