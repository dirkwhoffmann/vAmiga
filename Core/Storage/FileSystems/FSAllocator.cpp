// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystem.h"
#include "utl/support.h"
#include <bit>

namespace vamiga {

isize
FSAllocator::requiredDataBlocks(isize fileSize) const noexcept
{
    // Compute the capacity of a single data block
    isize numBytes = traits.bsize - (traits.ofs() ? 24 : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
FSAllocator::requiredFileListBlocks(isize fileSize) const noexcept
{
    // Compute the required number of data blocks
    isize numBlocks = requiredDataBlocks(fileSize);

    // Compute the number of data block references in a single block
    isize numRefs = (traits.bsize / 4) - 56;

    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;

    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

isize
FSAllocator::requiredBlocks(isize fileSize) const noexcept
{
    isize numDataBlocks = requiredDataBlocks(fileSize);
    isize numFileListBlocks = requiredFileListBlocks(fileSize);

    debug(FS_DEBUG, "Required file header blocks : %d\n",  1);
    debug(FS_DEBUG, "       Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "  Required file list blocks : %ld\n", numFileListBlocks);

    return 1 + numDataBlocks + numFileListBlocks;
}

bool
FSAllocator::allocatable(isize count) const noexcept
{
    Block i = ap;
    isize capacity = fs.blocks();

    while (count > 0) {

        if (cache.getType(Block(i)) == FSBlockType::EMPTY) {
            if (--count == 0) break;
        }

        i = (i + 1) % capacity;
        if (i == ap) return false;
    }

    return true;
}

Block
FSAllocator::allocate()
{
    auto numBlocks = fs.blocks();
    Block i = ap;

    while (!fs.isEmpty(i)) {

        if ((i = (i + 1) % numBlocks) == ap) {

            debug(FS_DEBUG, "No more free blocks\n");
            throw FSError(FSError::FS_OUT_OF_SPACE);
        }
    }

    fs.read(i)->init(FSBlockType::UNKNOWN); // ->type = FSBlockType::UNKNOWN;
    markAsAllocated(i);
    ap = (i + 1) % numBlocks;
    return i;
}

void
FSAllocator::allocate(isize count, std::vector<Block> &result, std::vector<Block> prealloc)
{
    /* Allocate multiple blocks and return them in `result`.
     *
     * Parameters:
     *
     * count    – number of blocks to allocate
     * result   – vector to store the allocated blocks
     * prealloc – optional list of pre-allocated blocks. If not empty, these
     *            blocks are used first: the allocator moves blocks from
     *            `prealloc` into `result` until `prealloc` is empty.
     *            Remaining blocks (if any) are allocated normally.
     *
     * Notes:
     *
     * - The function does not modify `prealloc` outside of moving blocks.
     * - Guarantees that `result` contains exactly `count` blocks upon return.
     */

    // Step 1: Use pre-allocated blocks first
    while (!prealloc.empty() && count > 0) {

        result.push_back(prealloc.back());
        prealloc.pop_back();
        count--;
    }

    // Step 2: Allocate remaining blocks from free space
    Block i = ap;
    while (count > 0) {

        if (fs.isEmpty(i)) {

            fs.read(i)->type = FSBlockType::UNKNOWN;
            result.push_back(i);
            count--;
        }

        // Move to the next block
        i = (i + 1) % fs.blocks();

        // Fail if we looped all the way and still need blocks
        if (i == ap && count > 0) {

            debug(FS_DEBUG, "No more free blocks\n");
            throw FSError(FSError::FS_OUT_OF_SPACE);
        }
    }

    // Step 3: Mark all blocks as allocated
    for (const auto &b : result) markAsAllocated(b);

    // Step 4: Advance allocation pointer
    ap = i;
}

void
FSAllocator::deallocateBlock(Block nr)
{
    cache.modify(nr).init(FSBlockType::EMPTY);
    markAsFree(nr);
}

void
FSAllocator::deallocateBlocks(const std::vector<Block> &nrs)
{
    for (Block nr : nrs) { deallocateBlock(nr); }
}

void
FSAllocator::allocateFileBlocks(isize bytes,
                                std::vector<Block> &listBlocks,
                                std::vector<Block> &dataBlocks)
{
    /* This function takes a file size and two lists:

            listBlocks  – pre-allocated list blocks (extension blocks)
            dataBlocks  – pre-allocated data blocks

        It first determines how many blocks of each type are required to store
        a file of the given size. If the caller provided more blocks than needed,
        the surplus blocks are freed. If fewer blocks are provided, new blocks
        are allocated and appended to the respective lists.
    */

    auto freeSurplus = [&](std::vector<Block> &blocks, usize count) {

        if (blocks.size() > count) {

            for (auto i = count; i < blocks.size(); i++) {
                deallocateBlock(blocks[i]);
            }
            blocks.resize(count);

        } else {

            blocks.reserve(count);
        }
    };

    usize dataBlocksNeeded = 0;
    auto ensureDataBlocks = [&](isize n) {

        dataBlocksNeeded += n;
        while (dataBlocks.size() < dataBlocksNeeded) allocate(1, dataBlocks);
    };

    usize listBlocksNeeded = 0;
    auto ensureListBlocks = [&](isize n) {

        listBlocksNeeded += n;
        while (listBlocks.size() < listBlocksNeeded) allocate(1, listBlocks);
    };

    isize numDataBlocks         = requiredDataBlocks(bytes);
    isize numListBlocks         = requiredFileListBlocks(bytes);
    isize refsPerBlock          = (traits.bsize / 4) - 56;
    isize refsInHeaderBlock     = std::min(numDataBlocks, refsPerBlock);
    isize refsInListBlocks      = numDataBlocks - refsInHeaderBlock;
    isize refsInLastListBlock   = refsInListBlocks % refsPerBlock;

    debug(FS_DEBUG, "                   Data bytes : %ld\n", bytes);
    debug(FS_DEBUG, "         Required data blocks : %ld\n", numDataBlocks);
    debug(FS_DEBUG, "         Required list blocks : %ld\n", numListBlocks);
    debug(FS_DEBUG, "         References per block : %ld\n", refsPerBlock);
    debug(FS_DEBUG, "   References in header block : %ld\n", refsInHeaderBlock);
    debug(FS_DEBUG, "    References in list blocks : %ld\n", refsInListBlocks);
    debug(FS_DEBUG, "References in last list block : %ld\n", refsInLastListBlock);

    // Free the surplus list blocks
    freeSurplus(listBlocks, numListBlocks);
    freeSurplus(dataBlocks, numDataBlocks);

    if (traits.ofs()) {

        // Header block -> Data blocks -> List block -> Data blocks ... List block -> Data blocks
        ensureDataBlocks(refsInHeaderBlock);

        for (isize i = 0; i < numListBlocks; i++) {

            ensureListBlocks(1);
            ensureDataBlocks(i < numListBlocks - 1 ? refsPerBlock : refsInLastListBlock);
        }
    }

    if (traits.ffs()) {

        // Header block -> Data blocks -> All list block -> All remaining data blocks
        ensureDataBlocks(refsInHeaderBlock);
        ensureListBlocks(numListBlocks);
        ensureDataBlocks(refsInListBlocks);
    }

    // Rectify checksums
    for (auto &it : fs.bmBlocks) fs.at(it).updateChecksum();
    for (auto &it : fs.bmExtBlocks) fs.at(it).updateChecksum();
}

bool
FSAllocator::isUnallocated(Block nr) const noexcept
{
    assert(isize(nr) < traits.blocks);

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;

    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    auto *bm = locateAllocationBit(nr, &byte, &bit);

    // Read the bit
    return bm ? GET_BIT(bm->data()[byte], bit) : false;
}

FSBlock *
FSAllocator::locateAllocationBit(Block nr, isize *byte, isize *bit) noexcept
{
    assert(isize(nr) < traits.blocks);

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;

    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (traits.bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm = (bmNr < (isize)fs.bmBlocks.size()) ? fs.read(fs.bmBlocks[bmNr], FSBlockType::BITMAP) : nullptr;
    if (!bm) {
        debug(FS_DEBUG, "Failed to lookup allocation bit for block %d (%ld)\n", nr, bmNr);
        return nullptr;
    }

    // Locate the byte position (note: the long word ordering will be reversed)
    nr = nr % bitsPerBlock;
    isize rByte = nr / 8;

    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < traits.bsize);

    *byte = rByte;
    *bit = nr % 8;

    return bm;
}

const FSBlock *
FSAllocator::locateAllocationBit(Block nr, isize *byte, isize *bit) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSAllocator *>(this)->locateAllocationBit(nr, byte, bit));
}

isize
FSAllocator::numUnallocated() const noexcept
{
    isize result = 0;
    for (auto &it : serializeBitmap()) result += std::popcount(it);

    if (FS_DEBUG) {

        isize count = 0;
        for (isize i = 0; i < fs.blocks(); i++) { if (isUnallocated(Block(i))) count++; }
        debug(true, "Unallocated blocks: Fast code: %ld Slow code: %ld\n", result, count);
        assert(count == result);
    }

    return result;
}

isize
FSAllocator::numAllocated() const noexcept
{
    return fs.blocks() - numUnallocated();
}

std::vector<u32>
FSAllocator::serializeBitmap() const
{
    if (!fs.isFormatted()) return {};

    auto longwords = ((fs.blocks() - 2) + 31) / 32;
    std::vector<u32> result;
    result.reserve(longwords);

    // Iterate through all bitmap blocks
    isize j = 0;
    for (auto &it : fs.bmBlocks) {

        if (auto *bm = fs.read(it, FSBlockType::BITMAP); bm) {

            auto *data = bm->data();
            for (isize i = 4; i < traits.bsize; i += 4) {

                if (j == longwords) break;
                result.push_back(HI_HI_LO_LO(data[i], data[i+1], data[i+2], data[i+3]));
                j++;
            }
        }
    }

    // Zero out the superfluous bits in the last word
    if (auto bits = (fs.blocks() - 2) % 32; bits && !result.empty()) {
        result.back() &= (1 << bits) - 1;
    }

    return result;
}

void
FSAllocator::setAllocationBit(Block nr, bool value)
{
    isize byte, bit;

    if (FSBlock *bm = locateAllocationBit(nr, &byte, &bit)) {
        REPLACE_BIT(bm->data()[byte], bit, value);
    }
}

}
