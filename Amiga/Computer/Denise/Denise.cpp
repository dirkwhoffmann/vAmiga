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
        
        { &clock,         sizeof(clock),         0 },
        { &bplcon0,       sizeof(bplcon0),       0 },
        { &bplcon1,       sizeof(bplcon1),       0 },
        { &bplcon2,       sizeof(bplcon2),       0 },
        { &bpldat,        sizeof(bpldat),        WORD_ARRAY },
        { &sprpos,        sizeof(sprpos),        WORD_ARRAY },
        { &sprctl,        sizeof(sprctl),        WORD_ARRAY },
        
        { &joydat,        sizeof(joydat),        WORD_ARRAY },
        { &shiftReg,      sizeof(shiftReg),      DWORD_ARRAY },
        { &scrollLowEven, sizeof(scrollLowEven), 0 },
        { &scrollLowOdd,  sizeof(scrollLowOdd),  0 },
        { &scrollHiEven,  sizeof(scrollHiEven),  0 },
        { &scrollHiOdd,   sizeof(scrollHiOdd),   0 },

        { &pixel,         sizeof(pixel),   0 },

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
    frameBuffer = longFrame;
    rasterline = frameBuffer;
    
    // Initialize frame buffers with a recognizable debug pattern
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {
            longFrame[line * HPIXELS + i] =
            shortFrame[line * HPIXELS + i] =
            (line % 2) ? 0x000000FF : 0x0000FFFF;
        }
    }
}

void
Denise::_powerOff()
{
    /* REMOVE AFTER DEBUGGING
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {
            longFrame[line * HPIXELS + i] =
            shortFrame[line * HPIXELS + i] = 0x000000FF;
        }
    }
    */
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
Denise::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.bplcon0 = bplcon0;
    info.bplcon1 = bplcon1;
    info.bplcon2 = bplcon2;
    
    for (unsigned i = 0; i < 32; i++)
    info.bpldat[i] = bpldat[i];
    
    for (unsigned i = 0; i < 32; i++)
    info.color[i] = colorizer.getRGBA(i);
    
    pthread_mutex_unlock(&lock);
}

void
Denise::_dump()
{

}

DeniseInfo
Denise::getInfo()
{
    DeniseInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
Denise::didLoadFromBuffer(uint8_t **buffer)
{
    // The values in the color registers may have changed. To make the change
    // visible, we need to recompute the RGBA patterns based on the new values.
    colorizer.updateRGBAs();
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(2, "pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;
    
    // Tell Agnus how many bitplanes we have
    amiga->agnus.activeBitplanes = (value >> 12) & 0b111;

    // Update the DMA time slot allocation table
    amiga->agnus.buildDMAEventTable();

    // Clear data registers of all inactive bitplanes
    for (int plane = 5; plane >= amiga->agnus.activeBitplanes; plane--) {
        bpldat[plane] = 0;
    }
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug(2, "pokeBPLCON1(%X)\n", value);
    
    bplcon1 = value & 0xFF;
    
    uint16_t ddfstrt = amiga->agnus.ddfstrt;

    // Compute scroll values (adapted from WinFellow)
    scrollLowOdd  = (bplcon1        + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiOdd   = ((scrollLowOdd  + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
    scrollLowEven = ((bplcon1 >> 4) + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiEven  = ((scrollLowEven + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
}

void
Denise::pokeBPLCON2(uint16_t value)
{
    debug(2, "pokeBPLCON2(%X)\n", value);
    
    bplcon2 = value;
}

void
Denise::pokeBPLxDAT(int x, uint16_t value)
{
    assert(x < 6);
    // debug(2, "pokeBPL%dDAT(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

void
Denise::pokeSPRxPOS(int x, uint16_t value)
{
    assert(x < 8);
    // debug(2, "pokeSPR%dPOS(%X)\n", x, value);
    
    sprpos[x] = value;
}

void
Denise::pokeSPRxCTL(int x, uint16_t value)
{
    assert(x < 8);
    // debug(2, "pokeSPR%dCTL(%X)\n", x, value);
    
    sprctl[x] = value;
}

uint16_t
Denise::peekJOYxDATR(int x)
{
    assert(x < 2);
    debug(2, "peekJOY%dDATR: %X\n", x, joydat[x]);
    
    return joydat[x];
}

/*
void
Denise::serviceEvent(EventID id, int64_t data)
{
  
}
*/

void
Denise::fillShiftRegisters()
{
    // warn("fillShiftRegisters: IMPLEMENTATION MISSING (vpos = %d)\n", amiga->agnus.vpos);
    // warn("blpdat: %X %X %X %X\n", bpldat[0], bpldat[1], bpldat[2], bpldat[3]);

    shiftReg[0] = bpldat[0];
    shiftReg[1] = bpldat[1];
    shiftReg[2] = 0; // bpldat[2];
    shiftReg[3] = 0; // bpldat[3];
    shiftReg[4] = 0; // bpldat[4];
    shiftReg[5] = 0; // bpldat[5];

    // draw16();
}

void
Denise::draw16()
{
    assert(amiga->agnus.vpos >= 26); // 0 .. 25 is VBLANK area
    
    int16_t vpos = amiga->agnus.vpos - 26;
    int16_t hpos = (amiga->agnus.hpos - 63) * 4; // 2;
    
    // if (amiga->debugDMA) debug("draw16: (%d, %d)\n", vpos, hpos);
    
    // if (hpos > HPIXELS) return;
    // if (vpos > 250) return;
    
    // uint32_t offset = (vpos * HPIXELS) + (2 * hpos);
    uint32_t offset = (vpos * HPIXELS) + hpos;
    if (offset + HPIXELS >= VPIXELS * HPIXELS) {
        // warn("OUT OF RANGE!!!\n");
        return;
        
    }
    
    uint8_t index = 0;
    // uint8_t ind = 0;
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }
        
        // DEBUGGING
        // index = ((hpos + i) == vpos) ? 1 : 0;
        
        // Draw two pixels in lores mode (no hires mode yet)
        uint32_t rgba = colorizer.getRGBA(index);
        rasterline[pixel++] = rgba;
        rasterline[pixel++] = rgba;
    }
}

void
Denise::endOfLine()
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);
    
    // Check for VBLANK area
    if (amiga->agnus.vpos >= 26) {
        
        // Fill the rest of the line with the current background color
        int bgcol = colorizer.getRGBA(0);
        for (; pixel < HPIXELS; pixel++) rasterline[pixel] = bgcol;
    
        // Reset the horizontal pixel counter
        pixel = 0;
        
        // Move on to the next rasterline
        rasterline += HPIXELS;
    }
}

void
Denise::endOfFrame()
{
    // Switch the active frame buffer
    frameBuffer = (frameBuffer == longFrame) ? shortFrame : longFrame;
    rasterline = frameBuffer;
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
