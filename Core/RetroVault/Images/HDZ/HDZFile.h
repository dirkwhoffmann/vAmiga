// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HDFFile.h"

namespace vamiga {

class HDZFile : public HardDiskImage {

public:

    HDFFile hdf;
    
    static optional<ImageInfo> about(const fs::path &path);

    
    //
    // Initializing
    //
    
public:

    HDZFile(const fs::path &path) { init(path); }
    HDZFile(const u8 *buf, isize len) { init(buf, len); }
    HDZFile(const class HDFFile &hdf) { init(hdf); }

    using AnyImage::init;
    void init(const class HDFFile &hdf);


    //
    // Methods from Hashable
    //

public:

    u64 hash(HashAlgorithm algorithm) const override {
        return hdf.hash(algorithm);
    }


    //
    // Methods from AnyImage
    //

public:

    bool validateURL(const fs::path& path) const noexcept override {
        return about(path).has_value();
    }

    ImageType type() const noexcept override { return ImageType::HARDDISK; }
    ImageFormat format() const noexcept override { return ImageFormat::HDZ; }


    //
    // Methods from DiskImage
    //

public:

    FSFamily fsFamily() const noexcept override { return FSFamily::AMIGA; }


    //
    // Methods from AnyFile
    //

public:

    void didLoad() override;

    
    //
    // Methods from DiskImage
    //

public:

    isize numCyls() const override { return hdf.numCyls(); }
    isize numHeads() const override { return hdf.numHeads(); }
    isize numSectors(isize t) const override { return hdf.numSectors(t); }


    //
    // Methods from HardDiskImage
    //

public:

    isize numPartitions() const override { return isize(hdf.ptable.size()); }
    Range<isize> partition(isize nr) const override { return hdf.ptable[nr].range(); }


    //
    // Methods from BlockDevice
    //

public:

    isize capacity() const override { return hdf.numBlocks(); }
    isize bsize() const override { return hdf.bsize(); }
    void readBlock(u8 *dst, isize nr) const override { hdf.readBlock(dst, nr); }
    void writeBlock(const u8 *src, isize nr) override { hdf.writeBlock(src, nr); }


    //
    // Serializing
    //
    
public:
    
    isize writePartitionToFile(const fs::path &path, isize nr) const;
};

}
