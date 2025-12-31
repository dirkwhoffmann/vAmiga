// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Script.h"
#include "Amiga.h"
#include "utl/support/Strings.h"
#include <sstream>

namespace vamiga {

bool
Script::isCompatible(const fs::path &path)
{
    auto suffix = utl::uppercased(path.extension().string());

    return suffix == ".RETROSH";
}

}
