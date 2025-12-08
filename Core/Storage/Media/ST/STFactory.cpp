// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "STFactory.h"
#include "STEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "DeviceError.h"

namespace vamiga {

std::unique_ptr<STFile>
STFactory::make(const fs::path &path)
{
    return std::make_unique<STFile>(path);
}

std::unique_ptr<STFile>
STFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<STFile>(buf, len);
}

std::unique_ptr<STFile>
STFactory::make(Diameter dia, Density den)
{
    if (dia != Diameter::INCH_35 || den != Density::DD) {

        // We only support 3.5"DD disks at the moment
        throw DeviceError(DeviceError::DSK_INVALID_LAYOUT);
    }

    return make_unique<STFile>(9 * 160 * 512);
}

std::unique_ptr<STFile>
STFactory::make(const class FloppyDisk &disk)
{
    auto st = make(disk.getDiameter(), disk.getDensity());
    STEncoder::decode(*st, disk);
    return st;
}

std::unique_ptr<STFile>
STFactory::make(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return make(*drive.disk);
}

}
