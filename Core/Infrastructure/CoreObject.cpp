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

/*
isize
CoreObject::verbosity = 3;
*/

CoreObject::CoreObject()
{
    verbosity = 3;
}

void
CoreObject::prefix(long level, const void *sender, long line) const
{
    auto *obj = dynamic_cast<CoreObject *>((CoreObject *)sender);

    if (level == 1) {
        fprintf(stderr, "%s: ", obj ? obj->objectName() : "");
    }
    if (level >= 2) {
        fprintf(stderr, "%s:%ld ", obj ? obj->objectName() : "", line);
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
