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
    
    static bool isCompatible(const fs::path &path);

    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    HDZFile(const fs::path &path) { init(path); }
    HDZFile(const u8 *buf, isize len) { init(buf, len); }
    HDZFile(const class HDFFile &hdf) { init(hdf); }

    void init(const class HDFFile &hdf);


    //
    // Methods from Hashable
    //

public:

    u64 hash(HashAlgorithm algorithm) const override {
        return hdf.hash(algorithm);
    }


    //
    // Methods from AnyFile
    //
    

public:

    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    void finalizeRead() override;

    
    //
    // Methods from DiskImage
    //

public:

    isize numCyls() const override { return hdf.numCyls(); }
    isize numHeads() const override { return hdf.numHeads(); }
    isize numSectors(isize t) const override { return hdf.numSectors(t); }


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
