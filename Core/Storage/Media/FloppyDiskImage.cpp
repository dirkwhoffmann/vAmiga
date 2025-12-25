// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FloppyDiskImage.h"

namespace vamiga {

GeometryDescriptor
FloppyDiskImage::getGeometry() const
{
    return GeometryDescriptor(numCyls(), numHeads(), numSectors(0), bsize());
}

}
