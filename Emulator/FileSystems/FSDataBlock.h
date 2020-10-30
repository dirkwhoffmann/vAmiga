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
  
    // Block number (first is 1)
    u32 blockNumber = 1;
    
    // Reference to the file header block
    u32 fileHeaderBlock = 0;

    // Number of header bytes stored in this block
    u32 numHeaderBytes = 0;

    // Maximum number of data bytes stored in this block
    u32 maxDataBytes = 0;

    // Number of data bytes stored in this block
    u32 numDataBytes = 0;

    // Reference to the next data block
    u32 next = 0;
    
    // Data bytes
    u8 *data;
    
    
    //
    // Methods
    //
    
    FSDataBlock(FSVolume &ref, u32 nr);
    ~FSDataBlock();

    // Methods from Block class
    FSBlockType type() override { return FS_DATA_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;

    u32 getNext() override { return next; }
    void setNext(u32 next) override { this->next = next; }

    bool append(u8 byte) override;
    bool append(const u8 *buffer, size_t size) override;
    bool append(const char *buffer, size_t size) override;
};

#endif
