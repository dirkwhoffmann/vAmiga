// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Amiga.h"

namespace vamiga {

class Emulator {

public:

    // The virtual Amiga
    // Amiga main = Amiga(*this, 0);
    Amiga main = Amiga(*this);
};

}

