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
#include "D64File.h"
#include "utl/io.h"

namespace vamiga {

optional<ImageInfo>
FloppyDiskImage::about(const fs::path& url)
{
    if (auto info = ADFFile::isCompatible(url))  return info;
    if (auto info = ADZFile::isCompatible(url))  return info;
    if (auto info = EADFFile::isCompatible(url)) return info;
    if (auto info = IMGFile::isCompatible(url))  return info;
    if (auto info = STFile::isCompatible(url))   return info;
    if (auto info = DMSFile::isCompatible(url))  return info;
    if (auto info = EXEFile::isCompatible(url))  return info;
    if (auto info = D64File::isCompatible(url))  return info;

    return {};
}

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
