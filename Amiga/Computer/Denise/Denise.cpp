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
        { &sprpos,    sizeof(sprpos),    WORD_ARRAY },
        { &sprctl,    sizeof(sprctl),    WORD_ARRAY },
        
        { &joydat,    sizeof(joydat),    WORD_ARRAY },
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
    // debug("pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;
    
    // Tell Agnus how many bitplanes we have
    amiga->dma.activeBitplanes = (value >> 12) & 0b111;

    // Update the DMA time slot allocation table
    amiga->dma.buildDMAEventTable();

    // Clear data registers of all inactive bitplanes
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
    // debug("pokeBPL%dDAT(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

void
Denise::pokeSPRxPOS(int x, uint16_t value)
{
    assert(x < 8);
    // debug("pokeSPR%dPOS(%X)\n", x, value);
    
    sprpos[x] = value;
}

void
Denise::pokeSPRxCTL(int x, uint16_t value)
{
    assert(x < 8);
    // debug("pokeSPR%dCTL(%X)\n", x, value);
    
    sprctl[x] = value;
}

uint16_t
Denise::peekJOYxDAT(int x)
{
    assert(x < 2);
    debug("peekJOY%dDAT: %X\n", x, joydat[x]);
    
    return joydat[x];
}

void
Denise::serviceEvent(EventID id, int64_t data)
{
  
}

void
Denise::fillShiftRegisters()
{
    // warn("fillShiftRegisters: IMPLEMENTATION MISSING (vpos = %d)\n", amiga->dma.vpos);
    // warn("blpdat: %X %X %X %X\n", bpldat[0], bpldat[1], bpldat[2], bpldat[3]);

    shiftReg[0] = bpldat[0];
    shiftReg[1] = bpldat[1];
    shiftReg[2] = bpldat[2];
    shiftReg[3] = bpldat[3];
    shiftReg[4] = bpldat[4];
    shiftReg[5] = bpldat[5];

    draw16();
}

void
Denise::draw16()
{
    int16_t vpos = amiga->dma.vpos;
    int16_t hpos = amiga->dma.hpos; // - 63;
    
    // debug("draw16: (%d, %d)\n", vpos, hpos);
    
    uint32_t offset = (vpos * HPIXELS) + (hpos * 4);
    if (offset + HPIXELS >= VPIXELS * HPIXELS) {
        // warn("OUT OF RANGE!!!\n");
        return;
        
    }
    
    uint8_t index;
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned i = 0; i < 6; i++) {
            shiftReg[i] <<= 1;
        }

        // index = ((hpos / 2) % 2 == 0) ? 0 : 1;
        
        if (index == 0) {
            pixelBuffer[offset + (2 * i)] = 0;
            pixelBuffer[offset + (2 * i) + 1] = 0;
        } else {
            pixelBuffer[offset + (2 * i)] = 0x000000FF;
            pixelBuffer[offset + (2 * i) + 1] = 0x000000FF;
        }
    }
}

void
Denise::endOfFrame()
{
    bufferoffset = 0;
    
    // Switch the active frame buffer
    frameBuffer = (frameBuffer == longFrame) ? shortFrame : longFrame;
    pixelBuffer = frameBuffer;
    
    // Toggle the fake image from time to time
    /*
    if ((frame / 25) % 2) {
        memcpy((void *)longFrame, fakeImage1, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage1, BUFSIZE);
    } else {
        memcpy((void *)longFrame, fakeImage2, BUFSIZE);
        memcpy((void *)shortFrame, fakeImage2, BUFSIZE);
    }
    */
    
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

void
Denise::debugSetActivePlanes(int count)
{
    assert(count >= 0 && count <= 6);
    
    amiga->suspend();
    
    uint16_t value = bplcon0 & 0b1000111111111111;
    pokeBPLCON0(value | (count << 12));
    
    amiga->resume();
}

void
Denise::debugSetBPLCON0Bit(unsigned bit, bool value)
{
    assert(bit <= 15);
    
    amiga->suspend();
    
    uint16_t mask = 1 << bit;
    pokeBPLCON0(value ? (bplcon0 | mask) : (bplcon0 & ~mask));
    
    amiga->resume();
}
