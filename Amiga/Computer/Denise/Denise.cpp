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

Denise::Denise(Amiga& ref) : SubComponent(ref)
{
    setDescription("Denise");
    
    subComponents = vector<HardwareComponent *> {
        
        &pixelEngine,
    };

    config.emulateSprites = true;
    config.clxSprSpr = true;
    config.clxSprPlf = true;
    config.clxPlfPlf = true;
}

void
Denise::_powerOn()
{
}

void
Denise::_reset()
{
    RESET_SNAPSHOT_ITEMS

    memset(bBuffer, 0, sizeof(bBuffer));
    memset(iBuffer, 0, sizeof(iBuffer));
    memset(zBuffer, 0, sizeof(zBuffer));
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
    info.bpu = bpu();

    info.diwstrt = agnus.diwstrt;
    info.diwstop = agnus.diwstop;
    info.diwHstrt = agnus.diwHstrt;
    info.diwHstop = agnus.diwHstop;
    info.diwVstrt = agnus.diwVstrt;
    info.diwVstop = agnus.diwVstop;

    info.joydat[0] = amiga.controlPort1.joydat();
    info.joydat[1] = amiga.controlPort2.joydat();
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
        info.sprite[i].ptr = agnus.sprpt[i];
        info.sprite[i].hstrt = ((pos & 0x00FF) << 1) | (ctl & 0b001);
        info.sprite[i].vstrt = ((pos & 0xFF00) >> 8) | ((ctl & 0b100) << 6);
        info.sprite[i].vstop = ((ctl & 0xFF00) >> 8) | ((ctl & 0b010) << 7);
        info.sprite[i].attach = GET_BIT(ctl, 7);

        // debug("%d: hstrt = %d vstsrt = %d vstop = %d\n", i, info.sprite[i].hstrt, info.sprite[i].vstrt, info.sprite[i].vstop);
    }

    pthread_mutex_unlock(&lock);
}

void
Denise::_dumpConfig()
{
    plainmsg(" emulateSprites: %d\n", config.emulateSprites);
    plainmsg("      clxSprSpr: %d\n", config.clxSprSpr);
    plainmsg("      clxSprPlf: %d\n", config.clxSprPlf);
    plainmsg("      clxPlfPlf: %d\n", config.clxPlfPlf);
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
    uint16_t result = amiga.controlPort1.joydat();
    debug(2, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

uint16_t
Denise::peekJOY1DATR()
{
    uint16_t result = amiga.controlPort2.joydat();
    debug(2, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(uint16_t value)
{
    debug(2, "pokeJOYTEST(%04X)\n", value);

    amiga.controlPort1.pokeJOYTEST(value);
    amiga.controlPort2.pokeJOYTEST(value);
}

/*
void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(BPLREG_DEBUG, "pokeBPLCON0(%X)\n", value);

    if (bplcon0 != value) {

        pokeBPLCON0(bplcon0, value);
        bplcon0 = value;
    }
}
*/

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(BPLREG_DEBUG, "pokeBPLCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), REG_BPLCON0_DENISE, value);
}

void
Denise::setBPLCON0(uint16_t oldValue, uint16_t newValue)
{
    debug(BPLREG_DEBUG, "pokeBPLCON0(%X,%X)\n", oldValue, newValue);

    // Record the register change
    conRegChanges.add(4 * agnus.pos.h - 4, REG_BPLCON0_DENISE, newValue);
    
    // Update value
    bplcon0 = newValue;
}

int
Denise::bpu(uint16_t v)
{
    // Extract the three BPU bits and check for hires mode
    int bpu = (v >> 12) & 0b111;
    bool hires = GET_BIT(v, 15);

    if (hires) {
        return bpu < 5 ? bpu : 0; // Disable all bitplanes if value is invalid
    } else {
        return bpu < 7 ? bpu : 6; // Enable six bitplanes if value is invalid
    }
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug(BPLREG_DEBUG, "pokeBPLCON1(%X)\n", value);

    // Record the register change
    agnus.recordRegisterChange(DMA_CYCLES(2), REG_BPLCON1, value);
}

void
Denise::setBPLCON1(uint16_t value)
{
    debug(BPLREG_DEBUG, "setBPLCON1(%X)\n", value);

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
    debug(BPLREG_DEBUG, "pokeBPLCON2(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(2), REG_BPLCON2, value);
}

void
Denise::setBPLCON2(uint16_t value)
{
    debug(BPLREG_DEBUG, "setBPLCON2(%X)\n", value);

    bplcon2 = value;

    // Record the pixel coordinate where the change takes place
    conRegChanges.add(4 * agnus.pos.h + 4, REG_BPLCON2, value);
}

uint16_t
Denise::zPF(uint16_t priorityBits)
{
    switch (priorityBits) {

        case 0: return Z_0;
        case 1: return Z_1;
        case 2: return Z_2;
        case 3: return Z_3;
        case 4: return Z_4;
    }

    return 0;
}

uint16_t
Denise::peekCLXDAT()
{
    uint16_t result = clxdat | 0x8000;
    clxdat = 0;
    return result;
}

void
Denise::pokeCLXCON(uint16_t value)
{
    clxcon = value;
}

template <int x> void
Denise::pokeBPLxDAT(uint16_t value)
{
    assert(x < 6);
    debug(BPLREG_DEBUG, "pokeBPL%dDAT(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

template <int x> void
Denise::pokeSPRxPOS(uint16_t value)
{
    assert(x < 8);
    debug(SPRREG_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.

    sprhstrt[x] = ((value & 0xFF) << 1) | (sprhstrt[x] & 0x01);
    
    // Update debugger info
    if (agnus.pos.v == 26) {
        info.sprite[x].pos = value;
    }
}

template <int x> void
Denise::pokeSPRxCTL(uint16_t value)
{
    assert(x < 8);
    debug(SPRREG_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.
    
    sprhstrt[x] = (sprhstrt[x] & 0x1FE) | (value & 0x01);
    WRITE_BIT(attach, x, GET_BIT(value, 7));

    // Disarm the sprite (stop drawing)
    CLR_BIT(armed, x);

    // Update debugger info
    if (agnus.pos.v == 26) {
        info.sprite[x].ctl = value;
        info.sprite[x].ptr = agnus.sprpt[x];
        assert(IS_EVEN(info.sprite[x].ptr));
    }
}

template <int x> void
Denise::pokeSPRxDATA(uint16_t value)
{
    assert(x < 8);
    debug(SPRREG_DEBUG, "pokeSPR%dDATA(%X)\n", x, value);
    
    sprdata[x] = value;
    armSprite(x);
}

template <int x> void
Denise::pokeSPRxDATB(uint16_t value)
{
    assert(x < 8);
    debug(SPRREG_DEBUG, "pokeSPR%dDATB(%X)\n", x, value);
    
    sprdatb[x] = value;
}

void
Denise::pokeColorReg(uint32_t addr, uint16_t value)
{

    // if (addr == 0x182) value = 0; //REMOVE ASAP

    assert(addr >= 0x180 && addr <= 0x1BE && IS_EVEN(addr));
    debug(COL_DEBUG, "pokeColorReg(%X, %X)\n", addr, value);

    pixelEngine.colRegChanges.add(4 * agnus.pos.h, addr, value);
}

bool
Denise::attached(int x) {

    assert(x >= 1 && x <= 7);
    assert(IS_ODD(x));

    return GET_BIT(attach,x) && sprhstrt[x-1] == sprhstrt[x];
}

void
Denise::armSprite(int x)
{
    SET_BIT(armed, x);
}

void
Denise::updateSpritePriorities(uint16_t bplcon2)
{
    switch (bplcon2 & 0b111) {

        case 0: prio1 = Z_0; break;
        case 1: prio1 = Z_1; break;
        case 2: prio1 = Z_2; break;
        case 3: prio1 = Z_3; break;
        case 4: prio1 = Z_4; break;

        default: // Illegal value
            prio1 = 0;
            break;
    }

    switch ((bplcon2 >> 3) & 0b111) {

        case 0: prio2 = Z_0; break;
        case 1: prio2 = Z_1; break;
        case 2: prio2 = Z_2; break;
        case 3: prio2 = Z_3; break;
        case 4: prio2 = Z_4; break;

        default: // Illegal value
            prio2 = 0;
            break;
    }

    prio12 = MAX(prio1, prio2);

    // debug("bplcon2 = %X prio1 = %d prio2 = %d prio12 = %d\n", bplcon2, prio1, prio2, prio12);
}

void
Denise::fillShiftRegisters()
{
    switch (bpu()) {
        case 6: shiftReg[5] = REPLACE_LO_WORD(shiftReg[5], bpldat[5]);
        case 5: shiftReg[4] = REPLACE_LO_WORD(shiftReg[4], bpldat[4]);
        case 4: shiftReg[3] = REPLACE_LO_WORD(shiftReg[3], bpldat[3]);
        case 3: shiftReg[2] = REPLACE_LO_WORD(shiftReg[2], bpldat[2]);
        case 2: shiftReg[1] = REPLACE_LO_WORD(shiftReg[1], bpldat[1]);
        case 1: shiftReg[0] = REPLACE_LO_WORD(shiftReg[0], bpldat[0]);
    }
}

template <int HIRES> void
Denise::draw(int pixels)
{
    uint8_t index;

    int16_t currentPixel = ppos(agnus.pos.h);

    if (firstDrawnPixel == 0) {
        firstDrawnPixel = currentPixel;
        spriteClipBegin = currentPixel - 2;
    }

    uint32_t maskOdd = 0x8000 << scrollLoresOdd * (HIRES ? 2 : 1);
    uint32_t maskEven = 0x8000 << scrollLoresEven * (HIRES ? 2 : 1);

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
            assert(currentPixel < sizeof(bBuffer));
            bBuffer[currentPixel++] = index;

        } else {

            // Synthesize two lores pixels
            assert(currentPixel + 1 < sizeof(bBuffer));
            bBuffer[currentPixel++] = index;
            bBuffer[currentPixel++] = index;
        }
    }

    // Shift out drawn bits
    for (int i = 0; i < 6; i++) shiftReg[i] <<= pixels;

    lastDrawnPixel = currentPixel;

#ifdef PIXEL_DEBUG
    rasterline[currentPixel - 2 * pixels] = 64;
#endif
}

void
Denise::translate()
{
    int pixel = 0;

    uint16_t bplcon0 = initialBplcon0;
    bool dual = dbplf(bplcon0);

    uint16_t bplcon2 = initialBplcon2;
    bool pri = PF2PRI(bplcon2);
    prio1 = zPF1(bplcon2);
    prio2 = zPF2(bplcon2);
    // updateSpritePriorities(bplcon2);

    // Add a dummy register change to ensure we draw until the line ends
    conRegChanges.add(sizeof(bBuffer), REG_NONE, 0);

    // Iterate over all recorded register changes
    for (int i = conRegChanges.begin(); i != conRegChanges.end(); i = conRegChanges.next(i)) {

        Change &change = conRegChanges.change[i];

        // Translate a chunk of bitplane data
        if (dual) {
            translateDPF(pri, pixel, change.trigger);
        } else {
            translateSPF(pixel, change.trigger);
        }
        pixel = change.trigger;

        // Apply the register change
        switch (change.addr) {

            case REG_BPLCON0_DENISE:
                bplcon0 = change.value;
                dual = dbplf(bplcon0);
                break;

            case REG_BPLCON2:
                bplcon2 = change.value;
                pri = PF2PRI(bplcon2);
                // updateSpritePriorities(bplcon2);
                prio1 = zPF1(bplcon2);
                prio2 = zPF2(bplcon2);
                break;

            default:
                assert(change.addr == REG_NONE);
                break;
        }
    }

    // Clear the history cache
    conRegChanges.clear();
}

void
Denise::translateSPF(int from, int to)
{
    // The usual case: prio2 is a valid value
    if (prio2) {
        for (int i = from; i < to; i++) {

            uint8_t s = bBuffer[i];

            assert(PixelEngine::isRgbaIndex(s));
            iBuffer[i] = s;
            zBuffer[i] = s ? prio2 : 0;
        }

    // The unusual case: prio2 is ivalid
    } else {

        for (int i = from; i < to; i++) {

             uint8_t s = bBuffer[i];

             assert(PixelEngine::isRgbaIndex(s));
             iBuffer[i] = (s & 16) ? 16 : s;
             zBuffer[i] = 0;
         }
    }
}

void
Denise::translateDPF(bool pf2pri, int from, int to)
{
    pf2pri ? translateDPF<true>(from, to) : translateDPF<false>(from, to);
}

template <bool pf2pri> void
Denise::translateDPF(int from, int to)
{
    /* If the priority of a playfield is set to an illegal value (prio1 or
     * prio2 will be 0 in that case), all pixels are drawn transparent.
     */
    uint8_t mask1 = prio1 ? 0b1111 : 0b0000;
    uint8_t mask2 = prio2 ? 0b1111 : 0b0000;

    for (int i = from; i < to; i++) {

        uint8_t s = bBuffer[i];

        // Determine color indices for both playfields
        uint8_t index1 = (((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2));
        uint8_t index2 = (((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3));

        /*
        if (pf2pri) {

            // Playfield 2 appears in front
            if (index2) {
                iBuffer[i] = index2 | 0b1000;
                zBuffer[i] = prio2 | Z_DPF;
            } else if (index1) {
                iBuffer[i] = index1;
                zBuffer[i] = prio1 | Z_DPF;
            } else {
                iBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }

        } else {

            // Playfield 1 appears in front
            if (index1) {
                iBuffer[i] = index1;
                zBuffer[i] = prio1 | Z_DPF;
            } else if (index2) {
                iBuffer[i] = index2 | 0b1000;
                zBuffer[i] = prio2 | Z_DPF;
            } else {
                iBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }
        }
        */
        
        if (index1) {
            if (index2) {

                // PF1 is solid, PF2 is solid
                if (pf2pri) {
                    iBuffer[i] = (index2 | 0b1000) & mask2;
                    zBuffer[i] = prio2 | Z_DPF | Z_PF1 | Z_PF2;
                } else {
                    iBuffer[i] = index1 & mask1;
                    zBuffer[i] = prio1 | Z_DPF | Z_PF1 | Z_PF2;
                }

            } else {

                // PF1 is solid, PF2 is transparent
                iBuffer[i] = index1 & mask1;
                zBuffer[i] = prio1 | Z_DPF | Z_PF1;
            }

        } else {
            if (index2) {

                // PF1 is transparent, PF2 is solid
                iBuffer[i] = (index2 | 0b1000) & mask2;
                zBuffer[i] = prio2 | Z_DPF | Z_PF2;

            } else {

                // PF1 is transparent, PF2 is transparent
                iBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }
        }
    }
}

void
Denise::drawSprites()
{
    // Compute final clipping area
    // spriteClipBegin = MAX(spriteClipBegin, firstDrawnPixel - 2);
    // spriteClipEnd = MIN(spriteClipEnd, lastDrawnPixel);

    if (config.emulateSprites) {

        // Sprites 6 and 7
        if (armed & 0b11000000) {
            if (attached(7)) {
                drawSpritePair<7>();
            } else {
                if (armed & 0b10000000) drawSpriteNew<7>();
                if (armed & 0b01000000) drawSpriteNew<6>();
            }
        }

        // Sprites 4 and 5
        if (armed & 0b00110000) {
            if (attached(5)) {
                drawSpritePair<5>();
            } else {
                if (armed & 0b00100000) drawSpriteNew<5>();
                if (armed & 0b00010000) drawSpriteNew<4>();
            }
        }

        // Sprites 2 and 3
        if (armed & 0b00001100) {
            if (attached(3)) {
                drawSpritePair<3>();
            } else {
                if (armed & 0b00001000) drawSpriteNew<3>();
                if (armed & 0b00000100) drawSpriteNew<2>();
            }
        }

        // Sprites 1 and 0
        if (armed & 0b00000011) {
            if (attached(1)) {
                drawSpritePair<1>();
            } else {
                if (armed & 0b00000010) drawSpriteNew<1>();
                if (armed & 0b00000001) drawSpriteNew<0>();
            }
        }
    }
}

template <int x> void
Denise::drawSprite()
{
    assert(x >= 0 && x <= 7);

    // Check for a quick-exit
    if (spriteClipBegin == HPIXELS) return;

    uint16_t z = Z_SP[x];

    uint32_t d1 = (uint32_t)sprdatb[x] << 1;
    uint32_t d0 = (uint32_t)sprdata[x] << 0;

    int baseCol = 16 + 2 * (x & 6);

    int start = 2 + 2 * sprhstrt[x];
    int end = start + 31;

    for (int pos = end; pos >= start; pos -= 2) {

        if (pos >= spriteClipBegin && pos < spriteClipEnd) {

            int col = (d1 & 0b0010) | (d0 & 0b0001);

            if (col) {
                if (z > zBuffer[pos]) iBuffer[pos] = baseCol | col;
                if (z > zBuffer[pos-1]) iBuffer[pos-1] = baseCol | col;
                zBuffer[pos] |= z;
                zBuffer[pos-1] |= z;
            }
        }

        d1 >>= 1;
        d0 >>= 1;
    }

    // Perform collision checks (if enabled)
    if (config.clxSprSpr) checkS2SCollisions<x>(start, end);
    if (config.clxSprPlf) checkS2PCollisions<x>(start, end);
}

template <int x> void
Denise::drawSpritePair()
{
    assert(x >= 1 && x <= 7);
    assert(IS_ODD(x));

    uint16_t z = Z_SP[x];

    uint32_t d3 = (uint32_t)sprdatb[x]   << 3;
    uint32_t d2 = (uint32_t)sprdata[x]   << 2;
    uint32_t d1 = (uint32_t)sprdatb[x-1] << 1;
    uint32_t d0 = (uint32_t)sprdata[x-1] << 0;

    int start = 2 + 2 * sprhstrt[x];
    int end = MIN(start + 31, LAST_PIXEL);

    for (int pos = end; pos >= start; pos -= 2) {

        int col = (d3 & 0b1000) | (d2 & 0b0100) | (d1 & 0b0010) | (d0 & 0b0001);

        if (col) {
            if (z > zBuffer[pos]) {
                iBuffer[pos] = 0b10000 | col;
                zBuffer[pos] |= z;
            }
            if (z > zBuffer[pos-1]) {
                iBuffer[pos-1] = 0b10000 | col;
                zBuffer[pos-1] |= z;
            }
        }

        d3 >>= 1;
        d2 >>= 1;
        d1 >>= 1;
        d0 >>= 1;
    }

    // Perform collision checks (if enabled)
    if (config.clxSprSpr) checkS2SCollisions<x>(start, end);
    if (config.clxSprPlf) checkS2PCollisions<x>(start, end);
}

template <int x> void
Denise::drawSpritePixel(int pixel, int hpos)
{
    assert(pixel >= 0 && pixel <= 15);
    assert(hpos >= spriteClipBegin);
    assert(hpos < spriteClipEnd);

    uint8_t a = !!GET_BIT(sprdata[x], pixel);
    uint8_t b = !!GET_BIT(sprdatb[x], pixel);
    uint8_t col = (b << 1) | a;

    if (col) {

        uint16_t z = Z_SP[x];
        int base = 16 + 2 * (x & 6);

        if (z > zBuffer[hpos]) iBuffer[hpos] = base | col;
        if (z > zBuffer[hpos + 1]) iBuffer[hpos + 1] = base | col;
        zBuffer[hpos] |= z;
        zBuffer[hpos + 1] |= z;
    }
}

template <int x> void
Denise::drawSpriteNew()
{
    assert(x >= 0 && x <= 7);

    // Check for a quick-exit
    if (spriteClipBegin == HPIXELS) return;

    int start = 2 + 2 * sprhstrt[x];
    int end = start + 31;
    int cnt = 0;

    for (int hpos = 0; hpos < sizeof(iBuffer) - 1; hpos += 2) {

        if (hpos == start) cnt = 16;

        if (cnt) {
            if (hpos >= spriteClipBegin && hpos < spriteClipEnd) {
                drawSpritePixel<x>(--cnt, hpos);
            }
        }
    }

    // Perform collision checks (if enabled)
    if (config.clxSprSpr) checkS2SCollisions<x>(start, end);
    if (config.clxSprPlf) checkS2PCollisions<x>(start, end);
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
    bool hFlopWasSet = agnus.diwHFlop || agnus.diwHFlopOn != -1;

    // Check if the whole line is blank (drawn in background color)
    bool lineIsBlank = !agnus.diwVFlop || !hFlopWasSet;

    // Draw the border
    if (lineIsBlank) {

        for (int i = 0; i <= LAST_PIXEL; i++) {
            iBuffer[i] = borderV;
        }

    } else {

        // Draw left border
        if (!agnus.diwHFlop && agnus.diwHFlopOn != -1) {
            for (int i = 0; i < 2 * agnus.diwHFlopOn; i++) {
                assert(i < sizeof(iBuffer));
                iBuffer[i] = borderL;
            }
        }

        // Draw right border
        if (agnus.diwHFlopOff != -1) {
            for (int i = 2 * agnus.diwHFlopOff; i <= LAST_PIXEL; i++) {
                assert(i < sizeof(iBuffer));
                iBuffer[i] = borderR;
            }
        }
    }

#ifdef LINE_DEBUG
    int16_t vpos = agnus->pos.v;
    bool lines = vpos == 0x94; // vpos == 26 || vpos == 0x50 || vpos == 276 || vpos == 255;
    if (lines) {
        // printf("Line\n");
        for (int i = 0; i <= LAST_PIXEL / 2; iBuffer[i++] = 64);
    }
#endif
}

template <int x> void
Denise::checkS2SCollisions(int start, int end)
{
    // For the odd sprites, only proceed if collision detection is enabled
    if (IS_ODD(x) && !GET_BIT(clxcon, 12 + (x/2))) return;

    // Set up the sprite comparison masks
    uint16_t comp01 = Z_SP0 | (GET_BIT(clxcon, 12) ? Z_SP1 : 0);
    uint16_t comp23 = Z_SP2 | (GET_BIT(clxcon, 13) ? Z_SP3 : 0);
    uint16_t comp45 = Z_SP4 | (GET_BIT(clxcon, 14) ? Z_SP5 : 0);
    uint16_t comp67 = Z_SP6 | (GET_BIT(clxcon, 15) ? Z_SP7 : 0);

    // Iterate over all sprite pixels
    for (int pos = end; pos >= start; pos -= 2) {

        uint16_t z = zBuffer[pos];

        // Skip if there are no other sprites at this pixel coordinate
        if (!(z & (Z_SP01234567 ^ Z_SP[x]))) continue;

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // Set sprite collision bits
        if ((z & comp45) && (z & comp67)) SET_BIT(clxdat, 14);
        if ((z & comp23) && (z & comp67)) SET_BIT(clxdat, 13);
        if ((z & comp23) && (z & comp45)) SET_BIT(clxdat, 12);
        if ((z & comp01) && (z & comp67)) SET_BIT(clxdat, 11);
        if ((z & comp01) && (z & comp45)) SET_BIT(clxdat, 10);
        if ((z & comp01) && (z & comp23)) SET_BIT(clxdat, 9);

        if (CLX_DEBUG == 1) {
            if ((z & comp45) && (z & comp67)) debug("Collision between 45 and 67\n");
            if ((z & comp23) && (z & comp67)) debug("Collision between 23 and 67\n");
            if ((z & comp23) && (z & comp45)) debug("Collision between 23 and 45\n");
            if ((z & comp01) && (z & comp67)) debug("Collision between 01 and 67\n");
            if ((z & comp01) && (z & comp45)) debug("Collision between 01 and 45\n");
            if ((z & comp01) && (z & comp23)) debug("Collision between 01 and 23\n");
        }
    }
}

template <int x> void
Denise::checkS2PCollisions(int start, int end)
{
    // For the odd sprites, only proceed if collision detection is enabled
    if (IS_ODD(x) && !getENSP<x>()) return;

    // Set up the sprite comparison mask
    uint16_t sprMask;
    switch(x) {
        case 0:
        case 1: sprMask = Z_SP0 | (getENSP<1>() ? Z_SP1 : 0); break;
        case 2:
        case 3: sprMask = Z_SP2 | (getENSP<3>() ? Z_SP3 : 0); break;
        case 4:
        case 5: sprMask = Z_SP4 | (getENSP<5>() ? Z_SP5 : 0); break;
        case 6:
        case 7: sprMask = Z_SP6 | (getENSP<7>() ? Z_SP7 : 0); break;

        default: sprMask = 0; assert(false);
    }

    uint8_t enabled1 = getENBP1();
    uint8_t enabled2 = getENBP2();
    uint8_t compare1 = getMVBP1() & enabled1;
    uint8_t compare2 = getMVBP2() & enabled2;

    // Check for sprite-playfield collisions
    for (int pos = end; pos >= start; pos -= 2) {

        uint16_t z = zBuffer[pos];

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // debug(CLX_DEBUG, "<%d> b[%d] = %X e1 = %X e2 = %X, c1 = %X c2 = %X\n",
        //     x, pos, bBuffer[pos], enabled1, enabled2, compare1, compare2);

        // Check for a collision with playfield 2
        if ((bBuffer[pos] & enabled2) == compare2) {
            debug(CLX_DEBUG, "S%d collides with PF2\n", x);
            SET_BIT(clxdat, 5 + (x / 2));
            SET_BIT(clxdat, 1 + (x / 2));

        } else {
            // There is a hardware oddity in single-playfield mode. If PF2
            // doesn't match, playfield 1 doesn't match, too. No matter what.
            // See http://eab.abime.net/showpost.php?p=965074&postcount=2
            if ((zBuffer[pos] & Z_DPF) == 0) continue;
        }

        // Check for a collision with playfield 1
        if ((bBuffer[pos] & enabled1) == compare1) {
            debug(CLX_DEBUG, "S%d collides with PF1\n", x);
            SET_BIT(clxdat, 1 + (x / 2));
        }
    }
}

void
Denise::checkP2PCollisions()
{
    // Quick-exit if the collision bit already set
    if (GET_BIT(clxdat, 0)) return;

    // Set up comparison masks
    uint8_t enabled1 = getENBP1();
    uint8_t enabled2 = getENBP2();
    uint8_t compare1 = getMVBP1() & enabled1;
    uint8_t compare2 = getMVBP2() & enabled2;

    // Check all pixels one by one
    for (int pos = 0; pos < HPIXELS; pos++) {

        uint16_t b = bBuffer[pos];
        // debug(CLX_DEBUG, "b[%d] = %X e1 = %X e2 = %X c1 = %X c2 = %X\n",
        //       pos, b, enabled1, enabled2, compare1, compare2);

        // Check if there is a hit with playfield 1
        if ((b & enabled1) != compare1) continue;

        // Check if there is a hit with playfield 2
        if ((b & enabled2) != compare2) continue;

        // Set collision bit
        SET_BIT(clxdat, 0);
        return;
    }
}

void
Denise::beginOfFrame(bool interlace)
{
    pixelEngine.beginOfFrame(interlace);
}

void
Denise::beginOfLine(int vpos)
{
    // Reset the register history buffers
    conRegChanges.clear();
    pixelEngine.colRegChanges.clear();

    // Save the current values of the bitplane control registers
    initialBplcon0 = bplcon0;
    initialBplcon1 = bplcon1;
    initialBplcon2 = bplcon2;

    // Clear the bBuffer
    memset(bBuffer, 0, sizeof(bBuffer));

    firstDrawnPixel = 0;
    lastDrawnPixel = 0;

    // Reset sprite clipping range
    spriteClipBegin = HPIXELS;
    spriteClipEnd = HPIXELS;
}

void
Denise::endOfLine(int vpos)
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);

    // Check if we are below the VBLANK area
    if (vpos >= 26) {

        // Translate bitplane data to color register indices
        translate();

        // Draw sprites if at least one is armed
        if (armed) drawSprites();

        // Draw border pixels
        drawBorder();

        // Perform playfield-playfield collision check (if enabled)
        if (config.clxPlfPlf) checkP2PCollisions();

        // Synthesize RGBA values and write the result into the frame buffer
        pixelEngine.colorize(iBuffer, vpos);

    } else {
        pixelEngine.endOfVBlankLine();
    }

    // Invoke the DMA debugger
    dmaDebugger.computeOverlay();
}

void
Denise::pokeDMACON(uint16_t oldValue, uint16_t newValue)
{
    if (Agnus::bplDMA(newValue)) {

        // Bitplane DMA on
        debug(DMA_DEBUG, "Bitplane DMA switched on\n");

    } else {

        // Bitplane DMA off
        debug(DMA_DEBUG, "Bitplane DMA switched off\n");
    }
}

void
Denise::debugSetBPU(int count)
{
    if (count < 0) count = 0;
    if (count > 6) count = 6;

    amiga.suspend();
    
    uint16_t value = bplcon0 & 0b1000111111111111;
    pokeBPLCON0(value | (count << 12));
    
    amiga.resume();
}

void
Denise::debugSetBPLCONx(unsigned x, uint16_t value)
{
    assert(x <= 2);

    amiga.suspend();

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

    amiga.resume();
}

void
Denise::debugSetBPLCONxBit(unsigned x, unsigned bit, bool value)
{
    assert(x <= 2);
    assert(bit <= 15);

    uint16_t mask = 1 << bit;

    amiga.suspend();

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
    
    amiga.resume();
}

void
Denise::debugSetBPLCONxNibble(unsigned x, unsigned nibble, uint8_t value)
{
    assert(x <= 2);
    assert(nibble <= 4);

    uint16_t mask = 0b1111 << (4 * nibble);
    uint16_t bits = (value & 0b1111) << (4 * nibble);

    amiga.suspend();

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

    amiga.resume();
}

void
Denise::dumpBuffer(uint8_t *buffer, size_t length)
{
    const size_t cols = 16;

    for (int i = 0; i < (length + cols - 1) / cols; i++) {
        for (int j =0; j < cols; j++) plainmsg("%2d ", buffer[i * cols + j]);
        plainmsg("\n");
    }
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

template void Denise::translateDPF<true>(int from, int to);
template void Denise::translateDPF<false>(int from, int to);
