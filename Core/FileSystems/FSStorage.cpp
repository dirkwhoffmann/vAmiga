// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "FSStorage.h"
#include "FileSystem.h"
#include <algorithm>

namespace vamiga {

FSStorage::FSStorage(FileSystem *fs, isize capacity, isize bsize) : fs(fs)
{
    init(capacity, bsize);
}

FSStorage::~FSStorage()
{
    dealloc();
}

void
FSStorage::dealloc()
{
    fs = nullptr;
    blocks.clear();
}

void
FSStorage::init(isize capacity, isize bsize)
{
    this->capacity = capacity;
    this->bsize = bsize;

    // Remove all existing blocks
    blocks.clear();

    // Request a capacity change
    blocks.reserve(capacity);
}

void
FSStorage::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    switch (category) {

        case Category::Blocks:

            os << tab("Capacity") << numBlocks() << " blocks (x " << bsize << " bytes)" << std::endl;
            os << tab("Hashed blocks") << blocks.size() << std::endl;
            break;

        default:
            break;
    }
}

std::vector<Block>
FSStorage::sortedKeys() const
{
    std::vector<Block> result;
    result.reserve(blocks.size());

    for (const auto& [key, _] : blocks) result.push_back(key);
    std::ranges::sort(result);

    return result;
}

bool
FSStorage::isEmpty(Block nr) const noexcept
{
    return getType(nr) == FSBlockType::EMPTY;
}

FSBlockType
FSStorage::getType(Block nr) const noexcept
{
    if (isize(nr) >= capacity) return FSBlockType::UNKNOWN;
    return blocks.contains(nr) ? blocks.at(nr)->type : FSBlockType::EMPTY;
}

void
FSStorage::setType(Block nr, FSBlockType type)
{
    if (isize(nr) >= capacity) throw AppError(Fault::FS_OUT_OF_RANGE);
    blocks.at(nr)->init(type);
}

FSBlock *
FSStorage::read(Block nr) noexcept
{
    if (nr >= size_t(capacity)) return nullptr;

    // Create the block if it does not yet exist
    if (!blocks.contains(nr)) {
        blocks.emplace(nr, std::make_unique<FSBlock>(fs, nr, FSBlockType::EMPTY));
    }

    // Return a block reference
    return blocks.at(nr).get();
}

const FSBlock *
FSStorage::read(Block nr) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSStorage *>(this)->read(nr));
}

FSBlock *
FSStorage::read(Block nr, FSBlockType type) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

FSBlock *
FSStorage::read(Block nr, std::vector<FSBlockType> types) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        for (auto &type: types) if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

const FSBlock *
FSStorage::read(Block nr, FSBlockType type) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSStorage *>(this)->read(nr, type));
}

const FSBlock *
FSStorage::read(Block nr, std::vector<FSBlockType> types) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<FSStorage *>(this)->read(nr, types));
}

FSBlock &
FSStorage::at(Block nr)
{
    if (auto *result = read(nr); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else {
        throw AppError(Fault::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

FSBlock &
FSStorage::at(Block nr, FSBlockType type)
{
    if (auto *result = read(nr, type); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else if (read(nr)) {
        throw AppError(Fault::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw AppError(Fault::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

FSBlock &
FSStorage::at(Block nr, std::vector<FSBlockType> types)
{
    if (auto *result = read(nr, types); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else if (read(nr)) {
        throw AppError(Fault::FS_WRONG_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw AppError(Fault::FS_OUT_OF_RANGE, std::to_string(nr));
    }
}

const FSBlock &
FSStorage::at(Block nr) const
{
    return const_cast<const FSBlock &>(const_cast<FSStorage *>(this)->at(nr));
}

const FSBlock &
FSStorage::at(Block nr, FSBlockType type) const
{
    return const_cast<const FSBlock &>(const_cast<FSStorage *>(this)->at(nr, type));
}

const FSBlock &
FSStorage::at(Block nr, std::vector<FSBlockType> types) const
{
    return const_cast<const FSBlock &>(const_cast<FSStorage *>(this)->at(nr, types));
}

void
FSStorage::erase(Block nr)
{
    if (blocks.contains(nr)) { blocks.erase(nr); }
}

void
FSStorage::updateChecksums() noexcept
{
    for (auto &it : blocks) { it.second->updateChecksum(); }
}

void
FSStorage::createUsageMap(u8 *buffer, isize len) const
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

    isize max = numBlocks();

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
FSStorage::createAllocationMap(u8 *buffer, isize len, const FSDiagnosis diagnosis) const
{
    auto &unusedButAllocated = diagnosis.unusedButAllocated;
    auto &usedButUnallocated = diagnosis.usedButUnallocated;

    isize max = numBlocks();

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
FSStorage::createHealthMap(u8 *buffer, isize len, const FSDiagnosis diagnosis) const
{
    auto &blockErrors = diagnosis.blockErrors;

    isize max = numBlocks();

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
