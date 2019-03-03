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

        { &bltafwm,    sizeof(bltafwm),    0 },
        { &bltalwm,    sizeof(bltalwm),    0 },

        { &bltsize,    sizeof(bltsize),    0 },

        { &bltamod,    sizeof(bltamod),    0 },
        { &bltbmod,    sizeof(bltbmod),    0 },
        { &bltcmod,    sizeof(bltcmod),    0 },
        { &bltdmod,    sizeof(bltdmod),    0 },

        { &bltadat,    sizeof(bltadat),    0 },
        { &bltbdat,    sizeof(bltbdat),    0 },
        { &bltcdat,    sizeof(bltcdat),    0 },

    });
}

BlitterInfo
Blitter::getInfo()
{
    BlitterInfo info;
    
    /* Note: We call the Blitter 'active' if there is a pending message in the
     * Copper event slot.
     */
    info.active  = amiga->dma.eventHandler.isPending(BLT_SLOT);
    info.bltcon0 = bltcon0;
    info.bltcon1 = bltcon1;
    info.bltafwm = bltafwm;
    info.bltalwm = bltalwm;
    info.bltsize = bltsize;
    info.bltamod = bltamod;
    info.bltbmod = bltbmod;
    info.bltcmod = bltcmod;
    info.bltdmod = bltdmod;
    info.bltadat = bltadat;
    info.bltbdat = bltbdat;
    info.bltcdat = bltcdat;
    
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
    plainmsg("   bltafwm: %X\n", bltafwm);
    plainmsg("   bltalwm: %X\n", bltalwm);
    plainmsg("   bltsize: %X\n", bltsize);
    plainmsg("   bltamod: %X\n", bltamod);
    plainmsg("   bltbmod: %X\n", bltbmod);
    plainmsg("   bltcmod: %X\n", bltcmod);
    plainmsg("   bltdmod: %X\n", bltdmod);
    plainmsg("   bltadat: %X\n", bltadat);
    plainmsg("   bltbdat: %X\n", bltbdat);
    plainmsg("   bltcdat: %X\n", bltcdat);
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
Blitter::pokeBLTAFWM(uint16_t value)
{
    debug("pokeBLTAFWM(%X)\n", value);
    bltafwm = value;
}

void
Blitter::pokeBLTALWM(uint16_t value)
{
    debug("pokeBLTALWM(%X)\n", value);
    bltalwm = value;
}

void
Blitter::pokeBLTSIZE(uint16_t value)
{
    debug("pokeBLTSIZE(%X)\n", value);
    bltsize = value;
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
