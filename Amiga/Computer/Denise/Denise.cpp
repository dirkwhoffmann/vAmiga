// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Denise::Denise()
{
    setDescription("Denise");
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &colorizer,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &bplcon0,   sizeof(bplcon0),   0 },
        { &bplcon1,   sizeof(bplcon1),   0 },
        { &bplcon2,   sizeof(bplcon2),   0 },
        { &bpldat,    sizeof(bpldat),    WORD_ARRAY },
    });
    
}

Denise::~Denise()
{
    debug(2, "Destroying Denise[%p]\n", this);
}

void
Denise::_powerOn()
{
    clock = 0;
    frame = 0;
    frameBuffer = longFrame;
    pixelBuffer = frameBuffer;
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

DeniseInfo
Denise::getInfo()
{
    DeniseInfo info;

    info.bplcon0 = bplcon0;
    info.bplcon1 = bplcon1;
    info.bplcon2 = bplcon2;
    
    for (unsigned i = 0; i < 32; i++)
        info.bpldat[i] = bpldat[i];
    
    for (unsigned i = 0; i < 32; i++)
        info.color[i] = colorizer.getRGBA(i);

    return info;
}

void
Denise::didLoadFromBuffer(uint8_t **buffer)
{
    // Update the RGBA color values by poking into the color registers
    colorizer.updateRGBAs();
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug("pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;
    
    // The number of active bitplanes is managed by Agnus
    amiga->dma.activeBitplanes = (value >> 12) & 0b111;
    
    // Clear data registers of inactive bitplanes
    for (int plane = 5; plane >= amiga->dma.activeBitplanes; plane--) {
        bpldat[plane] = 0;
    }
    
    
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug("pokeBPLCON1(%X)\n", value);
    
    bplcon1 = value & 0xFF;
    
    uint16_t ddfstrt = amiga->dma.ddfstrt;

    // Compute scroll values (adapted from WinFellow)
    scrollLowOdd  = (bplcon1        + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiOdd   = ((scrollLowOdd  + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
    scrollLowEven = ((bplcon1 >> 4) + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiEven  = ((scrollLowEven + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
}

void
Denise::pokeBPLCON2(uint16_t value)
{
    debug("pokeBPLCON2(%X)\n", value);
    
    bplcon2 = value;
}

void
Denise::pokeBPLxDAT(int x, uint16_t value)
{
    assert(x < 6);
    debug("pokeBPL%dDAT(%X)\n", x, value);
    
    bpldat[x] = value;
}

void
Denise::serviceEvent(EventID id, int64_t data)
{
  
}

void
Denise::fillShiftRegisters()
{
    warn("IMPLEMENTATION MISSING");
}

void
Denise::endOfFrame()
{
    // Switch the active frame buffer
    frameBuffer = (frameBuffer == longFrame) ? shortFrame : longFrame;
    pixelBuffer = frameBuffer;
    
    // Toggle the fake image from time to time
    if ((frame / 25) % 2) {
        memcpy((void *)longFrame, fakeImage1, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage1, BUFSIZE);
    } else {
        memcpy((void *)longFrame, fakeImage2, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage2, BUFSIZE);
    }
    
    // Take a snapshot once in a while
    if (amiga->getTakeAutoSnapshots() && amiga->getSnapshotInterval() > 0) {
        unsigned fps = 50;
        if (frame % (fps * amiga->getSnapshotInterval()) == 0) {
            amiga->takeAutoSnapshot();
        }
    }
    
    // Count some sheep (zzzzzz) ...
    if (!amiga->getWarp()) {
        amiga->synchronizeTiming();
    }
    
    frame++;
}

