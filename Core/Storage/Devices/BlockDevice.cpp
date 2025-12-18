// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockDevice.h"
#include "Media.h"

namespace vamiga {

/*
isize
Partition::capacity() const
{
    return descriptor.numBlocks();
}

isize
Partition::bsize() const
{
    return device.bsize();
}

void
Partition::freeBlock(isize nr)
{
    if (inRange(nr)) device.freeBlock(descriptor.translate(nr));
}

Buffer<u8> *
Partition::readBlock(isize nr)
{
    return inRange(nr) ? device.readBlock(descriptor.translate(nr)) : nullptr;
}

Buffer<u8> *
Partition::ensureBlock(isize nr)
{
    return inRange(nr) ? device.ensureBlock(descriptor.translate(nr)) : nullptr;
}

void
Partition::writeBlock(isize nr, const Buffer<u8> &buffer)
{
    device.writeBlock(descriptor.translate(nr), buffer);
}

void
Device::init(const GeometryDescriptor &desc)
{
    geometry = desc;

    // Remove all existing blocks
    blocks.clear();

    // Adjust capacity
    blocks.resize(capacity());
}

void
Device::init(Diameter dia, Density den)
{
    init(GeometryDescriptor(dia, den));
}

void
Device::init(const class ADFFile &adf)
{
    auto geometry = adf.getGeometry();
    init(geometry);

    for (isize i = 0; i < capacity(); i++) {
        importBlock(i, adf.data.ptr + i * geometry.bsize);
    }
}

void
Device::init(const class HDFFile &hdf)
{
    auto geometry = hdf.getGeometry();
    init(geometry);

    // Import all blocks
    for (isize i = 0; i < capacity(); i++) {
        importBlock(i, hdf.data.ptr + i * geometry.bsize);
    }

    // Import partition layout
    partitions.reserve(hdf.numPartitions());
    for (isize i = 0; i < hdf.numPartitions(); i++) {
        partitions.emplace_back(Partition(*this, hdf.getPartitionDescriptor(i)));
    }
}

void
Device::importBlock(isize nr, const u8 *data)
{
    if (auto *block = ensureBlock(nr)) {
        block->init(data, bsize());
    }
}

void
Device::freeBlock(isize nr)
{
    if (inRange(nr)) blocks[nr].reset();
}

Buffer<u8> *
Device::readBlock(isize nr)
{
    return inRange(nr) ? blocks[nr].get() : nullptr;
}

Buffer<u8> *
Device::ensureBlock(isize nr)
{
    // Return the block if it already exists
    if (auto block = readBlock(nr)) { return block; }

    // Create a new block
    blocks[nr] = std::make_unique<Buffer<u8>>(geometry.bsize);
    return blocks[nr].get();
}

void
Device::writeBlock(isize nr, const Buffer<u8> &buffer)
{
    assert(buffer.size == bsize());

    auto *block = ensureBlock(nr);
    *block = buffer;
}
*/

}
