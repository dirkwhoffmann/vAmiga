// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Debugger.h"
#include "Amiga.h"
#include "IOUtils.h"
#include <sstream>

namespace vamiga {

void
Debugger::stopAndGo()
{
    isRunning() ? amiga.pause() : amiga.run();
}

void
Debugger::stepInto()
{
    if (isRunning()) return;

    cpu.debugger.stepInto();
    amiga.run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

void
Debugger::stepOver()
{
    if (isRunning()) return;

    cpu.debugger.stepOver();
    amiga.run();

    // Inform the GUI
    msgQueue.put(MSG_STEP);
}

}
