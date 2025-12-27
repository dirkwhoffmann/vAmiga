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
#include "ADFFactory.h"
#include "ADZFactory.h"
#include "DMSFactory.h"
#include "EADFFactory.h"
#include "EXEFactory.h"
#include "IMGFactory.h"
#include "STFactory.h"
#include "utl/io.h"

namespace vamiga {

std::unique_ptr<FloppyDiskImage>
FloppyDiskImage::make(const fs::path &path)
{
    std::unique_ptr<FloppyDiskImage> result;

    if (ADFFile::isCompatible(path))   return ADFFactory::make(path);
    if (ADZFile::isCompatible(path))   return ADZFactory::make(path);
    if (EADFFile::isCompatible(path))  return EADFFactory::make(path);
    if (IMGFile::isCompatible(path))   return IMGFactory::make(path);
    if (STFile::isCompatible(path))    return STFactory::make(path);
    if (DMSFile::isCompatible(path))   return DMSFactory::make(path);
    if (EXEFile::isCompatible(path))   return EXEFactory::make(path);

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

}
