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
        
public:

    // Reference to the parent block
    // u32 parent = 0;

    // References stored in this block
    u32 *dataBlocks = nullptr;


    //
    // Initializing
    //
    
    FSFileBlock(FSVolume &ref, u32 nr);
    ~FSFileBlock();

    bool check(bool verbose) override;
    
    u32 numDataBlockRefs() override { return read32(data + 8); }
    u32 maxDataBlockRefs() override { return bsize() / 4 - 56; }
    void incDataBlockRefs() override { write32(data + 8, read32(data + 8) + 1); }

};

#endif
