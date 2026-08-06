// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskImage.h"
#include "DeviceTypes.h"

namespace retro::vault {

class FloppyDiskImage : public DiskImage {

public:

    // Static functions
    static optional<ImageInfo> about(const fs::path& url);
    static unique_ptr<FloppyDiskImage> tryMake(const fs::path &path);
    static unique_ptr<FloppyDiskImage> make(const fs::path &path);
    static unique_ptr<FloppyDiskImage> make(const u8 *buf, isize len, ImageFormat fmt);


    //
    // Querying floppy disk properties
    //

public:

    // Returns the disk diameter and density
    virtual Diameter getDiameter() const noexcept = 0;
    virtual Density getDensity() const noexcept = 0;
    bool isSD() const noexcept { return getDensity() == Density::SD; }
    bool isDD() const noexcept { return getDensity() == Density::DD; }
    bool isHD() const noexcept { return getDensity() == Density::HD; }

    // Returns a string representation for the diameter and density
    virtual string getDiameterStr() const noexcept;
    virtual string getDensityStr() const noexcept;

    //
    // Encoding and decoding
    //

public:

    virtual BitView encode(TrackNr t) const = 0;
    virtual void decode(TrackNr t, BitView bits) = 0;
};

}
