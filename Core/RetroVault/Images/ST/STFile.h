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

class STFile : public FloppyDiskImage {

public:

    static constexpr isize STSIZE_35_DD = 737280;  // 720 KB Atari ST disk

    static optional<ImageInfo> about(const fs::path &path);


    //
    // Initializing
    //

public:

    STFile(const fs::path &path) { init(path); }
    STFile(isize len) { init(len); }
    STFile(const u8 *buf, isize len) { init(buf, len); }
    STFile(Diameter dia, Density den) { init(dia, den); }

    using AnyImage::init;
    void init(Diameter dia, Density den);


    //
    // Methods from AnyImage
    //

public:

    bool validateURL(const fs::path& path) const noexcept override {
        return about(path).has_value();
    }
    
    ImageType type() const noexcept override { return ImageType::FLOPPY; }
    ImageFormat format() const noexcept override { return ImageFormat::ST; }


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

public:
    
    FSFormat getDos() const override { return FSFormat::NODOS; }
    Diameter getDiameter() const override { return Diameter::INCH_35; }
    Density getDensity() const override { return Density::DD; }
};

}
