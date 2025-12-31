// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "STFile.h"
#include "FloppyDisk.h"
#include "DeviceError.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

namespace vamiga {

bool
STFile::isCompatible(const fs::path &path)
{
    // Check suffix
    auto suffix = utl::uppercased(path.extension().string());
    if (suffix != ".ST") return false;

    // Check file size
    auto size = utl::getSizeOfFile(path);
    return size == STSIZE_35_DD;
}

void
STFile::init(Diameter dia, Density den)
{
    if (dia != Diameter::INCH_35 || den != Density::DD) {

        // We only support 3.5"DD disks at the moment
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    init(9 * 160 * 512);
}

isize
STFile::numCyls() const
{
    return 80;
}

isize
STFile::numHeads() const
{
    return 2;
}

isize
STFile::numSectors() const
{
    return 9;
}

}
