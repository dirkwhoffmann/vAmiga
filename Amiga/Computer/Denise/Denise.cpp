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
        
        { &clock,            sizeof(clock),            0 },
        { &sprhstrt,         sizeof(sprhstrt),         WORD_ARRAY },
        { &sprShiftReg,      sizeof(sprShiftReg),      WORD_ARRAY },
        { &sprDmaState,      sizeof(sprDmaState),      DWORD_ARRAY },
        { &attach,           sizeof(attach),           0 },
        { &armed,            sizeof(armed),            0 },

        { &bplcon0,          sizeof(bplcon0),          0 },
        { &bplcon1,          sizeof(bplcon1),          0 },
        { &bplcon2,          sizeof(bplcon2),          0 },
        { &bpldat,           sizeof(bpldat),           WORD_ARRAY },
        { &sprdata,          sizeof(sprdata),          WORD_ARRAY },
        { &sprdatb,          sizeof(sprdatb),          WORD_ARRAY },

        { &shiftReg,         sizeof(shiftReg),         DWORD_ARRAY },

        { &scrollLoresOdd,   sizeof(scrollLoresOdd),   0 },
        { &scrollLoresEven,  sizeof(scrollLoresEven),  0 },
        { &scrollHiresOdd,   sizeof(scrollHiresOdd),   0 },
        { &scrollHiresEven,  sizeof(scrollHiresEven),  0 },

        { &ham,              sizeof(ham),              0 },

        { &firstCanvasPixel, sizeof(firstCanvasPixel), 0 },
        { &lastCanvasPixel,  sizeof(lastCanvasPixel),  0 },
        { &currentPixel,     sizeof(currentPixel),     0 },
        { &inDisplayWindow,  sizeof(inDisplayWindow),  0 },

    });

    longFrame1.data = new int[HPIXELS * VPIXELS];
    longFrame2.data = new int[HPIXELS * VPIXELS];
    shortFrame1.data = new int[HPIXELS * VPIXELS];
    shortFrame2.data = new int[HPIXELS * VPIXELS];
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
    clock = 0;
    workingLongFrame = &longFrame1;
    workingShortFrame = &shortFrame1;
    stableLongFrame = &longFrame2;
    stableShortFrame = &shortFrame2;
    frameBuffer = &longFrame1;
    currentPixel = 0;
    
    // Initialize frame buffers with a recognizable debug pattern
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {

            int pos = line * HPIXELS + i;
            int col = (line / 4) % 2 == (i / 8) % 2 ? 0x00222222 : 0x00444444;
            longFrame1.data[pos] = longFrame2.data[pos] = col;
            shortFrame1.data[pos] = shortFrame2.data[pos] = col;
        }
    }
}

void
Denise::_powerOff()
{
    /* REMOVE AFTER DEBUGGING
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {
            screenBuffer1[line * HPIXELS + i] =
            screenBuffer2[line * HPIXELS + i] = 0x000000FF;
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
    
    // Biplane information
    info.bplcon0 = bplcon0;
    info.bplcon1 = bplcon1;
    info.bplcon2 = bplcon2;
    info.bpu = (bplcon0 >> 12) & 0b111;

    info.diwstrt = agnus->diwstrt;
    info.diwstop = agnus->diwstop;
    info.hstrt = agnus->hstrt;
    info.hstop = agnus->hstop;
    info.vstrt = agnus->vstrt;
    info.vstop = agnus->vstop;

    info.joydat[0] = peekJOY0DATR();
    info.joydat[1] = peekJOY1DATR();
    info.clxdat = 0;

    for (unsigned i = 0; i < 6; i++) {
        info.bpldat[i] = bpldat[i];
    }
    for (unsigned i = 0; i < 32; i++) {
        info.colorReg[i] = colorizer.peekColorReg(i);
        info.color[i] = colorizer.getRGBA(i);
    }
    
    // Sprite information
    
    for (unsigned i = 0; i < 8; i++) {

        /* The sprite info is extracted from the pos and ctl values that are
         * recorded by the hsync handler at the beginning of rasterline 26.
         *
         * pos:  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Hx = HSTART)
         *       E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Ex = VSTART)
         * ctl:  L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
         */
        uint16_t pos = info.sprite[i].pos;
        uint16_t ctl = info.sprite[i].ctl;

        info.sprite[i].hstrt = ((pos & 0x00FF) << 1) | (ctl & 0b001);
        info.sprite[i].vstrt = ((pos & 0xFF00) >> 8) | ((ctl & 0b100) << 6);
        info.sprite[i].vstop = ((ctl & 0xFF00) >> 8) | ((ctl & 0b010) << 7);
        info.sprite[i].attach = GET_BIT(ctl, 7);
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

void
Denise::didLoadFromBuffer(uint8_t **buffer)
{
    // The values in the color registers may have changed. To make the change
    // visible, we need to recompute the RGBA patterns based on the new values.
    colorizer.updateColorTable();
}

uint16_t
Denise::peekJOY0DATR()
{
    return amiga->controlPort1.joydat();
}

uint16_t
Denise::peekJOY1DATR()
{
    return amiga->controlPort2.joydat();
}

void
Denise::pokeJOYTEST(uint16_t value)
{
    /*
    value     &= 0b1111110011111100;
    
    joydat[0] &= 0b0000001100000011;
    joydat[0] |= value;

    joydat[1] &= 0b0000001100000011;
    joydat[1] |= value;
    */
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(BPL_DEBUG, "pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;

    // Determine the number of bitplanes
    uint8_t bpu = bplconBPU();

    // Let Agnus know about the register change
    agnus->activeBitplanes = bpu;
    agnus->hsyncActions |= HSYNC_BPLCON0;

    // Update the DMA time slot allocation table (hires / lores may change).
    // agnus->forceUpdateBitplaneDma();

    // Clear data registers of all inactive bitplanes.
    for (int plane = 5; plane >= bpu; plane--) bpldat[plane] = 0;

    /* "Bit 11 of register BPLCON0 selects hold-and-modify mode. The following
     *  bits in BPLCONO must be set for hold-and-modify mode to be active:
     *
     *      - Bit HOMOD, bit 11, is 1.
     *      - Bit DBLPF, bit 10, is 0 (single-playfield mode specified).
     *      - Bit HIRES, bit 15, is 0 (low-resolution mode specified).
     *      - Bits BPU2, BPU1, and BPUO - bits 14, 13, and 12, are 101 or 110
     *        (five or six bit-planes active)." [HRM]
     */
    ham = (bplcon0 & 0x8C00) == 0x0800 && (bpu == 5 || bpu == 6);

    // shiftReg[0] = 0xAAAA;
    // shiftReg[1] = 0xCCCC;
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
    
    bplcon2 = value;
}

void
Denise::pokeBPLxDAT(int x, uint16_t value)
{
    assert(x < 6);
    debug(BPL_DEBUG, "pokeBPL%dDATA(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

void
Denise::pokeSPRxPOS(int x, uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.

    sprhstrt[x] = ((value & 0xFF) << 1) | (sprhstrt[x] & 0x01);
    
    // Update debugger info
    if (agnus->vpos == 25) {
        info.sprite[x].pos = value;
    }
}

void
Denise::pokeSPRxCTL(int x, uint16_t value)
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
    if (agnus->vpos == 25) {
        info.sprite[x].ctl = value;
        info.sprite[x].ptr = agnus->sprpt[x];
    }
}

void
Denise::pokeSPRxDATA(int x, uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dDATA(%X)\n", x, value);
    
    sprdata[x] = value;
    armSprite(x);
}

void
Denise::pokeSPRxDATB(int x, uint16_t value)
{
    assert(x < 8);
    debug(SPR_DEBUG, "pokeSPR%dDATB(%X)\n", x, value);
    
    sprdatb[x] = value;
}

void
Denise::armSprite(int x)
{
    SET_BIT(armed, x);

    // sprShiftReg[x] = HI_W_LO_W(sprdatb[x], sprdata[x]);
}

void
Denise::prepareShiftRegisters()
{
    for (unsigned i = 0; i < 6; shiftReg[i++] = 0);

#ifdef SHIFTREG_DEBUG
    shiftReg[0] = 0xAAAAAAAA;
    shiftReg[1] = 0xCCCCCCCC;
#endif
}

void
Denise::fillShiftRegisters()
{
    shiftReg[0] = (shiftReg[0] << 16) | bpldat[0];
    shiftReg[1] = (shiftReg[1] << 16) | bpldat[1];
    shiftReg[2] = (shiftReg[2] << 16) | bpldat[2];
    shiftReg[3] = (shiftReg[3] << 16) | bpldat[3];
    shiftReg[4] = (shiftReg[4] << 16) | bpldat[4];
    shiftReg[5] = (shiftReg[5] << 16) | bpldat[5];
}

int *
Denise::pixelAddr(int pixel)
{
    assert(pixel < HPIXELS);

    int offset = pixel + agnus->vpos * HPIXELS;
    assert(offset < PIXELS);
    
    return frameBuffer->data + offset;
}

int
Denise::draw()
{
    int drawnPixels;

    if (lores()) {

        ham ? draw32HAM() : draw32();
        drawnPixels = 32;

    } else {

        draw16();
        drawnPixels = 16;
    }

#ifdef BORDER_DEBUG
    shiftReg[0] = 0xAAAAAAAA;
    shiftReg[1] = 0xCCCCCCCC;
    shiftReg[2] = 0x0;
    shiftReg[3] = 0x0;
    shiftReg[4] = 0x0;
    shiftReg[5] = 0x0;
#endif

    return drawnPixels;
}

void
Denise::newDrawLores(int pixels)
{
    assert(currentPixel == (agnus->hpos * 4) + 6);

    // Check if the vertical position is inside the drawing area
    if (inDisplayWindow) { ham ? draw32HAM(pixels) : draw32(pixels); }
}

void
Denise::newDrawHires(int pixels)
{
    assert(currentPixel == (agnus->hpos * 4) + 6);

    // Check if the vertical position is inside the drawing area
    if (inDisplayWindow) { draw16(pixels); }
}

void
Denise::draw16(int pixels)
{
    int *ptr = pixelAddr(currentPixel);

    uint32_t maskOdd = 0x8000 << scrollHiresOdd;
    uint32_t maskEven = 0x8000 << scrollHiresEven;

    for (int i = 0; i < pixels; i++) {
        
        // Read a bit slice
        uint8_t index =
        (!!(shiftReg[0] & maskOdd)  << 0) |
        (!!(shiftReg[1] & maskEven) << 1) |
        (!!(shiftReg[2] & maskOdd)  << 2) |
        (!!(shiftReg[3] & maskEven) << 3) |
        (!!(shiftReg[4] & maskOdd)  << 4) |
        (!!(shiftReg[5] & maskEven) << 5);
        
        maskOdd >>= 1;
        maskEven >>= 1;

        // Draw a single hires pixel
        uint32_t rgba = colorizer.getRGBA(index * inDisplayWindow);

        *ptr++ = rgba;
    }

#ifdef PIXEL_DEBUG
    *pixelAddr(pixel) = 0x000000FF;
#endif

    currentPixel += pixels;
}

void
Denise::draw32(int pixels)
{
    int *ptr = pixelAddr(currentPixel);

    uint32_t maskOdd = 0x8000 << scrollLoresOdd;
    uint32_t maskEven = 0x8000 << scrollLoresEven;

    for (int i = 0; i < pixels; i++) {
        
        // Read a bit slice
        uint8_t index =
        (!!(shiftReg[0] & maskOdd)  << 0) |
        (!!(shiftReg[1] & maskEven) << 1) |
        (!!(shiftReg[2] & maskOdd)  << 2) |
        (!!(shiftReg[3] & maskEven) << 3) |
        (!!(shiftReg[4] & maskOdd)  << 4) |
        (!!(shiftReg[5] & maskEven) << 5);

        maskOdd >>= 1;
        maskEven >>= 1;

        // Draw two lores pixels
        uint32_t rgba = colorizer.getRGBA(index * inDisplayWindow);

        *ptr++ = rgba;
        *ptr++ = rgba;
    }

#ifdef PIXEL_DEBUG
    *pixelAddr(pixel) = 0x000000FF;
#endif

    currentPixel += 2 * pixels;
}

void
Denise::draw32HAM(int pixels)
{
    int *ptr = pixelAddr(currentPixel);

    uint32_t maskOdd = 0x8000 << scrollLoresOdd;
    uint32_t maskEven = 0x8000 << scrollLoresEven;

    for (int i = 0; i < pixels; i++) {

        // Read a bit slice
        uint8_t index =
        (!!(shiftReg[0] & maskOdd)  << 0) |
        (!!(shiftReg[1] & maskEven) << 1) |
        (!!(shiftReg[2] & maskOdd)  << 2) |
        (!!(shiftReg[3] & maskEven) << 3) |
        (!!(shiftReg[4] & maskOdd)  << 4) |
        (!!(shiftReg[5] & maskEven) << 5);

        maskOdd >>= 1;
        maskEven >>= 1;

        // Draw two lores pixels
        uint32_t rgba = colorizer.computeHAM(index * inDisplayWindow);

        *ptr++ = rgba;
        *ptr++ = rgba;
    }

#ifdef PIXEL_DEBUG
    *pixelAddr(pixel) = 0x000000FF;
#endif

    currentPixel += 2 * pixels;
}

void
Denise::drawSprites()
{
    int nr = 0, col = 17;

    while (armed != 0) {

        if (armed & 0x1) {

            int16_t pixel = 2 * sprhstrt[nr] + 2;
            if (pixel >= HPIXELS - 33) { pixel = HPIXELS - 33; } // ????
            int *ptr = pixelAddr(pixel);

            int rgba[4];
            rgba[1] = colorizer.getRGBA(col);
            rgba[2] = colorizer.getRGBA(col + 1);
            rgba[3] = colorizer.getRGBA(col + 2);

            for (int i = 15; i >= 0; i--) {

                int colNr = !!GET_BIT(sprdata[nr], i) << 1;
                colNr |=    !!GET_BIT(sprdatb[nr], i);

                if (colNr) {
                    *ptr++ = rgba[colNr];
                    *ptr++ = rgba[colNr];
                } else {
                    ptr += 2;
                }
            }
        }
        armed >>= 1;
        col += (nr & 1) << 2;
        nr++;
    }
}

void
Denise::drawBorder()
{
#ifndef BORDER_DEBUG
    int rgba = colorizer.getRGBA(0);
    int rgbaHBorderL = rgba;
    int rgbaHBorderR = rgba;
    int rgbaVBorder  = rgba;
#else
    int rgbaHBorderL = 0x00000044;
    int rgbaHBorderR = 0x00000088;
    int rgbaVBorder  = 0x000000CC;
#endif

    int *ptr = pixelAddr(0);

    // Draw vertical border
    if (!inDisplayWindow) {
        for (int i = FIRST_VISIBLE; i <= LAST_VISIBLE; i++) {
            ptr[i] = rgbaVBorder;
        }

    } else {

        // Draw left border
        // debug("hstrt = %d hstop = %d\n", _agnus->hstrt, _agnus->hstop);
        for (int i = FIRST_VISIBLE; i < (2 * agnus->hstrt); i++) {
            ptr[i] = rgbaHBorderL;
        }

        // Draw right border
        for (int i = (2 * agnus->hstop); i <= LAST_VISIBLE; i++) {
            ptr[i] = rgbaHBorderR;
        }
    }

#ifdef LINE_DEBUG
    if (agnus->vpos == 37) {
        for (int i = 0; i < 256; i++) { ptr[i] = 0x00FFFF00; }
    }
#endif
}

void
Denise::newDrawBorder()
{
#ifndef BORDER_DEBUG
    int rgba = colorizer.getRGBA(0);
    int rgbaBorderL = rgba;
    int rgbaBorderR = rgba;
    int rgbaBorderV = rgba;
    int openL = rgba;
    int openR = rgba;
#else
    int rgbaBorderL = 0x00000044;
    int rgbaBorderR = 0x00000088;
    int rgbaBorderV = 0x000000CC;
    int openL = 0x00888800;
    int openR = 0x00AAAA00;
#endif

    int *ptr = pixelAddr(0);

    /*
    if (!agnus->bitplaneDMA || !inDisplayWindow) {

        // Fill the whole line with the background color
        for (int i = FIRST_VISIBLE; i <= LAST_VISIBLE; i++) {
            ptr[i] = rgbaVBorder;
        }
    */
    if (firstCanvasPixel == 0) {
        assert(lastCanvasPixel == 0);

        // Fill the whole line with the background color
        for (int i = FIRST_VISIBLE; i <= LAST_VISIBLE; i++) {
            ptr[i] = rgbaBorderV;
        }

    } else {

        // Draw left border
        for (int i = FIRST_VISIBLE; i < 2 * agnus->hstrt; i++) {
            ptr[i] = rgbaBorderL;
        }
        for (int i = (2 * agnus->hstrt); i < firstCanvasPixel; i++) {
            ptr[i] = openL;
        }

        // Draw right border
        for (int i = (2 * agnus->hstop); i <= LAST_VISIBLE; i++) {
            ptr[i] = rgbaBorderR;
        }
        for (int i = currentPixel; i < 2 * agnus->hstop; i++) {
            ptr[i] = openR;
        }
    }

#ifdef LINE_DEBUG
    if (agnus->vpos == 37) {
        for (int i = 0; i < 256; i++) { ptr[i] = 0x00FFFF00; }
    }
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

    if (vpos >= 26) {
        
        // Fill the rest of the current line with the background color.
        // drawRightBorder();

        // Draw sprites if one or more of them is armed.
        if (armed) drawSprites();

        // Draw border
#ifdef DEPRECATED_RES
        drawBorder();
#else
        newDrawBorder();
#endif
    }

    // Invoke the DMA debugger
    agnus->dmaDebugger.computeOverlay();

    // Initialize the HAM color storage with the background color.
    colorizer.prepareForHAM();
}

void
Denise::prepareForNextFrame(bool longFrame, bool interlace)
{
    assert(workingLongFrame == &longFrame1 || workingLongFrame == &longFrame2);
    assert(workingShortFrame == &shortFrame1 || workingShortFrame == &shortFrame2);
    assert(stableLongFrame == &longFrame1 || stableLongFrame == &longFrame2);
    assert(stableShortFrame == &shortFrame1 || stableShortFrame == &shortFrame2);
    assert(workingLongFrame != stableLongFrame);
    assert(workingShortFrame != stableShortFrame);
    assert(frameBuffer == workingLongFrame || frameBuffer == workingShortFrame);

    // pthread_mutex_lock(&lock);

    if (frameBuffer == &longFrame1 || frameBuffer == &longFrame2) {

        workingLongFrame = stableLongFrame;
        stableLongFrame = frameBuffer;
        frameBuffer = interlace ? workingShortFrame : workingLongFrame;

    } else {

        assert(frameBuffer == &shortFrame1 || frameBuffer == &shortFrame2);
        workingShortFrame = stableShortFrame;
        stableShortFrame = frameBuffer;
        frameBuffer = workingLongFrame;

    }

    // debug("long = %d interlace = %d\n", frameBuffer->longFrame, frameBuffer->interlace);
    frameBuffer->longFrame = longFrame;
    frameBuffer->interlace = interlace;

    agnus->dmaDebugger.vSyncHandler();
    // pthread_mutex_unlock(&lock);
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
