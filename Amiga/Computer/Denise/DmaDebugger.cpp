// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DmaDebugger::DmaDebugger()
{
    setDescription("DmaDebugger");
}

bool
DmaDebugger::isVisualized(BusOwner owner)
{
    return true;
}

void
DmaDebugger::setVisualized(BusOwner owner, bool value)
{

}

uint32_t
DmaDebugger::getColor(BusOwner owner)
{
    return 0;
}

void
DmaDebugger::setColor(BusOwner owner, uint32_t color)
{

}

float
DmaDebugger::getOpacity()
{
    return 1.0;
}

void
DmaDebugger::setOpacity(float value)
{

}

void
DmaDebugger::overlayDmaUsage()
{

}
