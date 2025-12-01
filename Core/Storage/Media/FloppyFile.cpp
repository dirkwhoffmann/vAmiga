// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MediaFile.h"
#include "FloppyFile.h"
#include "ADFFile.h"
#include "ADZFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "StringUtils.h"

namespace vamiga {

FloppyDiskDescriptor
FloppyFile::getDescriptor() const
{
    return FloppyDiskDescriptor {

        .diameter = getDiameter(),
        .density = getDensity(),
        .sides = numHeads(),
        .cylinders = numCyls()
    };
}

}
