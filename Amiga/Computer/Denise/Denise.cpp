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
        { &sprdata,       sizeof(sprdata),       WORD_ARRAY },
        { &sprdatb,       sizeof(sprdatb),       WORD_ARRAY },

        { &joydat,        sizeof(joydat),        WORD_ARRAY },
        { &shiftReg,      sizeof(shiftReg),      DWORD_ARRAY },
        
        { &scrollLowEven, sizeof(scrollLowEven), 0 },
        { &scrollLowOdd,  sizeof(scrollLowOdd),  0 },
        { &scrollHiEven,  sizeof(scrollHiEven),  0 },
        { &scrollHiOdd,   sizeof(scrollHiOdd),   0 },

        { &pixel,         sizeof(pixel),         0 },

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

uint16_t
Denise::peekJOYxDATR(int x)
{
    assert(x < 1);
    return joydat[x];
}

void
Denise::pokeJOYTEST(uint16_t value)
{
    value     &= 0b1111110011111100;
    
    joydat[0] &= 0b0000001100000011;
    joydat[0] |= value;

    joydat[1] &= 0b0000001100000011;
    joydat[1] |= value;
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
    debug(2, "pokeBPL%dDATA(%X)\n", x + 1, value);
    
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

void
Denise::pokeSPRxDATA(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dDATA(%X)\n", x, value);
    
    sprdata[x] = value;
}

void
Denise::pokeSPRxDATB(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dDATB(%X)\n", x, value);
    
    sprdatb[x] = value;
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

int *
Denise::pixelAddr(int pixel)
{
    assert(pixel < HPIXELS);
    assert(amiga->agnus.vpos >= 26); // 0 .. 25 is VBLANK area

    int offset = pixel + (amiga->agnus.vpos - 26) * HPIXELS;
    // debug("pixel offset for pixel %d is %d\n", pixel, offset);
    assert(offset < VPIXELS * HPIXELS);
    
    return frameBuffer + offset;
}

void
Denise::draw16()
{
    int *ptr = pixelAddr(pixel);
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        uint8_t index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }
        
        // Draw a single hires pixel
        uint32_t rgba = colorizer.getRGBA(index);
        *ptr++ = rgba;
    }
    pixel += 16;
}

void
Denise::draw32()
{
    int *ptr = pixelAddr(pixel);
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        uint8_t index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }
        
        // Draw two lores pixels
        uint32_t rgba = colorizer.getRGBA(index);
        *ptr++ = rgba;
        *ptr++ = rgba;
    }
    pixel += 32;
}

void
Denise::drawLeftBorder()
{
    if (amiga->agnus.hpos < 0x35) return; // HBLANK area
    
    // Assign the horizontal pixel counter
    // pixel = (amiga->agnus.hpos - (0x35 - 0xF)) * 4;
    pixel = (amiga->agnus.hpos - 0x35) * 4;

    // Fill the beginning of the line with the current background color
    int bgcol = colorizer.getRGBA(0);
    int *ptr  = pixelAddr(0);
    int *end  = pixelAddr(pixel);
    
    while (ptr < end) *ptr++ = bgcol;
}

void
Denise::drawRightBorder()
{
    // Fill the rest of the line with the current background color
    int bgcol = colorizer.getRGBA(0);
    int *ptr  = pixelAddr(pixel);
    int *end  = pixelAddr(HPIXELS - 2);
    
    while (ptr <= end) *ptr++ = bgcol;
    
    /* In the last pixel of each line, we store if this line was drawn in lores
     * or hires mode. This information is used by the uscaling shader to
     * determine if in-texture scaling should be applied.
     */
    *ptr = lores() ? 0x00FFFF00 : 0;
    
    // Reset the horizontal pixel counter
    pixel = 0;
}

void
Denise::endOfLine()
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);
    
    if (amiga->agnus.vpos >= 26) {
        drawRightBorder();
    }
}

void
Denise::endOfFrame()
{
    // Switch the active frame buffer
    frameBuffer = (frameBuffer == longFrame) ? shortFrame : longFrame;
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
