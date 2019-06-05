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

    memset(rasterline, 0, sizeof(rasterline));

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
    info.diwHstrt = agnus->diwHstrt;
    info.diwHstop = agnus->diwHstop;
    info.diwVstrt = agnus->diwVstrt;
    info.diwVstop = agnus->diwVstop;

    info.joydat[0] = peekJOY0DATR();
    info.joydat[1] = peekJOY1DATR();
    info.clxdat = 0;

    for (unsigned i = 0; i < 6; i++) {
        info.bpldat[i] = bpldat[i];
    }
    for (unsigned i = 0; i < 32; i++) {
        info.colorReg[i] = colorizer.getColor(i);
        info.color[i] = colorizer.getRGBA(i);
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
    agnus->hsyncActions |= HSYNC_UPDATE_EVENT_TABLE;

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
    if (agnus->vpos == 26) {
        info.sprite[x].ctl = value;
        info.sprite[x].ptr = agnus->sprpt[x];
        assert(IS_EVEN(info.sprite[x].ptr));
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
Denise::pokeCOLORx(int x, uint16_t value)
{
    assert(x < 32);
    debug(COL_DEBUG, "pokeCOLOR%(%X)\n", x, value);

    colorizer.recordColorRegisterChange(x, value & 0xFFF, 4 * agnus->hpos);
}

void
Denise::armSprite(int x)
{
    SET_BIT(armed, x);
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

void
Denise::drawLores(int pixels)
{
    assert(currentPixel == (agnus->hpos * 4) + 6);

    // Only proceed if the vertical position is inside the drawing area
    if (!inDisplayWindow) return;

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
        assert(currentPixel + 1 < sizeof(rasterline));
        rasterline[currentPixel++] = index * inDisplayWindow;
        rasterline[currentPixel++] = index * inDisplayWindow;
    }

#ifdef PIXEL_DEBUG
    rasterline[currentPixel - 16] = 64;
#endif
}

void
Denise::drawHires(int pixels)
{
    assert(currentPixel == (agnus->hpos * 4) + 6);

    // Only proceed if the vertical position is inside the drawing area
    if (!inDisplayWindow) return;

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
        assert(currentPixel < sizeof(rasterline));
        rasterline[currentPixel++] = index * inDisplayWindow;
    }

#ifdef PIXEL_DEBUG
    rasterline[currentPixel - 16] = 64;
#endif
}

void
Denise::drawSprites()
{
    for (int nr = 0; armed != 0; nr++, armed <<= 1) {

        if (armed & 0x1) {

            int colorBase = 16 + 2 * (nr & 6);

            int16_t strt = 2 * sprhstrt[nr] + 2;
            int16_t stop = MIN(strt + 32, LAST_VISIBLE - 1);

            // int16_t pixel = 2 * sprhstrt[nr] + 2;

            for (int i = 0; strt < stop; i++, strt += 2) {

                int offset = (sprdata[nr] >> (14 - i)) & 2;
                offset |=    (sprdatb[nr] >> (15 - i)) & 1;

                if (offset) {
                    assert(strt + 1 < sizeof(rasterline));
                    rasterline[strt] = rasterline[strt+1] = colorBase + offset;
                }
            }
        }
    }
}

void
Denise::drawBorder()
{
#ifndef BORDER_DEBUG
    int borderL = 0;
    int borderR = 0;
    int borderV = 0;
    int openL = 0;
    int openR = 0;
#else
    int borderL = 64;
    int borderR = 65;
    int borderV = 66;
    int openL = 68;
    int openR = 69;
#endif

    if (firstCanvasPixel == 0) {
        assert(lastCanvasPixel == 0);

        // Fill the whole line with the background color
        for (int i = FIRST_VISIBLE; i <= LAST_VISIBLE; i++) {
            rasterline[i] = borderV;
        }

    } else {

        // Draw left border
        for (int i = FIRST_VISIBLE; i < 2 * agnus->diwHstrt; i++) {
            assert(i < sizeof(rasterline));
            rasterline[i] = borderL;
        }
        for (int i = (2 * agnus->diwHstrt); i < firstCanvasPixel; i++) {
            assert(i < sizeof(rasterline));
            rasterline[i] = openL;
        }

        // Draw right border
        for (int i = (2 * agnus->diwHstop); i <= LAST_VISIBLE; i++) {
            assert(i < sizeof(rasterline));
            rasterline[i] = borderR;
        }
        for (int i = currentPixel; i < 2 * agnus->diwHstop; i++) {
            assert(i < sizeof(rasterline));
            rasterline[i] = openR;
        }
    }

#ifdef LINE_DEBUG
    if (agnus->vpos == 256) for (int i = 0; i <= LAST_VISIBLE; rasterline[i++] = 64);
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
        if (ham) {
            colorizer.translateToRGBA_HAM(rasterline, frameBuffer->data + vpos * HPIXELS);
        } else {
            colorizer.translateToRGBA(rasterline, frameBuffer->data + vpos * HPIXELS);
        }
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
