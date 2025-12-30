// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"

namespace vamiga {

class DMSFile : public FloppyDiskImage {

public:

    ADFFile adf;

    static bool isCompatible(const fs::path &path);

    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    DMSFile(const fs::path &path) { init(path); }
    DMSFile(const u8 *buf, isize len) { init(buf, len); }
    
    // const char *objectName() const override { return "DMS"; }
    const ADFFile &getADF() const { return adf; }


    //
    // Methods from Hashable
    //

public:

    u64 hash(HashAlgorithm algorithm) const override {
        return adf.hash(algorithm);
    }


    //
    // Methods from AnyFile
    //

public:
    
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    void finalizeRead() override;


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return adf.bsize(); }
    isize capacity() const override { return adf.capacity(); }
    void readBlock(u8 *dst, isize nr) const override { adf.readBlock(dst, nr); }
    void writeBlock(const u8 *src, isize nr) override { adf.writeBlock(src, nr); };


    //
    // Methods from TrackDevice
    //

public:

    isize numCyls() const override { return adf.numCyls(); }
    isize numHeads() const override { return adf.numHeads(); }
    isize numSectors(isize t) const override { return adf.numSectors(t); }


    //
    // Methods from FloppyDiskImage
    //

public:

    FSFormat getDos() const override { return adf.getDos(); }
    Diameter getDiameter() const override { return adf.getDiameter(); }
    Density getDensity() const override { return adf.getDensity(); }
};

}
