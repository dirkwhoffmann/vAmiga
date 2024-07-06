// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Drive.h"

namespace vamiga {

void
Drive::toggleWriteProtection()
{
    // Only proceed if a disk is present
    if (!hasDisk()) return;
    
    // Toggle the protection flag
    setFlag(FLAG_PROTECTED, !getFlag(FLAG_PROTECTED));
}

}
