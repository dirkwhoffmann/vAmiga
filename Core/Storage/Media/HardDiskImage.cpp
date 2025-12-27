// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HardDiskImage.h"
#include "HDFFactory.h"
#include "HDZFactory.h"
#include "utl/io.h"

namespace vamiga {

std::unique_ptr<HardDiskImage>
HardDiskImage::make(const fs::path &path)
{
    std::unique_ptr<HardDiskImage> result;

    if (HDFFile::isCompatible(path))   return HDFFactory::make(path);
    if (HDZFile::isCompatible(path))   return HDZFactory::make(path);

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

}
