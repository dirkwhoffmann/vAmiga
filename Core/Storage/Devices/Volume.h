// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockView.h"
#include "DeviceDescriptors.h"

namespace vamiga {

class BlockVolume : public BlockView {

    class BlockView &device;

    // Translates a logical block number into a physical block number
    virtual isize translate(isize block) const = 0;

public:

    BlockVolume(BlockView &dev) : device(dev) { }

    isize bsize() const override;
    void freeBlock(isize nr) override;
    Buffer<u8> *deprecatedReadBlock(isize nr) override;
    Buffer<u8> *ensureBlock(isize nr) override;
    void writeBlock(isize nr, const Buffer<u8> &buffer) override;
};

class HardDrivePartition : public BlockVolume {

    PartitionDescriptor descriptor {};

    isize capacity() const override { return descriptor.numBlocks(); }
    isize translate(isize block) const override { return descriptor.translate(block); }

public:

    HardDrivePartition(BlockView &dev, const PartitionDescriptor &des)
    : BlockVolume(dev), descriptor(des) { }
};

}
