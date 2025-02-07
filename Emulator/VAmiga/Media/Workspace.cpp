// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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

void
Workspace::init(const std::filesystem::path &path)
{
    // Only proceed if the provided filename points to a directory
    if (!isCompatiblePath(path)) throw CoreException(CoreError::FILE_TYPE_MISMATCH);
    
    this->path = path;
}

}
