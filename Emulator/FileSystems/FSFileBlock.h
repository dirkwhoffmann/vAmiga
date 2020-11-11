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

private:
    
    // Number of references stored in this block
    // u32 numDataBlocks = 0;

public:
    
    // Reference to the very first data block
    u32 firstDataBlock = 0;

    // Reference to the parent block
    u32 parent = 0;

    // References stored in this block
    u32 *dataBlocks = nullptr;

    // Reference to the next ref table block
    // u32 nextTableBlock = 0;

    //
    // Initializing
    //
    
    FSFileBlock(FSVolume &ref, u32 nr);
    ~FSFileBlock();

    void dump() override;
    bool check(bool verbose) override;
    
    u32 getParent() override { return parent; }
    void setParent(u32 parent) override { this->parent = parent; }

    u32 numDataBlockRefs() override { return read32(data + 8); }
    u32 maxDataBlockRefs() override { return bsize() / 4 - 56; }
    void incDataBlockRefs() override { write32(data + 8, read32(data + 8) + 1); }

};

#endif
