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
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &bplcon0,   sizeof(bplcon0),   0 },
        { &bplcon1,   sizeof(bplcon1),   0 },
        { &bplcon2,   sizeof(bplcon2),   0 },
        { &bpldat,    sizeof(bpldat),    WORD_ARRAY },
        { &colorReg,  sizeof(colorReg),  WORD_ARRAY },
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
        info.color[i] = colorRGBA[i];

    return info;
}

void
Denise::didLoadFromBuffer(uint8_t **buffer)
{
    // Update the RGBA color values by poking into the color registers
    for (unsigned i = 0; i < 32; i++) pokeCOLORxx(i, colorReg[i]);
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug("pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug("pokeBPLCON1(%X)\n", value);
    
    bplcon1 = value;
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
Denise::pokeCOLORxx(int xx, uint16_t value)
{
    assert(xx < 32);
    debug("pokeCOLOR%02d(%X)\n", xx, value);
    
    // Write into the color register
    colorReg[xx] = value;
    
    // Translate the color into a GPU compatible RGBA format
    
    // Extract RGB from: xx xx xx xx R3 R2 R1 R0 G3 G2 G1 G0 B3 B2 B1 B0
    uint8_t r = (colorReg[xx] >> 8) & 0xF;
    uint8_t g = (colorReg[xx] >> 4) & 0xF;
    uint8_t b = colorReg[xx] & 0xF;

    // At this point we can apply arbitrary color adjustments
    // For now, we do a simple scaling
    r <<= 4;
    g <<= 4;
    b <<= 4;

    // Compose the RGBA quadrupels
    colorRGBA[xx] = HI_HI_LO_LO(r, g, b, 0xFF);
    colorRGBAhb[xx] = HI_HI_LO_LO(r / 2, g / 2, b / 2, 0xFF);
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

