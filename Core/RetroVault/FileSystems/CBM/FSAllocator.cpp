// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FileSystems/CBM/FileSystem.h"
#include "utl/support.h"
#include <bit>

namespace retro::vault::cbm {

isize
FSAllocator::requiredDataBlocks(isize fileSize) const noexcept
{
    // Compute the capacity of a single data block
    isize numBytes = traits.bsize - 2;

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
    return (fileSize + 253) / 254;
}

/*
bool
FSAllocator::allocatable(isize count) const noexcept
{
    BlockNr i = ap;
    isize capacity = fs.blocks();

    while (count > 0) {

        if (fs.isEmpty(i)) {
            if (--count == 0) break;
        }

        i = (i + 1) % capacity;
        if (i == ap) return false;
    }

    return true;
}
*/

BlockNr
FSAllocator::allocate()
{
    if (auto blocks = allocate(1); !blocks.empty())
        return blocks[0];

    throw FSError(FSError::FS_OUT_OF_SPACE);
}

std::vector<BlockNr>
FSAllocator::allocate(isize count)
{
    std::vector<BlockNr> result;
    TSLink ts = ap;

    // Gather 'count' free blocks
    while (count > 0) {

        if (auto nr = traits.blockNr(ts); nr.has_value()) {

            // Note this block if it is empty
            if (fs.isEmpty(*nr)) {

                result.push_back(*nr);
                count--;
            }

            // Move to the next block
            ts = advance(ts);
            if (ts != ap) continue;
        }
        throw FSError(FSError::FS_OUT_OF_SPACE);
    }

    // Allocate blocks
     for (const auto &b : result) {

        fs.fetch(b).mutate().type = FSBlockType::UNKNOWN;
        markAsAllocated(b);
    }

    return result;
}

std::vector<BlockNr>
FSAllocator::allocate(isize count, std::vector<BlockNr> prealloc)
{
    std::vector<BlockNr> result;
     result.reserve(count);

     // Step 1: Use pre-allocated blocks from the start
     auto it = prealloc.begin();
     while (it != prealloc.end() && count > 0) {

         result.push_back(*it);
         ++it;
         --count;
     }

     // Step 2: Allocate remaining blocks from free space
     if (count > 0) {

         auto more = allocate(count);
         result.insert(result.end(), more.begin(), more.end());
     }

     // Step 3: Free all unused preallocated blocks
     for (; it != prealloc.end(); ++it) deallocateBlock(*it);

     return result;
}

void
FSAllocator::deallocateBlock(BlockNr nr)
{
    fs.fetch(nr).mutate().init(FSBlockType::EMPTY);
    markAsFree(nr);
}

void
FSAllocator::deallocateBlocks(const std::vector<BlockNr> &nrs)
{
    for (BlockNr nr : nrs) { deallocateBlock(nr); }
}

TSLink
FSAllocator::advance(TSLink ts)
{
    // Interleave patterns used to determine the next sector
    static constexpr SectorNr next[5][21] = {

        // Speed zone 0 - 3
        { 10,11,12,13,14,15,16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 10,11,12,13,14,15,16,17, 1, 0, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 10,11,12,13,14,15,16,17,18, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        { 10,11,12,13,14,15,16,17,18,19,20, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },

        // Directory track
        {  3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18, 0, 1, 2 }
    };

    if (!traits.isValidLink(ts)) return {0,0};

    TrackNr t  = ts.t;
    SectorNr s = ts.s;

    if (t == 18) {

        // Take care of the directory track
        s = next[4][s];

        // Return immediately if we've wrapped over (directory track is full)
        if (s == 0) return {0,0};

    } else {

        // Take care of all other tracks
        s = next[traits.speedZone(t)][s];

        // Move to the next track if we've wrapped over
        if (s == 0) t = t >= traits.numTracks() ? 1 : t == 17 ? 19 : t + 1;
    }

    assert(traits.isValidLink(TSLink{t,s}));
    return TSLink{t,s};
}

/*
void
FSAllocator::allocateFileBlocks(isize bytes,
                                std::vector<BlockNr> &listBlocks,
                                std::vector<BlockNr> &dataBlocks)
{
    auto freeSurplus = [&](std::vector<BlockNr> &blocks, usize count) {

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

    loginfo(FS_DEBUG, "                   Data bytes : %ld\n", bytes);
    loginfo(FS_DEBUG, "         Required data blocks : %ld\n", numDataBlocks);
    loginfo(FS_DEBUG, "         Required list blocks : %ld\n", numListBlocks);
    loginfo(FS_DEBUG, "         References per block : %ld\n", refsPerBlock);
    loginfo(FS_DEBUG, "   References in header block : %ld\n", refsInHeaderBlock);
    loginfo(FS_DEBUG, "    References in list blocks : %ld\n", refsInListBlocks);
    loginfo(FS_DEBUG, "References in last list block : %ld\n", refsInLastListBlock);

    // Free the surplus list blocks
    freeSurplus(listBlocks, numListBlocks);
    freeSurplus(dataBlocks, numDataBlocks);

    { // if (traits.ofs()) {

        // Header block -> Data blocks -> List block -> Data blocks ... List block -> Data blocks
        ensureDataBlocks(refsInHeaderBlock);

        for (isize i = 0; i < numListBlocks; i++) {

            ensureListBlocks(1);
            ensureDataBlocks(i < numListBlocks - 1 ? refsPerBlock : refsInLastListBlock);
        }
    }

    // Rectify checksums
    for (auto &it : fs.getBmBlocks()) fs[it].mutate().updateChecksum();
    for (auto &it : fs.getBmExtBlocks()) fs[it].mutate().updateChecksum();
}
*/

bool
FSAllocator::isUnallocated(BlockNr nr) const noexcept
{
    assert(isize(nr) < traits.blocks);

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;

    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    auto *bm = locateAllocBit(nr, &byte, &bit);

    // Read the bit
    return bm ? GET_BIT(bm->data()[byte], bit) : false;
}

const FSBlock *
FSAllocator::locateAllocBit(BlockNr nr, isize *byte, isize *bit) const noexcept
{
    return locateAllocBit(traits.tsLink(nr), byte, bit);
}

const FSBlock *
FSAllocator::locateAllocBit(TSLink ts, isize *byte, isize *bit) const noexcept
{
    if (!traits.isValidLink(ts)) return nullptr;

    /* Bytes $04 - $8F store the BAM entries for each track, in groups of four
     * bytes per track, starting on track 1. [...] The first byte is the number
     * of free sectors on that track. The next three bytes represent the bitmap
     * of which sectors are used/free. Since it is 3 bytes we have 24 bits of
     * storage. Remember that at most, each track only has 21 sectors, so there
     * are a few unused bits.
     */

    *byte = (4 * ts.t) + 1 + (ts.s >> 3);
    *bit = ts.s & 0x07;

    return fs.tryFetchBAM();
}

isize
FSAllocator::numUnallocated() const noexcept
{
    isize result = 0;
    for (auto &it : serializeBitmap()) result += std::popcount(it);

    if constexpr (debug::FS_DEBUG) {

        isize count = 0;
        for (isize i = 0; i < fs.blocks(); i++) { if (isUnallocated(BlockNr(i))) count++; }
        loginfo(FS_DEBUG, "Unallocated blocks: Fast code: %ld Slow code: %ld\n", result, count);
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
    for (auto &it : fs.getBmBlocks()) {

        if (auto *bm = fs.tryFetch(it, FSBlockType::BITMAP)) {

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
FSAllocator::setAllocBit(BlockNr nr, bool value)
{
    isize byte, bit;

    if (auto *bm = locateAllocBit(nr, &byte, &bit)) {

        auto *data = bm->mutate().data();
        REPLACE_BIT(data[byte], bit, value);
    }
}

}
