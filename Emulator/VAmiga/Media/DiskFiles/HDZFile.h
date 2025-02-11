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

class HDZFile : public DiskFile {

public:

    HDFFile hdf;
    
    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);
    
    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    HDZFile(const std::filesystem::path &path) throws { init(path); }
    HDZFile(const u8 *buf, isize len) throws { init(buf, len); }
    
    const char *objectName() const override { return "HDZ"; }

    
    //
    // Methods from AnyFile
    //
    
    FileType type() const override { return FileType::HDZ; }
    u64 fnv64() const override { return hdf.fnv64(); }
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }
    void finalizeRead() throws override;

    
    //
    // Methods from DiskFile
    //

    isize numCyls() const override { return hdf.numCyls(); }
    isize numHeads() const override { return hdf.numHeads(); }
    isize numSectors() const override { return hdf.numSectors(); }
};

}
