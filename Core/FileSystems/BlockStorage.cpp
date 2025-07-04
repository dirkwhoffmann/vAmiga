// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockStorage.h"
#include "FileSystem.h"

namespace vamiga {

BlockStorage::BlockStorage(FileSystem *fs, isize capacity, isize bsize) : fs(fs)
{
    init(capacity, bsize);
}

BlockStorage::~BlockStorage()
{
    dealloc();
}

void
BlockStorage::dealloc()
{
    fs = nullptr;
    blocks.clear();
}

void
BlockStorage::init(isize capacity, isize bsize)
{
    this->capacity = capacity;
    this->bsize = bsize;

    // Remove all existing blocks
    blocks.clear();

    // Request a capacity change
    blocks.reserve(capacity);
}

void
BlockStorage::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    switch (category) {

        case Category::Blocks:

            os << tab("Capacity") << numBlocks() << " blocks" << std::endl;
            os << tab("Block size") << bsize << " bytes" << std::endl;
            os << tab("Hashed blocks") << blocks.size() << std::endl;
            break;

        default:
            break;
    }
}

bool
BlockStorage::isEmpty(Block nr) const
{
    return getType(nr) == FSBlockType::EMPTY_BLOCK;
}

FSBlockType
BlockStorage::getType(Block nr) const
{
    if (isize(nr) >= capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    return blocks.contains(nr) ? blocks.at(nr)->type : FSBlockType::EMPTY_BLOCK;
}

void
BlockStorage::setType(Block nr, FSBlockType type)
{
    if (isize(nr) >= capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    blocks.at(nr)->init(type);
}

FSBlock *
BlockStorage::read(Block nr) noexcept
{
    if (nr >= size_t(capacity)) return nullptr;

    // Create the block if it does not yet exist
    if (!blocks.contains(nr)) blocks.emplace(nr, std::make_unique<FSBlock>(fs, nr, FSBlockType::EMPTY_BLOCK));

    // Return a block reference
    return blocks.at(nr).get();
}

const FSBlock *
BlockStorage::read(Block nr) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<BlockStorage *>(this)->read(nr));
}

FSBlock *
BlockStorage::read(Block nr, FSBlockType type) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

FSBlock *
BlockStorage::read(Block nr, std::vector<FSBlockType> types) noexcept
{
    if (auto *ptr = read(nr); ptr) {

        for (auto &type: types) if (ptr->type == type) { return ptr; }
    }
    return nullptr;
}

const FSBlock *
BlockStorage::read(Block nr, FSBlockType type) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<BlockStorage *>(this)->read(nr, type));
}

const FSBlock *
BlockStorage::read(Block nr, std::vector<FSBlockType> types) const noexcept
{
    return const_cast<const FSBlock *>(const_cast<BlockStorage *>(this)->read(nr, types));
}

FSBlock &
BlockStorage::at(Block nr)
{
    if (auto *result = read(nr); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else {
        throw AppError(Fault::FS_INVALID_BLOCK_REF, std::to_string(nr));
    }
}

FSBlock &
BlockStorage::at(Block nr, FSBlockType type)
{
    if (auto *result = read(nr, type); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else if (read(nr)) {
        throw AppError(Fault::FS_INVALID_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw AppError(Fault::FS_INVALID_BLOCK_REF, std::to_string(nr));
    }
}

FSBlock &
BlockStorage::at(Block nr, std::vector<FSBlockType> types)
{
    if (auto *result = read(nr, types); result) return *result;

    if (!fs->isInitialized()) {
        throw AppError(Fault::FS_UNINITIALIZED);
    } else if (read(nr)) {
        throw AppError(Fault::FS_INVALID_BLOCK_TYPE, std::to_string(nr));
    } else {
        throw AppError(Fault::FS_INVALID_BLOCK_REF, std::to_string(nr));
    }
}

const FSBlock &
BlockStorage::at(Block nr) const
{
    return const_cast<const FSBlock &>(const_cast<BlockStorage *>(this)->at(nr));
}

const FSBlock &
BlockStorage::at(Block nr, FSBlockType type) const
{
    return const_cast<const FSBlock &>(const_cast<BlockStorage *>(this)->at(nr, type));
}

const FSBlock &
BlockStorage::at(Block nr, std::vector<FSBlockType> types) const
{
    return const_cast<const FSBlock &>(const_cast<BlockStorage *>(this)->at(nr, types));
}

void
BlockStorage::erase(Block nr)
{
    if (blocks.contains(nr)) { blocks.erase(nr); }
}

void
BlockStorage::createUsageMap(u8 *buffer, isize len) const
{
    isize max = numBlocks();

    // Setup priorities
    i8 pri[12];
    pri[isize(FSBlockType::UNKNOWN_BLOCK)]      = 0;
    pri[isize(FSBlockType::EMPTY_BLOCK)]        = 1;
    pri[isize(FSBlockType::BOOT_BLOCK)]         = 8;
    pri[isize(FSBlockType::ROOT_BLOCK)]         = 9;
    pri[isize(FSBlockType::BITMAP_BLOCK)]       = 7;
    pri[isize(FSBlockType::BITMAP_EXT_BLOCK)]   = 6;
    pri[isize(FSBlockType::USERDIR_BLOCK)]      = 5;
    pri[isize(FSBlockType::FILEHEADER_BLOCK)]   = 3;
    pri[isize(FSBlockType::FILELIST_BLOCK)]     = 2;
    pri[isize(FSBlockType::DATA_BLOCK_OFS)]     = 2;
    pri[isize(FSBlockType::DATA_BLOCK_FFS)]     = 2;

    // Start from scratch
    for (isize i = 0; i < len; i++) buffer[i] = 0;

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
        if (buffer[pos] == u8(FSBlockType::UNKNOWN_BLOCK)) buffer[pos] = buffer[pos - 1];
    }
}

void
BlockStorage::createAllocationMap(u8 *buffer, isize len) const
{
    auto &unusedButAllocated = fs->doctor.diagnosis.unusedButAllocated;
    auto &usedButUnallocated = fs->doctor.diagnosis.usedButUnallocated;

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
BlockStorage::createHealthMap(u8 *buffer, isize len) const
{
    auto &blockErrors = fs->doctor.diagnosis.blockErrors;

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
