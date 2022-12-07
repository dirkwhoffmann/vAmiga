// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaObject.h"
#include <iostream>

namespace vamiga {

bool
AmigaObject::verbose = true;

void
AmigaObject::prefix() const
{
    fprintf(stderr, "%s: ", getDescription());
}

void
AmigaObject::dump(Category category, std::ostream& ss) const
{
    _dump(category, ss);
}

void
AmigaObject::dump(Category category) const
{
    dump(category, std::cout);
}

}
