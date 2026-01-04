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

string
CoreObject::prefix(LogLevel level, const std::source_location &loc) const
{
    const isize verbosity = 2;

    switch (verbosity) {

        case 0:  return "";
        case 1:  return std::format("{}: ", objectName());

        default:
            return std::format("{}:{}: ", objectName(), loc.line());
    }
}

void
CoreObject::dump(Category category, std::ostream &ss) const
{
    _dump(category, ss);
}

void
CoreObject::dump(Category category) const
{
    dump(category, std::cout);
}

}
