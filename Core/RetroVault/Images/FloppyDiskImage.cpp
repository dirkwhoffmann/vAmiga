// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "rvconfig.h"
#include "FloppyDiskImage.h"
#include "ADFFile.h"
#include "DMSFile.h"
#include "EADFFile.h"
#include "EXEFile.h"
#include "IMGFile.h"
#include "STFile.h"
#include "D64File.h"
#include "utl/io.h"

namespace retro::vault {

using namespace image;

optional<ImageInfo>
FloppyDiskImage::about(const fs::path& url)
{
    if (auto info = ADFFile::about(url))  return info;
    if (auto info = EADFFile::about(url)) return info;
    if (auto info = IMGFile::about(url))  return info;
    if (auto info = STFile::about(url))   return info;
    if (auto info = DMSFile::about(url))  return info;
    if (auto info = EXEFile::about(url))  return info;
    if (auto info = D64File::about(url))  return info;

    return {};
}

unique_ptr<FloppyDiskImage>
FloppyDiskImage::tryMake(const fs::path &path)
{
    unique_ptr<FloppyDiskImage> result;

    if (ADFFile::about(path).has_value())  return make_unique<ADFFile>(path);
    if (EADFFile::about(path).has_value()) return make_unique<EADFFile>(path);
    if (IMGFile::about(path).has_value())  return make_unique<IMGFile>(path);
    if (STFile::about(path).has_value())   return make_unique<STFile>(path);
    if (DMSFile::about(path).has_value())  return make_unique<DMSFile>(path);
    if (EXEFile::about(path).has_value())  return make_unique<EXEFile>(path);
    if (D64File::about(path).has_value())  return make_unique<D64File>(path);

    return nullptr;
}

unique_ptr<FloppyDiskImage>
FloppyDiskImage::make(const fs::path &path)
{
    if (!utl::fileExists(path))
        throw IOError(IOError::FILE_NOT_FOUND, path.string());
        
    if (auto img = tryMake(path))
        return img;

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

unique_ptr<FloppyDiskImage>
FloppyDiskImage::make(const u8 *buf, isize len, ImageFormat fmt)
{
    switch (fmt) {

        case ImageFormat::ADF:  return make_unique<ADFFile>(buf, len);
        case ImageFormat::EADF: return make_unique<EADFFile>(buf, len);
        case ImageFormat::IMG:  return make_unique<IMGFile>(buf, len);
        case ImageFormat::ST:   return make_unique<STFile>(buf, len);
        case ImageFormat::DMS:  return make_unique<DMSFile>(buf, len);
        case ImageFormat::EXE:  return make_unique<EXEFile>(buf, len);
        case ImageFormat::D64:  return make_unique<D64File>(buf, len);

        case ImageFormat::UNKNOWN:
        case ImageFormat::HDF:
        case ImageFormat::HDZ:
            break;
    }

    throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
}

string
FloppyDiskImage::getDiameterStr() const noexcept
{
    switch (getDiameter()) {

        case Diameter::INCH_35:  return "3.5\"";
        case Diameter::INCH_525: return "5.25\"";
        case Diameter::INCH_8:   return "8\"";
    }

    return "???";
}

string
FloppyDiskImage::getDensityStr() const noexcept
{
    switch (getDensity()) {

        case Density::SD: return "SD";
        case Density::DD: return "DD";
        case Density::HD: return "HD";
    }

    return "???";
}

}
