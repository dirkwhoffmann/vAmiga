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

namespace vamiga {

class HardDiskImage : public DiskImage {

    // Hard disk factory
    static std::unique_ptr<HardDiskImage> make(const fs::path &path);

};

}
