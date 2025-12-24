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

class HDZFile : public DiskImage {

public:

    HDFFile hdf;
    
    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
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
    // Methods from AnyFile
    //
    
    u64 fnv64() const override { return hdf.fnv64(); }
    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) const override { return isCompatible(buf, len); }
    void finalizeRead() override;

    
    //
    // Methods from DiskFile
    //

    isize numCyls() const override { return hdf.numCyls(); }
    isize numHeads() const override { return hdf.numHeads(); }
    isize numSectors() const override { return hdf.numSectors(); }
    
    
    //
    // Serializing
    //
    
public:
    
    isize writePartitionToFile(const fs::path &path, isize nr) const;
};

}
