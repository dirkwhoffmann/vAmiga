// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskController::DiskController()
{
    setDescription("DiskController");
}

void
DiskController::_powerOn()
{
    
}

void
DiskController::_powerOff()
{
    
}

void
DiskController::_reset()
{
    
}

void
DiskController::_ping()
{
    amiga->putMessage(doesDMA(0) ? MSG_DRIVE_DMA_ON : MSG_DRIVE_DMA_OFF, 0);
    amiga->putMessage(doesDMA(1) ? MSG_DRIVE_DMA_ON : MSG_DRIVE_DMA_OFF, 1);
}

void
DiskController::_dump()
{
}
