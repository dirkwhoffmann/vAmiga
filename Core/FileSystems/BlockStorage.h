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
#include "Inspectable.h"

namespace vamiga {

class BlockStorage final : public CoreObject, public Inspectable<Void, BlockStorageStats> {

private:

    // Reference to the owner of this storage
    class FileSystem *fs = nullptr;

    // File system capacity in blocks
    isize capacity {};

    // Size of a single block in bytes
    isize bsize {};

    // Block storage
    std::unordered_map<usize, FSBlock> blocks;


    //
    // Initializing
    //

public:

    BlockStorage(FileSystem *fs) : fs(fs), bsize(512) { };
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
    // Methods from Inspectable
    //

public:

    void cacheStats(BlockStorageStats &result) const override;


    //
    // Querying devide properties
    //

public:

    // Returns capacity information
    isize numBlocks() const { return capacity; }
    isize numBytes() const { return capacity * bsize; }
    isize blockSize() const { return bsize; }

    // Reports usage information
    isize freeBlocks() const { return numBlocks() - usedBlocks(); }
    isize usedBlocks() const { return (isize)blocks.size(); }
    isize freeBytes() const { return freeBlocks() * blockSize(); }
    isize usedBytes() const { return usedBlocks() * blockSize(); }
    double fillLevel() const { return numBlocks() ? double(100) * usedBlocks() / numBlocks() : 0; }
    bool isEmpty() const { return usedBlocks() == 0; }


    //
    // Accessing blocks
    //

    // Checks if a block is empty
    bool isEmpty(Block nr) const;

    // Gets or sets the block type
    FSBlockType getType(Block nr) const;
    void setType(Block nr, FSBlockType type);

    // Returns a block pointer or null if the block does not exist
    FSBlock *read(Block nr) noexcept;
    FSBlock *read(Block nr, FSBlockType type) noexcept;
    FSBlock *read(Block nr, std::vector<FSBlockType> types) noexcept;
    const FSBlock *read(Block nr) const noexcept;
    const FSBlock *read(Block nr, FSBlockType type) const noexcept;
    const FSBlock *read(Block nr, std::vector<FSBlockType> types) const noexcept;

    // Returns a reference to a stored block
    FSBlock &at(Block nr);
    FSBlock &at(Block nr, FSBlockType type);
    FSBlock &at(Block nr, std::vector<FSBlockType> types);
    const FSBlock &at(Block nr) const;
    const FSBlock &at(Block nr, FSBlockType type) const;
    const FSBlock &at(Block nr, std::vector<FSBlockType> types) const;

    // Operator overload
    FSBlock &operator[](size_t nr) { return at(Block(nr)); }
    const FSBlock &operator[](size_t nr) const { return at(Block(nr)); }

    // Wipes out a block (makes it an empty block)
    void erase(Block nr);
};

}
