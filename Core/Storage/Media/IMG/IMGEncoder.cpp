// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IMGEncoder.h"
#include "IMGFactory.h"
#include "DiskEncoder.h"
#include "DeviceError.h"
#include "MFM.h"

namespace vamiga {

void
IMGEncoder::encode(const class IMGFile &source, FloppyDisk &disk)
{
    auto img = IMGFactory::make(source.data.ptr, source.data.size);
    disk.encode(*img);
}

void
IMGEncoder::decode(class IMGFile &target, const FloppyDisk &disk)
{
    auto img = IMGFactory::make(target.data.ptr, target.data.size);
    disk.decode(*img);
    target.data = img->data;
}

}
