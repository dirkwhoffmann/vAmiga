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
#include "DiskEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "DeviceError.h"

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
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    return make_unique<IMGFile>(9 * 160 * 512);
}

}
