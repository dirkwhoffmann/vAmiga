// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

/*
#include "BlockStorage.h"

namespace vamiga {

void
BlockStorage::init(isize capacity, isize bsize)
{
    this->capacity = capacity;
    this->bsize = bsize;

    // Remove all existing blocks
    blocks.clear();

    // Adjust capacity
    blocks.resize(capacity);
}

void
BlockStorage::freeBlock(isize nr)
{
    if (nr < 0 || nr >= capacity) return;

    blocks[nr].reset();
}

Buffer<u8> *
BlockStorage::readBlock(isize nr)
{
    if (nr < 0 || nr >= capacity) return nullptr;

    return blocks[nr].get();
}

Buffer<u8> *
BlockStorage::ensureBlock(isize nr)
{
    // Return the block if it already exists
    if (auto block = readBlock(nr)) { return block; }

    // Create a new block
    blocks[nr] = std::make_unique<Buffer<u8>>(bsize);
    return blocks[nr].get();
}

}
*/
