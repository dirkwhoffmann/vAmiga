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

    amiga.emu = emu;
    amiga.amiga = &emu->main;

    agnus.emu = emu;
    agnus.agnus = &emu->main.agnus;

    blitter.emu = emu;
    blitter.blitter = &emu->main.agnus.blitter;

    copper.emu = emu;
    copper.copper = &emu->main.agnus.copper;

    cpu.emu = emu;
    cpu.cpu = &emu->main.cpu;

    ciaA.emu = emu;
    ciaA.cia = &emu->main.ciaA;

    ciaB.emu = emu;
    ciaB.cia = &emu->main.ciaB;

    denise.emu = emu;
    denise.denise = &emu->main.denise;

    mem.emu = emu;
    mem.mem = &emu->main.mem;


}

VAmiga::~VAmiga()
{
    // emu->halt();
    delete emu;
}

string
VAmiga::version()
{
    return Amiga::version();
}

string
VAmiga::build()
{
    return Amiga::build();
}

}
