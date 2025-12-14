// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockStorage.h"
#include "DeviceDescriptors.h"

#pragma once

namespace vamiga {

class BlockView {

public:

    virtual ~BlockView() = default;

    virtual isize capacity() const = 0;
    virtual isize bsize() const = 0;
    virtual void freeBlock(isize nr) = 0;
    virtual Buffer<u8> *readBlock(isize nr) = 0;
    virtual Buffer<u8> *ensureBlock(isize nr) = 0;

    bool inRange(isize nr) { return nr >= 0 && nr < capacity(); }
};

class Partition : public BlockView {

    class Device &device;
    PartitionDescriptor descriptor {};

public:

    Partition(Device &dev, const PartitionDescriptor &des) : device(dev), descriptor(des) { }

    isize capacity() const override;
    isize bsize() const override;
    void freeBlock(isize nr) override;
    Buffer<u8> *readBlock(isize nr) override;
    Buffer<u8> *ensureBlock(isize nr) override;
};

class Device : public BlockView {

    // Physical geometry of this device
    GeometryDescriptor geometry;

    // Block data
    std::vector<std::unique_ptr<Buffer<u8>>> blocks;

    // Logical partitions
    std::vector<Partition> partitions;

public:

    Device(const GeometryDescriptor &desc) { init(desc); }

    void init(const GeometryDescriptor &desc);
    void init(const class ADFFile &adf);
    void init(const class HDFFile &hdf);

private:

    void importBlock(isize nr, const u8 *data);

public:

    const GeometryDescriptor &getGeometry() const { return geometry; }

    isize capacity() const override { return geometry.numBlocks(); }
    isize bsize() const override { return geometry.bsize; }
    void freeBlock(isize nr) override;
    Buffer<u8> *readBlock(isize nr) override;
    Buffer<u8> *ensureBlock(isize nr) override;


};

}
