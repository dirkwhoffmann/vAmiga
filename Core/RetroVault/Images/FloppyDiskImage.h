// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskImage.h"
#include "DeviceTypes.h"
#include "FileSystems/Amiga/FSTypes.h"

using retro::device::Diameter;
using retro::device::Density;

namespace retro::image {

class FloppyDiskImage : public DiskImage {

public:

    // Static functions
    static optional<ImageInfo> about(const fs::path& url);
    static unique_ptr<FloppyDiskImage> tryMake(const fs::path &path);
    static unique_ptr<FloppyDiskImage> make(const fs::path &path);

    // Returns the disk diameter and density
    virtual Diameter getDiameter() const noexcept = 0;
    virtual Density getDensity() const noexcept = 0;

    // Returns a string representation for the diameter and density
    virtual string getDiameterStr() const noexcept;
    virtual string getDensityStr() const noexcept;

    // Convenience wrappers
    bool isSD() const noexcept { return getDensity() == Density::SD; }
    bool isDD() const noexcept { return getDensity() == Density::DD; }
    bool isHD() const noexcept { return getDensity() == Density::HD; }
};

}
