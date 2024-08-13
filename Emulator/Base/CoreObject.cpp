// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CoreObject.h"
#include <iostream>

namespace vamiga {

isize
CoreObject::verbosity = 2;

void
CoreObject::prefix(isize level, const char *component, isize line) const
{
    if (level == 1) {
        fprintf(stderr, "%s: ", component);
    }
    if (level >= 2) {
        fprintf(stderr, "%s:%ld ", component, line);
    }
}

}
