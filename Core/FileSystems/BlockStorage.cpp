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

    // Clear statistics
    stats = {};
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
            os << tab("Block reads") << stats.blockReads << std::endl;
            break;

        default:
            break;
    }
}

void
BlockStorage::cacheStats(BlockStorageStats &result) const
{

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

    // Collect some statistical information
    stats.blockReads++;

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

}
