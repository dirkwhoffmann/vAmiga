// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_FILEHEADER_BLOCK_H
#define _FS_FILEHEADER_BLOCK_H

#include "FSBlock.h"

struct FileHeaderBlock : HashableBlock {
            
    // Name
    FSName name = FSName("");
    
    // Comment
    FSName comment = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();

    // Number of blocks
    u32 numBlocks = 0;
    
    // The data block list
    Block *dataBlocks[71];
    
    // File size in bytes
    u32 fileSize = 0;
    
    // Reference to the parent block
    Block *parent = nullptr;
    
    //
    // Methods
    //
    
    FileHeaderBlock(FSVolume &ref);
    
    // Methods from Block class
    FSBlockType type() override { return FS_FILEHEADER_BLOCK; }
    virtual void dump() override;
    void write(u8 *p) override;

    // Methods from HashableBlock class
    virtual u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }
};

#endif
