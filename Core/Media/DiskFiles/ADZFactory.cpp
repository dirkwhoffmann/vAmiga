// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADZFactory.h"
#include "ADFFactory.h"
#include "FloppyDisk.h"
#include "FloppyDrive.h"
#include <utility>

namespace vamiga {

std::unique_ptr<ADZFile>
ADZFactory::make(const fs::path &path)
{
    return std::make_unique<ADZFile>(path);
}

std::unique_ptr<ADZFile>
ADZFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<ADZFile>(buf, len);
}

std::unique_ptr<ADZFile>
ADZFactory::make(const ADFFile &adf)
{
    return std::make_unique<ADZFile>(adf);
}

std::unique_ptr<ADZFile>
ADZFactory::make(const std::unique_ptr<ADFFile>& adf)
{
    return make(*adf);
}

std::unique_ptr<ADZFile>
ADZFactory::make(const class FloppyDisk &disk)
{
    return make(ADFFactory::make(disk));
}

std::unique_ptr<ADZFile>
ADZFactory::make(const class FloppyDrive &drive)
{
    return make(ADFFactory::make(drive));
}

}
