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
#include "ADFFile.h"
#include "ADZFile.h"
#include "DMSFile.h"
#include "EADFFile.h"
#include "EXEFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "utl/io.h"

namespace vamiga {

std::unique_ptr<FloppyDiskImage>
FloppyDiskImage::make(const fs::path &path)
{
    std::unique_ptr<FloppyDiskImage> result;

    if (ADFFile::isCompatible(path))   return make_unique<ADFFile>(path);
    if (ADZFile::isCompatible(path))   return make_unique<ADZFile>(path);
    if (EADFFile::isCompatible(path))  return make_unique<EADFFile>(path);
    if (IMGFile::isCompatible(path))   return make_unique<IMGFile>(path);
    if (STFile::isCompatible(path))    return make_unique<STFile>(path);
    if (DMSFile::isCompatible(path))   return make_unique<DMSFile>(path);
    if (EXEFile::isCompatible(path))   return make_unique<EXEFile>(path);

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

}
