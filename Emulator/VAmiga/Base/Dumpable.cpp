// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Dumpable.h"

namespace vamiga {

void
Dumpable::dump(Category category, std::ostream& ss) const
{
    _dump(category, ss);
}

void
Dumpable::dump(Category category) const
{
    dump(category, std::cout);
}

}
