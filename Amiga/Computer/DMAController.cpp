// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DMAController::DMAController()
{
    setDescription("DMAController");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,  sizeof(clock), 0 },
        { &vhpos,  sizeof(vhpos), 0 },
        { &vpos,   sizeof(vpos),  0 },
    });
}

DMAController::~DMAController()
{
    debug(2, "Destroying DMAController[%p]\n", this);
}

void
DMAController::_powerOn()
{
    clock = 0;
}

void
DMAController::_powerOff()
{
    
}
void
DMAController::_reset()
{
    
}
void
DMAController::_ping()
{
    
}
void
DMAController::_dump()
{
    plainmsg("   clock: %lld\n", clock);
    plainmsg("   vhpos: %lld $%llX\n", vhpos, vhpos);
    plainmsg("    vpos: %lld $%llX\n", vpos, vpos);
}

void
DMAController::executeUntil(uint64_t targetClock)
{
    // Deterimine number of master clock cycles to execute
    uint64_t missingCycles = targetClock - clock;

    // Convert to DMA cycles
    uint64_t missingDMACycles = AS_DMA_CYCLE(missingCycles);

    // Execute missing cycles
    for (uint64_t i = 0; i < missingDMACycles; i++) {
        
        switch (vhpos) {
            case 0x07:
            case 0x09:
            case 0x0B:
                // Do disk DMA
                break;
                
            case 0x0D:
            case 0x0F:
            case 0x11:
            case 0x13:
                // Do Audio DMA
                break;
                
            // AND SO ON ...
                
            default:
                break;
                
        }
        
        // Check if the rasterline end has been reached
        if (vhpos < 227) vhpos++; else hsyncAction();
    }
    
    // Note the completed cycles
    clock += DMA_CYCLE(missingDMACycles);
}

void
DMAController::hsyncAction()
{
    vhpos = 0;
    
    // CIA B counts HSYNCs
    amiga->ciaB.incrementTOD();
    
    // Check if the frame end has been reached
    if (vpos < 312) vpos++; else vsyncAction();
}

void
DMAController::vsyncAction()
{
    vpos = 0;
    
    // CIA A counts VSYNCs
    amiga->ciaA.incrementTOD();
}
