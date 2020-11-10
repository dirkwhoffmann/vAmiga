// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_DATA_BLOCK_H
#define _FS_DATA_BLOCK_H

#include "FSBlock.h"

struct FSDataBlock : FSBlock {
      
    // Reference to the file header block
    // u32 fileHeaderBlock = 0;

    // Number of data bytes stored in this block
    // u32 numDataBytes = 0;
    
private:
  
    // Block number (first is 1)
    u32 blockNumber = 1;

    // Reference to the next data block
    u32 next = 0;

    
    //
    // Methods
    //
    
public:

    FSDataBlock(FSVolume &ref, u32 nr, u32 cnt);
    ~FSDataBlock();

    // Methods from Block class
    FSBlockType type() override { return FS_DATA_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;
    
    void setFileHeaderRef(u32 ref) override;
    void setNextDataBlockRef(u32 ref) override;

    // void setDataBlockNr(u32 nr) override;
    size_t addData(const u8 *buffer, size_t size) override;
};

#endif
