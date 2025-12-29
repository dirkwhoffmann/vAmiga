// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EADFFactory.h"
#include "EADFEncoder.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include "DeviceError.h"

namespace vamiga {

std::unique_ptr<EADFFile>
EADFFactory::make(const fs::path &path)
{
    return std::make_unique<EADFFile>(path);
}

std::unique_ptr<EADFFile>
EADFFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<EADFFile>(buf, len);
}

std::unique_ptr<EADFFile>
EADFFactory::make(const class FloppyDisk &disk)
{
    auto numTracks = disk.numTracks();

    auto length = 0;

    length += 12;               // File header
    length += 12 * numTracks;   // Track headers

    for (isize t = 0; t < numTracks; t++) {

        auto numBits = disk.track[t].size();
        assert(numBits % 8 == 0);
        length += numBits / 8;
    }

    auto eadf = make_unique<EADFFile>(length);
    EADFEncoder::decode(*eadf, disk);
    return eadf;
}

std::unique_ptr<EADFFile>
EADFFactory::make(const class FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw DeviceError(DeviceError::DSK_MISSING);
    return make(*drive.disk);
}

}
