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
#include <ranges>

namespace vamiga {

class FSStorage final : public CoreObject {

private:

    // Reference to the owner of this storage
    class FileSystem *fs = nullptr;

    // File system capacity in blocks
    isize capacity {};

    // Size of a single block in bytes
    isize bsize {};

    // Block storage
    std::unordered_map<Block, std::unique_ptr<FSBlock>> blocks;


    //
    // Initializing
    //

public:

    FSStorage(FileSystem *fs) : fs(fs), bsize(512) { };
    FSStorage(FileSystem *fs, isize capacity, isize bsize = 512);
    virtual ~FSStorage();

    void init(isize capacity, isize bsize = 512);

    void dealloc();


    //
    // Methods from CoreObject
    //

protected:

    const char *objectName() const override { return "BlockStorage"; }
    void _dump(Category category, std::ostream &os) const override;


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

    // Returns an iterator for the block storage
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::iterator begin() { return blocks.begin(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::iterator end() { return blocks.end(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::const_iterator begin() const { return blocks.begin(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::const_iterator end() const { return blocks.end(); }

    // Returns a view for all keys
    auto keys() const { return std::views::keys(blocks); }

    // Returns a view for all keys in a particular range
    auto keys(Block min, Block max) const {
        
        auto in_range = [=](Block key) { return key >= min && key <= max; };
        return std::views::keys(blocks) | std::views::filter(in_range);
    }

    // Returns a vector with all keys in sorted order
    std::vector<Block> sortedKeys() const;

    // Checks if a block is empty
    bool isEmpty(Block nr) const noexcept;

    // Gets or sets the block type
    FSBlockType getType(Block nr) const noexcept;
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


    //
    // Maintainance
    //

    // Updates the checksums in all blocks
    void updateChecksums() noexcept;


    //
    // GUI helper functions
    //

public:

    // Returns a block summary for creating the block usage image
    void createUsageMap(u8 *buffer, isize len) const;

    // Returns a usage summary for creating the block allocation image
    void createAllocationMap(u8 *buffer, isize len, const FSDiagnosis diagnosis = {}) const;

    // Returns a block summary for creating the diagnose image
    void createHealthMap(u8 *buffer, isize len, const FSDiagnosis diagnosis = {}) const;
};

}
