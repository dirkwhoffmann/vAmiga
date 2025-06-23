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
    this->_capacity = capacity;
    this->_bsize = bsize;

    // Remove all existing blocks
    blocks = {};

    // Request a capacity change
    blocks.reserve(_capacity);
}

void
BlockStorage::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    switch (category) {

        case Category::Blocks:

            os << tab("Capacity") << capacity() << std::endl;
            os << tab("Block size") << bsize() << std::endl;
            break;

        default:
            break;
    }
}

FSBlockType
BlockStorage::getType(Block nr) const
{
    if (isize(nr) >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    return blocks.contains(nr) ? blocks.at(nr).type : FSBlockType::EMPTY_BLOCK;
}

void
BlockStorage::setType(Block nr, FSBlockType type)
{
    if (isize(nr) >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    blocks.at(nr).init(type);
}

FSBlock &
BlockStorage::read(Block nr)
{
    if (isize(nr) >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    // Create the block if it does not yet exist
    if (!blocks.contains(nr)) blocks.emplace(nr, FSBlock(fs, Block(nr), FSBlockType::EMPTY_BLOCK));

    // Return a block reference
    return blocks.at(nr);
    // FSBlock &result = blocks.at(nr);

    // Allocate the buffer if necessary
    // if (!result.bdata) result.bdata = new u8[_bsize];

    //return result;
}

const FSBlock &
BlockStorage::read(Block nr) const
{
    auto &result = const_cast<BlockStorage *>(this)->read(nr);
    return const_cast<const FSBlock &>(result);
}

FSBlock *
BlockStorage::pread(Block nr)
{
    if (isize(nr) >= _capacity) return nullptr;

    return &read(nr);
}

const FSBlock *
BlockStorage::pread(Block nr) const
{
    auto result = const_cast<BlockStorage *>(this)->pread(nr);
    return const_cast<const FSBlock *>(result);
}

FSBlock *
BlockStorage::pread(Block nr, FSBlockType type)
{
    if (isize(nr) < _capacity) {

        auto &block = read(nr);
        if (block.type == type) return &block;
    }

    return nullptr;
}

const FSBlock *
BlockStorage::pread(Block nr, FSBlockType type) const
{
    auto result = const_cast<BlockStorage *>(this)->pread(nr, type);
    return const_cast<const FSBlock *>(result);
}

void
BlockStorage::write(Block nr, FSBlock *block)
{
    if (isize(nr) >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    read(nr) = *block;
}

}
