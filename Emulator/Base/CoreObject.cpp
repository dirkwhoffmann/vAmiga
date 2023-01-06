// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CoreObject.h"
#include <iostream>

namespace vamiga {

bool
CoreObject::verbose = true;

void
CoreObject::prefix() const
{
    fprintf(stderr, "%s: ", getDescription());
}

void
CoreObject::dump(Category category, std::ostream& ss) const
{
    _dump(category, ss);
}

void
CoreObject::dump(Category category) const
{
    dump(category, std::cout);
}

}
