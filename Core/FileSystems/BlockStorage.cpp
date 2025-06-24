// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockStorage.h"

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
    blocks = { };
}

void
BlockStorage::init(isize capacity, isize bsize)
{
    this->capacity = capacity;
    this->bsize = bsize;

    // Remove all existing blocks
    blocks = {};

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

    return blocks.contains(nr) ? blocks.at(nr).type : FSBlockType::EMPTY_BLOCK;
}

void
BlockStorage::setType(Block nr, FSBlockType type)
{
    if (isize(nr) >= capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    blocks.at(nr).init(type);
}

FSBlock &
BlockStorage::operator[](size_t nr)
{
    return *read(Block(nr));
}

const FSBlock &
BlockStorage::operator[](size_t nr) const
{
    return *read(Block(nr));
}

FSBlock *
BlockStorage::read(Block nr)
{
    if (nr >= size_t(capacity)) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    // Create the block if it does not yet exist
    if (!blocks.contains(nr)) blocks.emplace(nr, FSBlock(fs, Block(nr), FSBlockType::EMPTY_BLOCK));

    // Collect some statistical information
    stats.blockReads++;

    // Return a block reference
    return &blocks.at(nr);
}

const FSBlock *
BlockStorage::read(Block nr) const
{
    return const_cast<const FSBlock *>(const_cast<BlockStorage *>(this)->read(nr));
}

FSBlock *
BlockStorage::read(Block nr, FSBlockType type)
{
    if (isize(nr) < capacity) {

        auto &block = (*this)[nr];
        if (block.type == type) return &block;
    }

    return nullptr;
}

const FSBlock *
BlockStorage::read(Block nr, FSBlockType type) const
{
    auto result = const_cast<BlockStorage *>(this)->read(nr, type);
    return const_cast<const FSBlock *>(result);
}

void
BlockStorage::erase(Block nr)
{
    if (blocks.contains(nr)) { blocks.erase(nr); }
}

}
