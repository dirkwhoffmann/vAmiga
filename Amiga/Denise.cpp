// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Denise.h"

Denise::Denise()
{
    setDescription("Denise");
}

Denise::~Denise()
{
    debug(2, "Destroying Denise[%p]\n", this);
}

void
Denise::_powerOn()
{
    frame = 0;
    currentScreenBuffer = longFrame;
    pixelBuffer = longFrame;
}
void
Denise::_powerOff()
{
    
}
void
Denise::_reset()
{
    
}
void
Denise::_ping()
{
    
}
void
Denise::_dump()
{
    msg("Frame: %lld\n", frame);
}


// FAKE STUFF

void *
Denise::screenBuffer()
{
    frame++;
    if ((frame / 10) % 2) {
        return longFrame;
    } else {
        return shortFrame;
    }
}
