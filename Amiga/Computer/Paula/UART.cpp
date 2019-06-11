// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

UART::UART()
{
    setDescription("UART");

    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {

        { &serper,     sizeof(serper),     0 },
    });
}

void
UART::_initialize()
{
    events = &amiga->agnus.events;
    paula  = &amiga->paula;
}

void
UART::_powerOn()
{

}

void
UART::_powerOff()
{

}

void
UART::_reset()
{

}

void
UART::_dump()
{
    plainmsg("   serper: %X\n", serper);
}

