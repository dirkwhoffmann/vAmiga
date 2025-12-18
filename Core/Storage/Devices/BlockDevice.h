// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockVolume.h"

namespace vamiga {

class BlockDevice : public BlockView {

    virtual isize numVolumes() const = 0;
    virtual BlockVolume &volume(isize nr) = 0;
};

/*
namespace brainstorm {

class BlockView {

public:

    virtual ~BlockView() = default;

    virtual isize capacity() const = 0;
    virtual isize bsize() const = 0;

    virtual void readBlock(isize nr, u8 *dst) = 0;
    virtual void writeBlock(isize nr, const u8 *src) = 0;
};

class TrackView {

public:

    virtual ~TrackView() = default;

    virtual isize numTracks() const = 0;

    virtual void readTrack(isize nr, u8 *dst) = 0;
    virtual void writeTrack(isize nr, const u8 *src) = 0;
};

class Volume : BlockView {

    string name;
    string description;
};

class Device : BlockView {

    string name;
    string description;

    std::vector<Volume> volumes;
};


class BlockDevice : public Device, public BlockView {

};

class TrackDevice: public BlockView, public TrackView {

    virtual isize numPartitions() = 0;
    virtual class Partition &partition(isize nr) = 0;
};

class Partition : public BlockView {

    TrackDevice &device;
    PartitionDescriptor descriptor {};

public:

    Partition(TrackDevice &dev, const PartitionDescriptor &des) : device(dev), descriptor(des) { }
};

}
*/

/*
class Device : public BlockView {

    // Physical geometry of this device
    GeometryDescriptor geometry;

    // Block data
    std::vector<std::unique_ptr<Buffer<u8>>> blocks;

    // Logical partitions
    std::vector<Partition> partitions;

public:

    Device(const GeometryDescriptor &desc) { init(desc); }
    Device(Diameter dia, Density den) { init(dia, den); }

    void init(const GeometryDescriptor &desc);
    void init(Diameter dia, Density den);
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
    void writeBlock(isize nr, const Buffer<u8> &buffer) override;
};
*/

}
