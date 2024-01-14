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

Drive::Drive(Amiga& ref, isize n) : SubComponent(ref), nr(n)
{
    assert(usize(nr) < 4);
}

void
Drive::toggleWriteProtection()
{
    // Only proceed if a disk is present
    if (!hasDisk()) return;
    
    // Toggle the protection flag
    setProtectionFlag(hasUnprotectedDisk());
}

}
