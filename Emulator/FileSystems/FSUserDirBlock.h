// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_USERDIR_BLOCK_H
#define _FS_USERDIR_BLOCK_H

#include "FSBlock.h"

struct FSUserDirBlock : FSBlock {
            
    // Name
    FSName name = FSName("");
    
    // Comment
    FSName comment = FSName("");
    
    // Protection status bits
    u32 protection = 0;
    
    // Reference to the parent block
    u32 parent = 0;

    // Reference to the next block with the same hash
    u32 next = 0;

    
    //
    // Methods
    //
    
    FSUserDirBlock(FSVolume &ref, u32 nr);
    FSUserDirBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSUserDirBlock();

    
    //
    // Methods from Block class
    //
    
    FSBlockType type() override { return FS_USERDIR_BLOCK; }
    const char *getNameDeprecated() override { return name.name; };
    void dump() override;
    bool check(bool verbose) override;
    void exportBlock(u8 *p, size_t size) override;
    
    bool matches(FSName &otherName) override { return name == otherName; }
    // time_t getCreationDate() override { return created.get(); }

    FSName getName() override;
    void setName(FSName name) override;

    FSName getComment() override;
    void setComment(FSName name) override;

    time_t getCreationDate() override;
    void setCreationDate(time_t t) override;

    
    void setNext(u32 ref) override;
    u32 getNext() override { return next; }

    u32 getParent() override { return parent; }
    void setParent(u32 ref) override;

    u32 hashTableSize() override { return 72; }
    u32 hashValue() override { return name.hashValue(); }

    u32 getNextHashRef() override { return read32(data + bsize() - 16); }
    void setNextHashRef(u32 ref) override { write32(data + bsize() - 16, ref); }
};

#endif
