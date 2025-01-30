// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Workspace.h"
// #include "Amiga.h"
#include "IOUtils.h"

#include <sstream>

namespace vamiga {

bool
Workspace::isCompatible(const std::filesystem::path &path)
{
    if (!util::isDirectory(path)) return false;
    
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".VAMIGA";
}

/*
bool
Workspace::isCompatible(const u8 *buf, isize len)
{
    return false;
}

bool
Workspace::isCompatible(const Buffer<u8> &buf)
{
    return false;
}
*/

}
