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
    blocks = { };
}

void
BlockStorage::init(isize capacity, isize bsize)
{
    this->_capacity = capacity;
    this->_bsize = bsize;

    // Remove all existing blocks
    blocks = { };

    // Resize the block storage
    blocks.reserve(_capacity);
    // blocks.assign(_capacity, nullptr);

    // Create new blocks
    assert(blocks.size() == 0);
    for (isize i = 0; i < _capacity; i++) {
        blocks.push_back(FSBlock(fs, Block(i), FSBlockType::EMPTY_BLOCK));
        assert(blocks[i].data == nullptr);
    }
    assert((isize)blocks.size() == _capacity);
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
    assert(_capacity == (isize)blocks.size());
    return nr < _capacity ? blocks[nr].type : FSBlockType::UNKNOWN_BLOCK;
}

void
BlockStorage::setType(Block nr, FSBlockType type)
{
    assert(_capacity == (isize)blocks.size());
    if (nr >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);
    blocks[nr].init(type);
}

FSBlock &
BlockStorage::read(Block nr)
{
    assert(_capacity == (isize)blocks.size());
    if (nr >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    // Get the block
    FSBlock &result = blocks[nr];

    // Allocate the buffer if necessary
    if (!blocks[nr].data) blocks[nr].data = new u8[_bsize];

    return result;
}

const FSBlock &
BlockStorage::read(Block nr) const
{
    return *pread(nr);
}

FSBlock *
BlockStorage::pread(Block nr)
{
    assert(_capacity == (isize)blocks.size());

    if (nr < _capacity) {
        return &blocks[nr];
    }
    return nullptr;
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
    assert(_capacity == (isize)blocks.size());
    if (nr < _capacity && blocks[nr].type == type) {
        return &blocks[nr];
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
    assert(_capacity == (isize)blocks.size());
    if (nr >= _capacity) throw AppError(Fault::FS_INVALID_BLOCK_REF);

    blocks[nr] = *block;
}

}
