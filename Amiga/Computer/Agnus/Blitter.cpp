// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Blitter::Blitter()
{
    setDescription("Blitter");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &bltcon0,    sizeof(bltcon0),    0 },
        { &bltcon1,    sizeof(bltcon1),    0 },
    });
}

BlitterInfo
Blitter::getInfo()
{
    BlitterInfo info;
    
    /* Note: We call the Blitter 'active' if there is a pending message in the
     * Copper event slot.
     */
    
    info.active    = amiga->dma.eventHandler.isPending(BLT_SLOT);
    info.bltcon0   = bltcon0;
    info.bltcon1   = bltcon1;

    return info;
}

void
Blitter::_powerOn()
{
    
}

void
Blitter::_powerOff()
{
    
}

void
Blitter::_reset()
{
    
}

void
Blitter::_ping()
{
    
}

void
Blitter::_dump()
{
    plainmsg("   bltcon0: %X\n", bltcon0);
    plainmsg("   bltcon1: %X\n", bltcon1);
}

void
Blitter::pokeBLTCON0(uint16_t value)
{
    debug("pokeBLTCON0(%X)\n", value);
    
    bltcon0 = value;
}

void
Blitter::pokeBLTCON1(uint16_t value)
{
    debug("pokeBLTCON1(%X)\n", value);
    
    bltcon1 = value;
}

void
Blitter::cancelEvent()
{
    amiga->dma.eventHandler.cancelEvent(BLT_SLOT);
}

void
Blitter::serviceEvent(EventID id, int64_t data)
{
    debug("Servicing Blitter event %d\n", id);
    
    switch (id) {
            
        default:
            
            assert(false);
            break;
    }
}
