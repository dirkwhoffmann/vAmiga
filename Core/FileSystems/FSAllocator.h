// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSExtension.h"

namespace vamiga {

class FSAllocator final : public FSExtension {

public:

    // Allocation pointer (selects the block to allocate next)
    Block ap = 0;

    using FSExtension::FSExtension;


    //
    // Computing block counts
    //

public:
    
    // Returns the number of required blocks to store a file of certain size
    [[nodiscard]] isize requiredBlocks(isize fileSize) const noexcept;

private:

    // Returns the number of required file list or data blocks
    [[nodiscard]] isize requiredFileListBlocks(isize fileSize) const noexcept;
    [[nodiscard]] isize requiredDataBlocks(isize fileSize) const noexcept;


    //
    // Creating and deleting blocks
    //

public:

    // Returns true if at least 'count' free blocks are available
    [[nodiscard]] bool allocatable(isize count) const noexcept;

    // Seeks a free block and marks it as allocated
    Block allocate();

    // Allocates multiple blocks
    void allocate(isize count, std::vector<Block> &result, std::vector<Block> prealloc = {});

    // Deallocates a block
    void deallocateBlock(Block nr);

    // Allocates multiple blocks
    void deallocateBlocks(const std::vector<Block> &nrs);

    // Allocates all blocks needed for a file
    void allocateFileBlocks(isize bytes,
                            std::vector<Block> &listBlocks, std::vector<Block> &dataBlocks);

    //
    // Managing the block allocation bitmap
    //

public:

    // Checks if a block is allocated or unallocated
    [[nodiscard]] bool isUnallocated(Block nr) const noexcept;
    [[nodiscard]] bool isAllocated(Block nr) const noexcept { return !isUnallocated(nr); }

    // Returns the number of allocated or unallocated blocks
    [[nodiscard]] isize numUnallocated() const noexcept;
    [[nodiscard]] isize numAllocated() const noexcept;

    // Marks a block as allocated or free
    void markAsAllocated(Block nr) { setAllocationBit(nr, 0); }
    void markAsFree(Block nr) { setAllocationBit(nr, 1); }
    void setAllocationBit(Block nr, bool value);

private:

    // Locates the allocation bit for a certain block
    FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) noexcept;
    const FSBlock *locateAllocationBit(Block nr, isize *byte, isize *bit) const noexcept;

    // Translate the bitmap into to a vector with the n-th bit set iff the n-th block is free
    std::vector<u32> serializeBitmap() const;
};

}
