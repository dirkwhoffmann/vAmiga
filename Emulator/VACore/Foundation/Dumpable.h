// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DumpableTypes.h"
#include <iostream>

namespace vamiga {

class Dumpable {

public:

    virtual ~Dumpable() = default;
    virtual void _dump(Category category, std::ostream &ss) const { }

    void dump(Category category, std::ostream &ss) const;
    void dump(Category category) const;
};

}
