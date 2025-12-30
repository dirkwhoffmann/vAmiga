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

class ADZFile : public FloppyDiskImage {

public:

    ADFFile adf;

    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    ADZFile(const fs::path &path) { init(path); }
    ADZFile(const u8 *buf, isize len) { init(buf, len); }
    ADZFile(const class ADFFile &adf) { init(adf); }
    
    void init(const class ADFFile &adf);
    
    
    //
    // Methods from AnyFile
    //

public:
    
    u64 fnv64() const override { return adf.fnv64(); }
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) const override { return isCompatible(buf, len); }
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
