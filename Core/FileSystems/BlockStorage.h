// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FSBlock.h"
#include "CoreObject.h"

namespace vamiga {

class BlockStorage : public CoreObject {

private:

    // Reference to the owner of this storage
    class FileSystem *fs = nullptr;

    // File system capacity in blocks
    isize _capacity {};

    // Size of a single block in bytes
    isize _bsize {};

    // Block storage
    std::unordered_map<usize, FSBlock> blocks;


    //
    // Initializing
    //

public:

    BlockStorage(FileSystem *fs) : fs(fs), _bsize(512) { };
    BlockStorage(FileSystem *fs, isize capacity, isize bsize = 512);
    virtual ~BlockStorage();

    void init(isize capacity, isize bsize = 512);

    void dealloc();


    //
    // Methods from CoreObject
    //

protected:

    const char *objectName() const override { return "BlockStorage"; }
    void _dump(Category category, std::ostream &os) const override;


    //
    // Querying properties
    //

public:
    
    // Gets or sets the block type
    FSBlockType getType(Block nr) const;
    void setType(Block nr, FSBlockType type);

    // Returns the storage capacity in blocks
    isize capacity() const { return _capacity; }

    // Returns the block size in bytes
    isize bsize() const { return _bsize; }

    // Reads a block from the storage
    FSBlock &read(Block nr);
    const FSBlock &read(Block nr) const;

    // Returns a block pointer or null if the block does not exist
    FSBlock *pread(Block nr);
    FSBlock *pread(Block nr, FSBlockType type);
    const FSBlock *pread(Block nr) const;
    const FSBlock *pread(Block nr, FSBlockType type) const;

    // Write a block into the storage
    void write(Block nr, FSBlock *block);
};

}
