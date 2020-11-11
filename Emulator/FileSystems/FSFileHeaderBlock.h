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
    const char *getName() override { return name.name; }
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;

    void setNext(u32 ref) override;
    u32 getNext() override { return next; }

    bool matches(FSName &otherName) override { return name == otherName; }
    u32 getSize() override { return fileSize; }
    // time_t getCreationDate() override { return created.get(); }

    time_t getCreationDate() override;
    void setCreationDate(time_t t) override;
    
    void setNextDataBlockRef(u32 ref) override;

    u32 getNextHashRef() override { return read32(data + bsize() - 16); }
    void setNextHashRef(u32 ref) override { write32(data + bsize() - 16, ref); }

    size_t addData(const u8 *buffer, size_t size) override;

    u32 hashValue() override { return name.hashValue(); }
};

#endif
