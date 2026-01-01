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
#include "FSTypes.h"

namespace vamiga {

class FloppyDisk;

class FloppyDiskImage : public DiskImage {

public:

    static optional<ImageInfo> about(const fs::path& url);
    
    // Floppy disk factory
    static std::unique_ptr<FloppyDiskImage> make(const fs::path &path);

    // Returns the predicted file system of this disk
    virtual FSFormat getDos() const = 0;
    // [[deprecated]] virtual FSFormat getDos() const = 0;

    // Returns the disk diameter and density
    virtual Diameter getDiameter() const = 0;
    virtual Density getDensity() const = 0;

    // Convenience wrappers
    bool isSD() { return getDensity() == Density::SD; }
    bool isDD() { return getDensity() == Density::DD; }
    bool isHD() { return getDensity() == Density::HD; }
 };

}
