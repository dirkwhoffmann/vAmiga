// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockView.h"
#include "FSTypes.h"
#include "FSBlock.h"
#include "FSExtension.h"
#include "BlockVolume.h"
#include <iostream>
#include <ranges>
#include <unordered_set>

namespace vamiga {

class FSCache final : public FSExtension {

    friend struct FSBlock;

private:

    // The underlying volume
    Volume &dev;

    // Cached blocks
    mutable std::unordered_map<Block, std::unique_ptr<FSBlock>> blocks;

    // Dirty blocks
    mutable std::unordered_set<Block> dirty;


    //
    // Initializing
    //

public:

    FSCache(FileSystem &fs, Volume &vol);
    virtual ~FSCache();

    // [[deprecated]] void init(isize capacity, isize bsize = 512);

private:

    void dealloc();


    //
    // Printing debug information
    //

public:

    void dump(std::ostream &os) const;


    //
    // Querying devide properties
    //

public:

    // Returns capacity information
    isize bsize() const { return dev.bsize(); }
    isize capacity() const { return dev.capacity(); }
    isize numBytes() const { return capacity() * bsize(); }

    // Reports usage information
    isize freeBlocks() const { return capacity() - usedBlocks(); }
    isize usedBlocks() const { return (isize)blocks.size(); }
    isize freeBytes() const { return freeBlocks() * bsize(); }
    isize usedBytes() const { return usedBlocks() * bsize(); }
    double fillLevel() const { return capacity() ? double(100) * usedBlocks() / capacity() : 0; }
    bool isEmpty() const { return usedBlocks() == 0; }


    //
    // Accessing blocks
    //

    // Returns an iterator for the block storage
    /*
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::iterator begin() { return blocks.begin(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::iterator end() { return blocks.end(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::const_iterator begin() const { return blocks.begin(); }
    std::unordered_map<Block, std::unique_ptr<FSBlock>>::const_iterator end() const { return blocks.end(); }
    */

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

    // Caches a block (if not already cached)
    FSBlock *cache(Block nr) const noexcept;

    // Returns a pointer to a block with read permissions (maybe null)
    const FSBlock *tryFetch(Block nr) const noexcept;
    const FSBlock *tryFetch(Block nr, FSBlockType type) const noexcept;
    const FSBlock *tryFetch(Block nr, std::vector<FSBlockType> types) const noexcept;

    // Returns a reference to a block with read permissions (may throw)
    const FSBlock &fetch(Block nr) const;
    const FSBlock &fetch(Block nr, FSBlockType type) const;
    const FSBlock &fetch(Block nr, std::vector<FSBlockType> types) const;

    // Returns a pointer to a block with write permissions (maybe null)
    FSBlock *tryModify(Block nr) noexcept;
    FSBlock *tryModify(Block nr, FSBlockType type) noexcept;
    FSBlock *tryModify(Block nr, std::vector<FSBlockType> types) noexcept;

    // Returns a reference to a block with write permissions (may throw)
    FSBlock &modify(Block nr);
    FSBlock &modify(Block nr, FSBlockType type);
    FSBlock &modify(Block nr, std::vector<FSBlockType> types);




    [[deprecated]] FSBlock *read(Block nr) noexcept;
    [[deprecated]] FSBlock *read(Block nr, FSBlockType type) noexcept;
    [[deprecated]] FSBlock *read(Block nr, std::vector<FSBlockType> types) noexcept;
    /*
    const FSBlock *read(Block nr) const noexcept;
    const FSBlock *read(Block nr, FSBlockType type) const noexcept;
    const FSBlock *read(Block nr, std::vector<FSBlockType> types) const noexcept;
    */

    // Returns a reference to a stored block
    [[deprecated]] FSBlock &at(Block nr);
    [[deprecated]] FSBlock &at(Block nr, FSBlockType type);
    [[deprecated]] FSBlock &at(Block nr, std::vector<FSBlockType> types);
    /*
    const FSBlock &at(Block nr) const;
    const FSBlock &at(Block nr, FSBlockType type) const;
    const FSBlock &at(Block nr, std::vector<FSBlockType> types) const;
    */

    // Operator overload
    // FSBlock &operator[](size_t nr) { return at(Block(nr)); }
    const FSBlock &operator[](size_t nr) const { return fetch(Block(nr)); }

    // Wipes out a block (makes it an empty block)
    void erase(Block nr);


    //
    // Caching
    //

    void markAsDirty(Block nr) { dirty.insert(nr); }

    void flush(Block nr);
    void flush();

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
