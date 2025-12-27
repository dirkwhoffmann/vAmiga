// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDiskImage.h"

namespace vamiga {

class IMGFile : public FloppyDiskImage {
    
public:

    static constexpr isize IMGSIZE_35_DD = 737280;  // 720 KB PC disk
    
    static bool isCompatible(const fs::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);

    
    //
    // Initializing
    //
    
public:
    
    using AnyFile::init;
    
    IMGFile(const fs::path &path) { init(path); }
    IMGFile(isize len) { init(len); }
    IMGFile(const u8 *buf, isize len) { init(buf, len); }


    //
    // Methods from AnyFile
    //

public:

    bool isCompatiblePath(const fs::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) const override { return isCompatible(buf, len); }
    
    
    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return 512; }


    //
    // Methods from DiskImage
    //

public:

    isize numCyls() const override;
    isize numHeads() const override;
    isize numSectors(isize) const override { return numSectors(); }
    isize numSectors() const;

    
    //
    // Methods from FloppyDiskImage
    //

    FSFormat getDos() const override { return FSFormat::NODOS; }
    Diameter getDiameter() const override { return Diameter::INCH_35; }
    Density getDensity() const override { return Density::DD; }
};

}
