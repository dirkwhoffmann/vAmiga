// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

int dirk = 0;

Denise::Denise()
{
    setDescription("Denise");
    
    subComponents = vector<HardwareComponent *> {
        
        &pixelEngine,
    };
}

Denise::~Denise()
{
    debug(2, "Destroying Denise[%p]\n", this);
}

void
Denise::_initialize()
{
    agnus = &amiga->agnus;
}

void
Denise::_powerOn()
{
    memset(rasterline, 0, sizeof(rasterline));
}

void
Denise::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    // Biplane information
    info.bplcon0 = bplcon0;
    info.bplcon1 = bplcon1;
    info.bplcon2 = bplcon2;
    info.bpu = (bplcon0 >> 12) & 0b111;

    info.diwstrt = agnus->diwstrt;
    info.diwstop = agnus->diwstop;
    info.diwHstrt = agnus->diwHstrt;
    info.diwHstop = agnus->diwHstop;
    info.diwVstrt = agnus->diwVstrt;
    info.diwVstop = agnus->diwVstop;

    info.joydat[0] = amiga->controlPort1.joydat();
    info.joydat[1] = amiga->controlPort2.joydat();
    info.clxdat = 0;

    for (unsigned i = 0; i < 6; i++) {
        info.bpldat[i] = bpldat[i];
    }
    for (unsigned i = 0; i < 32; i++) {
        info.colorReg[i] = pixelEngine.getColor(i);
        info.color[i] = pixelEngine.getRGBA(i);
    }
    
    // Sprite information
    
    for (int i = 0; i < 8; i++) {

        /* The sprite info is extracted from the pos and ctl values that are
         * recorded by the hsync handler at the beginning of rasterline 26.
         *
         * pos:  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Hx = HSTART)
         *       E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Ex = VSTART)
         * ctl:  L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
         */
        uint16_t pos = info.sprite[i].pos;
        uint16_t ctl = info.sprite[i].ctl;
        info.sprite[i].ptr = agnus->sprpt[i]; 
        info.sprite[i].hstrt = ((pos & 0x00FF) << 1) | (ctl & 0b001);
        info.sprite[i].vstrt = ((pos & 0xFF00) >> 8) | ((ctl & 0b100) << 6);
        info.sprite[i].vstop = ((ctl & 0xFF00) >> 8) | ((ctl & 0b010) << 7);
        info.sprite[i].attach = GET_BIT(ctl, 7);

        // debug("%d: hstrt = %d vstsrt = %d vstop = %d\n", i, info.sprite[i].hstrt, info.sprite[i].vstrt, info.sprite[i].vstop);
    }

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

SpriteInfo
Denise::getSprInfo(int nr)
{
    SpriteInfo result;
    
    pthread_mutex_lock(&lock);
    result = info.sprite[nr];
    pthread_mutex_unlock(&lock);
    
    return result;
}

uint16_t
Denise::peekJOY0DATR()
{
    uint16_t result = amiga->controlPort1.joydat();
    debug(2, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

uint16_t
Denise::peekJOY1DATR()
{
    uint16_t result = amiga->controlPort2.joydat();
    debug(2, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(uint16_t value)
{
    debug(2, "pokeJOYTEST(%04X)\n", value);

    amiga->controlPort1.pokeJOYTEST(value);
    amiga->controlPort2.pokeJOYTEST(value);
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPLCON0(%X)\n", value);

    // Only proceed if the value changes
    if (bplcon0 != value) pokeBPLCON0(bplcon0, value);
}

void
Denise::pokeBPLCON0(uint16_t oldValue, uint16_t newValue)
{
    // Schedule the register to be updated inside the pixel engine
    pixelEngine.conRegHistory.recordChange(BPLCON0, newValue, 4 * agnus->pos.h + 4);
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPLCON1(%X)\n", value);

    bplcon1 = value & 0xFF;

    // Compute scroll values
    scrollLoresOdd  = (bplcon1 & 0b00001111);
    scrollLoresEven = (bplcon1 & 0b11110000) >> 4;
    scrollHiresEven = (bplcon1 & 0b00000111) << 1;
    scrollHiresOdd  = (bplcon1 & 0b01110000) >> 3;
}

void
Denise::pokeBPLCON2(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPLCON2(%X)\n", value);

    // Schedule the register to be updated inside the pixel engine
    pixelEngine.conRegHistory.recordChange(BPLCON2, value, 4 * agnus->pos.h + 4);

    bplcon2 = value;
}

template <int x> void
Denise::pokeBPLxDAT(uint16_t value)
{
    assert(x < 6);
    debug(BPL_DEBUG, "pokeBPL%dDATA(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

template <int x> void
Denise::pokeSPRxPOS(uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.

    sprhstrt[x] = ((value & 0xFF) << 1) | (sprhstrt[x] & 0x01);
    
    // Update debugger info
    if (agnus->pos.v == 26) {
        info.sprite[x].pos = value;
    }
}

template <int x> void
Denise::pokeSPRxCTL(uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.
    
    sprhstrt[x] = (sprhstrt[x] & 0x1FE) | (value & 0x01);
    WRITE_BIT(attach, x, GET_BIT(value, 7));
    
    // Update debugger info
    if (agnus->pos.v == 26) {
        info.sprite[x].ctl = value;
        info.sprite[x].ptr = agnus->sprpt[x];
        assert(IS_EVEN(info.sprite[x].ptr));
    }
}

template <int x> void
Denise::pokeSPRxDATA(uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dDATA(%X)\n", x, value);
    
    sprdata[x] = value;
    armSprite(x);
}

template <int x> void
Denise::pokeSPRxDATB(uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dDATB(%X)\n", x, value);
    
    sprdatb[x] = value;
}

/*
void
Denise::pokeCOLORx(int x, uint16_t value)
{
    assert(x < 32);
    debug(COL_DEBUG, "pokeCOLOR%(%X)\n", x, value);

    colorizer.recordRegisterChange(0x180 + 2*x, value & 0xFFF, 4 * agnus->pos.h);
}
*/

void
Denise::pokeColorReg(uint32_t addr, uint16_t value)
{
    assert(addr >= 0x180 && addr <= 0x1BE && IS_EVEN(addr));
    debug(COL_DEBUG, "pokeColorReg(%X, %X)\n", addr, value);

    pixelEngine.conRegHistory.recordChange(addr, value, 4 * agnus->pos.h);
}

void
Denise::armSprite(int x)
{
    SET_BIT(armed, x);
}

template <int HIRES> void
Denise::draw(int pixels)
{
    uint8_t index;

    currentPixel = ppos(agnus->pos.h);

    uint32_t maskOdd = 0x8000 << scrollLoresOdd;
    uint32_t maskEven = 0x8000 << scrollLoresEven;

    for (int i = 0; i < pixels; i++) {

        // Read a bit slice
        index =
        (!!(shiftReg[0] & maskOdd)  << 0) |
        (!!(shiftReg[1] & maskEven) << 1) |
        (!!(shiftReg[2] & maskOdd)  << 2) |
        (!!(shiftReg[3] & maskEven) << 3) |
        (!!(shiftReg[4] & maskOdd)  << 4) |
        (!!(shiftReg[5] & maskEven) << 5);

        maskOdd >>= 1;
        maskEven >>= 1;

        if (HIRES) {

            // Synthesize one hires pixel
            assert(currentPixel < sizeof(rasterline));
            rasterline[currentPixel++] = index;

        } else {

            // Synthesize two lores pixels
            assert(currentPixel + 1 < sizeof(rasterline));
            rasterline[currentPixel++] = index;
            rasterline[currentPixel++] = index;
        }
    }

    // Shift out drawn bits
    for (int i = 0; i < 6; i++) shiftReg[i] <<= pixels;

#ifdef PIXEL_DEBUG
    rasterline[currentPixel - 2 * pixels] = 64;
#endif
}

#if 0
void
Denise::drawHires(int pixels)
{
    uint8_t index;

    currentPixel = ppos(agnus->pos.h);

    uint32_t maskOdd = 0x8000 << scrollHiresOdd;
    uint32_t maskEven = 0x8000 << scrollHiresEven;

    for (int i = 0; i < pixels; i++) {

        // Read a bit slice
        index =
        (!!(shiftReg[0] & maskOdd)  << 0) |
        (!!(shiftReg[1] & maskEven) << 1) |
        (!!(shiftReg[2] & maskOdd)  << 2) |
        (!!(shiftReg[3] & maskEven) << 3) |
        (!!(shiftReg[4] & maskOdd)  << 4) |
        (!!(shiftReg[5] & maskEven) << 5);

        maskOdd >>= 1;
        maskEven >>= 1;

        // Synthesize a hires pixel
        assert(currentPixel < sizeof(rasterline));
        rasterline[currentPixel++] = index;
    }

    // Shift out drawn bits
    for (int i = 0; i < 6; i++) shiftReg[i] <<= pixels;

#ifdef PIXEL_DEBUG
    rasterline[currentPixel - pixels] = 64;
#endif
}
#endif

void
Denise::drawSprites()
{
    // Only proceed if we are not inside the upper or lower border area
    if (!agnus->inBplDmaArea()) return;

    for (int nr = 0; armed != 0; nr++, armed >>= 1) {

        if (armed & 0x1) {

            int baseCol = 16 + 2 * (nr & 6);
            int16_t pos = 2 * sprhstrt[nr] + 2;

            for (int i = 0; i < 16; i++, pos += 2) {

                int col = (sprdata[nr] >> (14 - i)) & 2;
                col |=    (sprdatb[nr] >> (15 - i)) & 1;

                if (col) {
                    if (pos < LAST_PIXEL) rasterline[pos] = baseCol + col;
                    if (pos < LAST_PIXEL) rasterline[pos+1] = baseCol + col;
                }
            }
        }
    }
}

void
Denise::drawBorder()
{
    int borderL = 0;
    int borderR = 0;
    int borderV = 0;

#ifdef BORDER_DEBUG
    borderL = 64;
    borderR = 65;
    borderV = 66;
#endif

    // Check if the horizontal flipflop was set somewhere in this rasterline
    bool hFlopWasSet = agnus->hFlop || agnus->hFlopOn != -1;

    // Check if the whole line is blank (drawn in background color)
    bool lineIsBlank = !agnus->vFlop || !hFlopWasSet;

    // Draw the border
    if (lineIsBlank) {

        for (int i = 0; i <= LAST_PIXEL; i++) {
           rasterline[i] = borderV;
        }

    } else {

        // Draw left border
        if (!agnus->hFlop && agnus->hFlopOn != -1) {
            for (int i = 0; i < 2 * agnus->hFlopOn; i++) {
                assert(i < sizeof(rasterline));
                rasterline[i] = borderL;
            }
        }

        // Draw right border
        if (agnus->hFlopOff != -1) {
            for (int i = 2 * agnus->hFlopOff; i <= LAST_PIXEL; i++) {
                assert(i < sizeof(rasterline));
                rasterline[i] = borderR;
            }
        }
    }

#ifdef LINE_DEBUG
    int16_t vpos = agnus->pos.v;
    bool lines = vpos == 300 || vpos == 0xA0; // vpos == 26 || vpos == 0x50 || vpos == 276 || vpos == 255;
    if (lines) for (int i = 0; i <= LAST_PIXEL / 2; rasterline[i++] = 64);
#endif
}

void
Denise::beginOfLine(int vpos)
{
    // Reset the horizontal pixel counter
    currentPixel = (agnus->dmaFirstBpl1Event * 4) + 6;
}

void
Denise::endOfLine(int vpos)
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);

    // Make sure we're below the VBLANK area
    if (vpos >= 26) {

        // Draw sprites if one or more of them is armed.
        if (armed) drawSprites();

        // Draw border pixels
        drawBorder();

        // Synthesize RGBA values and write into the frame buffer
        pixelEngine.translateToRGBA(rasterline, vpos);

        /* Note that Denise has already synthesized pixels that belong to the
         * next DMA line (i.e., the pixels that have been written into the
         * rasterline array with offset values > $E2). We move them to the
         * beginning of the rasterline array to make the appear when the next
         * line is drawn.
         */
        for (int i = 4 * 0xE3; i < sizeof(rasterline); i++) {
            rasterline[i - 4 * 0xE3] = rasterline[i];
            rasterline[i] = 0;
        }
    }

    // Invoke the DMA debugger
    agnus->dmaDebugger.computeOverlay();
}

void
Denise::debugSetBPU(int count)
{
    if (count < 0) count = 0;
    if (count > 6) count = 6;

    amiga->suspend();
    
    uint16_t value = bplcon0 & 0b1000111111111111;
    pokeBPLCON0(value | (count << 12));
    
    amiga->resume();
}

void
Denise::debugSetBPLCONx(unsigned x, uint16_t value)
{
    assert(x <= 2);

    amiga->suspend();

    switch (x) {
        case 0:
            pokeBPLCON0(value);
            break;
        case 1:
            pokeBPLCON1(value);
            break;
        case 2:
            pokeBPLCON2(value);
            break;
    }

    amiga->resume();
}

void
Denise::debugSetBPLCONxBit(unsigned x, unsigned bit, bool value)
{
    assert(x <= 2);
    assert(bit <= 15);

    uint16_t mask = 1 << bit;

    amiga->suspend();

    switch (x) {
        case 0:
            pokeBPLCON0(value ? (bplcon0 | mask) : (bplcon0 & ~mask));
            break;
        case 1:
            pokeBPLCON1(value ? (bplcon1 | mask) : (bplcon1 & ~mask));
            break;
        case 2:
            pokeBPLCON2(value ? (bplcon2 | mask) : (bplcon2 & ~mask));
            break;
    }
    
    amiga->resume();
}

void
Denise::debugSetBPLCONxNibble(unsigned x, unsigned nibble, uint8_t value)
{
    assert(x <= 2);
    assert(nibble <= 4);

    uint16_t mask = 0b1111 << (4 * nibble);
    uint16_t bits = (value & 0b1111) << (4 * nibble);

    amiga->suspend();

    switch (x) {
        case 0:
            pokeBPLCON0((bplcon0 & ~mask) | bits);
            break;
        case 1:
            pokeBPLCON1((bplcon1 & ~mask) | bits);
            break;
        case 2:
            pokeBPLCON2((bplcon2 & ~mask) | bits);
            break;
    }

    amiga->resume();
}

template void Denise::pokeBPLxDAT<0>(uint16_t value);
template void Denise::pokeBPLxDAT<1>(uint16_t value);
template void Denise::pokeBPLxDAT<2>(uint16_t value);
template void Denise::pokeBPLxDAT<3>(uint16_t value);
template void Denise::pokeBPLxDAT<4>(uint16_t value);
template void Denise::pokeBPLxDAT<5>(uint16_t value);

template void Denise::pokeSPRxPOS<0>(uint16_t value);
template void Denise::pokeSPRxPOS<1>(uint16_t value);
template void Denise::pokeSPRxPOS<2>(uint16_t value);
template void Denise::pokeSPRxPOS<3>(uint16_t value);
template void Denise::pokeSPRxPOS<4>(uint16_t value);
template void Denise::pokeSPRxPOS<5>(uint16_t value);
template void Denise::pokeSPRxPOS<6>(uint16_t value);
template void Denise::pokeSPRxPOS<7>(uint16_t value);

template void Denise::pokeSPRxCTL<0>(uint16_t value);
template void Denise::pokeSPRxCTL<1>(uint16_t value);
template void Denise::pokeSPRxCTL<2>(uint16_t value);
template void Denise::pokeSPRxCTL<3>(uint16_t value);
template void Denise::pokeSPRxCTL<4>(uint16_t value);
template void Denise::pokeSPRxCTL<5>(uint16_t value);
template void Denise::pokeSPRxCTL<6>(uint16_t value);
template void Denise::pokeSPRxCTL<7>(uint16_t value);

template void Denise::pokeSPRxDATA<0>(uint16_t value);
template void Denise::pokeSPRxDATA<1>(uint16_t value);
template void Denise::pokeSPRxDATA<2>(uint16_t value);
template void Denise::pokeSPRxDATA<3>(uint16_t value);
template void Denise::pokeSPRxDATA<4>(uint16_t value);
template void Denise::pokeSPRxDATA<5>(uint16_t value);
template void Denise::pokeSPRxDATA<6>(uint16_t value);
template void Denise::pokeSPRxDATA<7>(uint16_t value);

template void Denise::pokeSPRxDATB<0>(uint16_t value);
template void Denise::pokeSPRxDATB<1>(uint16_t value);
template void Denise::pokeSPRxDATB<2>(uint16_t value);
template void Denise::pokeSPRxDATB<3>(uint16_t value);
template void Denise::pokeSPRxDATB<4>(uint16_t value);
template void Denise::pokeSPRxDATB<5>(uint16_t value);
template void Denise::pokeSPRxDATB<6>(uint16_t value);
template void Denise::pokeSPRxDATB<7>(uint16_t value);

template void Denise::draw<0>(int pixels);
template void Denise::draw<1>(int pixels);
