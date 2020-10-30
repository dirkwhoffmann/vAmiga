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

#include "FSFileBlock.h"

struct FSFileHeaderBlock : FSFileBlock {
            
    // Name
    FSName name = FSName("");
    
    // Comment
    FSName comment = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();
    
    // Protection status bits
    u32 protection = 0;

    // File size in bytes
    u32 fileSize = 0;
    
    // Reference to the next block with the same hash
    u32 next = 0;

    
    //
    // Methods
    //
    
    FSFileHeaderBlock(FSVolume &ref, u32 nr);
    FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name);

    // Methods from Block class
    FSBlockType type() override { return FS_FILEHEADER_BLOCK; }
    void printName() override;
    void printPath() override;
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;

    void setNext(u32 ref) override;
    u32 getNext() override { return next; }

    u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }
    
    // Append data bytes
    bool append(const u8 *buffer, size_t size) override;
    bool append(const char *string) override;
};

#endif
