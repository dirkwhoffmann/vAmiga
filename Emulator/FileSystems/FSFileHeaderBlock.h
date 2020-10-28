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

struct FSFileHeaderBlock : FSBlock {
            
    // Name
    FSName name = FSName("");
    
    // Comment
    FSName comment = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();

    // Number of blocks
    u32 numBlocks = 0;
    
    // The data block list
    u32 dataBlocks[71];
    
    // File size in bytes
    u32 fileSize = 0;
    
    // Reference to the parent block
    u32 parent = 0;
    
    // Reference to the next block with the same hash
    u32 next = 0;

    
    //
    // Methods
    //
    
    FSFileHeaderBlock(FSVolume &ref, u32 nr);
    FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name);

    // Methods from Block class
    FSBlockType type() override { return FS_FILEHEADER_BLOCK; }
    virtual void dump() override;
    bool check() override;

    void write(u8 *p) override;

    virtual u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }

    void link(u32 ref) override;
    u32 nextBlock() override { return next; }

    u32 getParent() override { return parent; }
    void setParent(u32 ref) override;

    // Debugging
    void printPath() override;
};

#endif
