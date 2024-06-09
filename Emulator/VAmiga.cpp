// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "VAmiga.h"
#include "Emulator.h"

namespace vamiga {

void
API::suspend()
{
    // emu->suspend();
}

void
API::resume()
{
    // emu->resume();
}

bool
API::isUserThread() const
{
    // return !emu->isEmulatorThread();
    return false;
}

VAmiga::VAmiga() {

    emu = new Emulator();
}

VAmiga::~VAmiga()
{
    // emu->halt();
    delete emu;
}

}
