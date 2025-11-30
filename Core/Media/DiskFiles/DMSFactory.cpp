// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DMSFactory.h"

namespace vamiga {

std::unique_ptr<DMSFile>
DMSFactory::make(const fs::path &path)
{
    return std::make_unique<DMSFile>(path);
}

std::unique_ptr<DMSFile>
DMSFactory::make(const u8 *buf, isize len)
{
    return std::make_unique<DMSFile>(buf, len);
}

}
