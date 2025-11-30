// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HDZFactory.h"

namespace vamiga {

std::unique_ptr<HDZFile>
HDZFactory::make(const fs::path &path)
{
    return std::make_unique<HDZFile>(path);
}

std::unique_ptr<HDZFile>
HDZFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<HDZFile>(buf, len);
}

std::unique_ptr<HDZFile>
HDZFactory::make(const HDFFile &hdf)
{
    return std::make_unique<HDZFile>(hdf);
}

std::unique_ptr<HDZFile>
HDZFactory::make(const std::unique_ptr<HDFFile>& hdf)
{
    return make(*hdf);
}

}
