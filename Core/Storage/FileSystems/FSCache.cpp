// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSCache.h"
#include "FileSystem.h"
#include "utl/io.h"
#include <algorithm>

namespace vamiga {

FSCache::FSCache(FileSystem &fs, Volume &dev) : FSExtension(fs), dev(dev) {

    blocks.reserve(capacity());
};

FSCache::~FSCache()
{
    dealloc();
}

void
FSCache::dealloc()
{
    blocks.clear();
}

void
FSCache::dump(std::ostream &os) const
{
    using namespace utl;

    os << tab("Capacity") << capacity() << " blocks (x " << bsize() << " bytes)" << std::endl;
    os << tab("Hashed blocks") << blocks.size() << std::endl;
}

std::vector<Block>
FSCache::sortedKeys() const
{
    std::vector<Block> result;
    result.reserve(blocks.size());

    for (const auto& [key, _] : blocks) result.push_back(key);
    std::ranges::sort(result);

    return result;
}

bool
FSCache::isEmpty(Block nr) const noexcept
{
    return getType(nr) == FSBlockType::EMPTY;
}

FSBlockType
FSCache::getType(Block nr) const noexcept
{
    if (isize(nr) >= capacity()) return FSBlockType::UNKNOWN;
    return blocks.contains(nr) ? blocks.at(nr)->type : FSBlockType::EMPTY;
}

void
FSCache::setType(Block nr, FSBlockType type)
{
    if (isize(nr) >= capacity()) throw FSError(FSError::FS_OUT_OF_RANGE);
    blocks.at(nr)->init(type);
}

FSBlock *
FSCache::cache(Block nr) const noexcept
{
    if (isize(nr) >= capacity()) return nullptr;

    // Look up the block in the cache and return it if already present
    // On a miss, reserve an entry with a placeholder value
    auto [it, inserted] = blocks.try_emplace(nr, nullptr);
    if (!inserted) return it->second.get();

    // Create the block cache entry
    auto block = std::make_unique<FSBlock>(&fs, nr);
    block->dataCache.alloc(bsize());

    // Read block data from the underlying block device
    dev.readBlock(block->dataCache.ptr, nr);

    // Predict the block type based on its number and cached data
    block->type = fs.predictType(nr, block->dataCache.ptr);

    // Populate the reserved cache entry
    it->second = std::move(block);
    return it->second.get();
}

const FSBlock *
FSCache::tryFetch(Block nr) const noexcept
{
    return cache(nr);
}

const FSBlock *
FSCache::tryFetch(Block nr, FSBlockType type) const noexcept
{
    if (auto *ptr = tryFetch(nr); ptr) {

        if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

const FSBlock *
FSCache::tryFetch(Block nr, std::vector<FSBlockType> types) const noexcept
{
    if (auto *ptr = tryFetch(nr); ptr) {

        for (auto &type: types) if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

const FSBlock &
FSCache::fetch(Block nr) const
{
    if (auto *result = tryFetch(nr)) return *result;

    throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
}

const FSBlock &
FSCache::fetch(Block nr, FSBlockType type) const
{
    if (auto *result = tryFetch(nr, type)) return *result;

    if (tryFetch(nr)) {
        throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

const FSBlock &
FSCache::fetch(Block nr, std::vector<FSBlockType> types) const
{
    if (auto *result = tryFetch(nr, types); result) return *result;

    if (tryFetch(nr)) {
        throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

FSBlock *
FSCache::tryModify(Block nr) noexcept
{
    markAsDirty(nr);
    return cache(nr);
}

FSBlock *
FSCache::tryModify(Block nr, FSBlockType type) noexcept
{
    if (auto *ptr = tryModify(nr); ptr) {

        if (ptr->type == type) {

            markAsDirty(nr);
            return ptr;
        }
    }
    return nullptr;
}

FSBlock *
FSCache::tryModify(Block nr, std::vector<FSBlockType> types) noexcept
{
    if (auto *ptr = tryModify(nr); ptr) {

        for (auto &type: types) {

            if (ptr->type == type) {

                markAsDirty(nr);
                return ptr;
            }
        }
    }
    return nullptr;
}

FSBlock &
FSCache::modify(Block nr)
{
    if (isize(nr) >= capacity()) throw FSError(FSError::FS_OUT_OF_RANGE);
    if (auto *result = tryModify(nr)) return *result;
    throw FSError(FSError::FS_UNKNOWN);
}

FSBlock &
FSCache::modify(Block nr, FSBlockType type)
{
    if (isize(nr) >= capacity()) throw FSError(FSError::FS_OUT_OF_RANGE);
    if (auto *result = tryModify(nr, type)) return *result;
    throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
}

FSBlock &
FSCache::modify(Block nr, std::vector<FSBlockType> types)
{
    if (isize(nr) >= capacity()) throw FSError(FSError::FS_OUT_OF_RANGE);
    if (auto *result = tryModify(nr, types); result) return *result;
    throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
}





FSBlock *
FSCache::read(Block nr) noexcept
{
    return cache(nr);
}

/*
const FSBlock *
FSCache::read(Block nr) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSCache *>(this)->read(nr));
}
*/

FSBlock *
FSCache::read(Block nr, FSBlockType type) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

FSBlock *
FSCache::read(Block nr, std::vector<FSBlockType> types) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        for (auto &type: types) if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

/*
const FSBlock *
FSCache::read(Block nr, FSBlockType type) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSCache *>(this)->read(nr, type));
}

const FSBlock *
FSCache::read(Block nr, std::vector<FSBlockType> types) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSCache *>(this)->read(nr, types));
}
*/

FSBlock &
FSCache::at(Block nr)
{
    if (auto *result = read(nr)) return *result;

    throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
}

FSBlock &
FSCache::at(Block nr, FSBlockType type)
{
    if (auto *result = read(nr, type)) return *result;

    if (read(nr)) {
        throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

FSBlock &
FSCache::at(Block nr, std::vector<FSBlockType> types)
{
    if (auto *result = read(nr, types); result) return *result;

    if (read(nr)) {
        throw FSError(FSError::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw FSError(FSError::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

/*
const FSBlock &
FSCache::at(Block nr) const
{
    return const_cast<const FSBlock &>(const_cast<FSCache *>(this)->at(nr));
}

const FSBlock &
FSCache::at(Block nr, FSBlockType type) const
{
    return const_cast<const FSBlock &>(const_cast<FSCache *>(this)->at(nr, type));
}

const FSBlock &
FSCache::at(Block nr, std::vector<FSBlockType> types) const
{
    return const_cast<const FSBlock &>(const_cast<FSCache *>(this)->at(nr, types));
}
*/

void
FSCache::erase(Block nr)
{
    if (blocks.contains(nr)) { blocks.erase(nr); }
}

void
FSCache::flush(Block nr)
{
    if (dirty.contains(nr)) {

        storage.at(nr).flush();
        dirty.erase(nr);
    }
}

void
FSCache::flush()
{
    debug(FS_DEBUG, "Flushing %ld dirty blocks\n", dirty.size());

    for (auto block: dirty) flush(block);
}

void
FSCache::updateChecksums() noexcept
{
    for (auto &it : blocks) { it.second->updateChecksum(); }
}

void
FSCache::createUsageMap(u8 *buffer, isize len) const
{
    // Setup priorities
    i8 pri[12];
    pri[isize(FSBlockType::UNKNOWN)]      = 0;
    pri[isize(FSBlockType::EMPTY)]        = 1;
    pri[isize(FSBlockType::BOOT)]         = 8;
    pri[isize(FSBlockType::ROOT)]         = 9;
    pri[isize(FSBlockType::BITMAP)]       = 7;
    pri[isize(FSBlockType::BITMAP_EXT)]   = 6;
    pri[isize(FSBlockType::USERDIR)]      = 5;
    pri[isize(FSBlockType::FILEHEADER)]   = 3;
    pri[isize(FSBlockType::FILELIST)]     = 2;
    pri[isize(FSBlockType::DATA_OFS)]     = 2;
    pri[isize(FSBlockType::DATA_FFS)]     = 2;

    isize max = capacity();

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = (u8)FSBlockType::UNKNOWN;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = (u8)FSBlockType::EMPTY;

    // Mark all used blocks
    for (auto &it : blocks) {

        auto i = Block(it.first);

        auto val = u8(getType(Block(i)));
        auto pos = i * (len - 1) / (max - 1);
        if (pri[buffer[pos]] < pri[val]) buffer[pos] = val;
        if (pri[buffer[pos]] == pri[val] && pos > 0 && buffer[pos-1] != val) buffer[pos] = val;
    }

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == (u8)FSBlockType::UNKNOWN) buffer[pos] = buffer[pos - 1];
    }
}

void
FSCache::createAllocationMap(u8 *buffer, isize len, const FSDiagnosis diagnosis) const
{
    auto &unusedButAllocated = diagnosis.unusedButAllocated;
    auto &usedButUnallocated = diagnosis.usedButUnallocated;

    isize max = capacity();

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = 0;

    // Mark all used blocks
    for (auto &it : blocks) { if (!isEmpty(Block(it.first))) buffer[it.first * (len - 1) / (max - 1)] = 1; }

    // Mark all erroneous blocks
    for (auto &it : unusedButAllocated) buffer[it * (len - 1) / (max - 1)] = 2;
    for (auto &it : usedButUnallocated) buffer[it * (len - 1) / (max - 1)] = 3;

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == 255) buffer[pos] = buffer[pos - 1];
    }
}

void
FSCache::createHealthMap(u8 *buffer, isize len, const FSDiagnosis diagnosis) const
{
    auto &blockErrors = diagnosis.blockErrors;

    isize max = capacity();

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 255;

    // Mark all free blocks
    for (isize i = 0; i < max; i++) buffer[i * (len - 1) / (max - 1)] = 0;

    // Mark all used blocks
    for (auto &it : blocks) { if (!isEmpty(Block(it.first))) buffer[it.first * (len - 1) / (max - 1)] = 1; }

    // Mark all erroneous blocks
    for (auto &it : blockErrors) buffer[it * (len - 1) / (max - 1)] = 2;

    // Fill gaps
    for (isize pos = 1; pos < len; pos++) {
        if (buffer[pos] == 255) buffer[pos] = buffer[pos - 1];
    }
}

}
