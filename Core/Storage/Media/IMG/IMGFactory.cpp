// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IMGFactory.h"
#include "IMGEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"

namespace vamiga {

std::unique_ptr<IMGFile>
IMGFactory::make(const fs::path &path)
{
    return std::make_unique<IMGFile>(path);
}

std::unique_ptr<IMGFile>
IMGFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<IMGFile>(buf, len);
}

std::unique_ptr<IMGFile>
IMGFactory::make(Diameter dia, Density den)
{
    if (dia != Diameter::INCH_35 || den != Density::DD) {

        // We only support 3.5"DD disks at the moment
        throw AppError(AppError::DISK_INVALID_LAYOUT);
    }

    return make_unique<IMGFile>(9 * 160 * 512);
}

std::unique_ptr<IMGFile>
IMGFactory::make(const class FloppyDisk &disk)
{
    auto img = make(disk.getDiameter(), disk.getDensity());
    IMGEncoder::decode(*img, disk);
    return img;
}

std::unique_ptr<IMGFile>
IMGFactory::make(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw AppError(AppError::DISK_MISSING);
    return make(*drive.disk);
}

}
