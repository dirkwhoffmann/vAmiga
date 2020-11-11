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
    // FSName comment = FSName("");
        
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
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;

    void setNext(u32 ref) override;
    u32 getNext() override { return next; }

    bool matches(FSName &otherName) override { return name == otherName; }
    u32 getSize() override { return fileSize; }

    FSName getName() override;
    void setName(FSName name) override;

    FSComment getComment() override;
    void setComment(FSComment name) override;

    time_t getCreationDate() override;
    void setCreationDate(time_t t) override;
    
    u32 getNextHashRef() override { return read32(data + bsize() - 16); }
    void setNextHashRef(u32 ref) override { write32(data + bsize() - 16, ref); }

    u32 getParentRef() override { return read32(data + bsize() - 12); }
    void setParentRef(u32 ref) override { write32(data + bsize() - 12, ref); }

    u32 getFirstDataBlockRef() override { return read32(data + 16); }
    void setFirstDataBlockRef(u32 ref) override { write32(data + 16, ref); }

    u32 getNextExtensionBlockRef() override { return read32(data + bsize() - 8); }
    void setNextExtensionBlockRef(u32 ref) override { write32(data + bsize() - 8, ref); }

    u32 getDataBlockRef(int nr) { return read32(data + bsize() - (51 + nr) * 4); }
    void setDataBlockRef(int nr, u32 ref) { write32(data + bsize() - (51 + nr) * 4, ref); }

    u32 numDataBlockRefs() override { return read32(data + 8); }
    u32 maxDataBlockRefs() override { return bsize() / 4 - 56; }
    void incDataBlockRefs() override { write32(data + 8, read32(data + 8) + 1); }

    bool addDataBlockRef(u32 ref) override;
    bool addDataBlockRef(u32 first, u32 ref) override;

    size_t addData(const u8 *buffer, size_t size) override;

    u32 hashValue() override { return name.hashValue(); }
};

#endif
