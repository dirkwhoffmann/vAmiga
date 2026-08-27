// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"

namespace vamiga {

void
Denise::setDIWSTRT(u16 value)
{
    logmsg(LOG_DIW, "setDIWSTRT(%x)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0
    
    diwstrt = value;
    setHSTRT(LO_BYTE(diwstrt));
    setHSTOP(LO_BYTE(diwstop) | 0x100);
}

void
Denise::setDIWSTOP(u16 value)
{
    logmsg(LOG_DIW, "setDIWSTOP(%x)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1

    diwstop = value;
    setHSTRT(LO_BYTE(diwstrt));
    setHSTOP(LO_BYTE(diwstop) | 0x100);
}

void
Denise::setDIWHIGH(u16 value)
{
    logmsg(LOG_DIW, "setDIWHIGH(%x)\n", value);

    if (isOCS()) return;

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- H8 -- -- -- -- -- -- -- H8 -- -- -- -- --
    //     (stop)                  (strt)

    diwhigh = value;
    setHSTRT(LO_BYTE(diwstrt) | (GET_BIT(diwhigh,  5) ? 0x100 : 0x000));
    setHSTOP(LO_BYTE(diwstop) | (GET_BIT(diwhigh, 13) ? 0x100 : 0x000));
}

void
Denise::setHSTRT(isize val)
{
    logmsg(LOG_DIW, "setHSTRT(%lx)\n", val);

    // Record register change, converted to super-hires (buffer) resolution
    diwChanges.insert(agnus.pos.pixel(), RegChange { .reg = Reg::DIWSTRT, .value = (u16)(4 * val) });
    markBorderBufferAsDirty();
}

void
Denise::setHSTOP(isize val)
{
    logmsg(LOG_DIW, "setHSTOP(%lx)\n", val);

    // Record register change, converted to super-hires (buffer) resolution
    diwChanges.insert(agnus.pos.pixel(), RegChange { .reg = Reg::DIWSTOP, .value = (u16)(4 * val) });
    markBorderBufferAsDirty();
}

u16
Denise::peekJOY0DATR() const
{
    u16 result = controlPort1.joydat();
    logmsg(LOG_JOYREG, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

u16
Denise::peekJOY1DATR() const
{
    u16 result = controlPort2.joydat();
    logmsg(LOG_JOYREG, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(u16 value)
{
    logmsg(LOG_JOYREG, "pokeJOYTEST(%04X)\n", value);

    controlPort1.pokeJOYTEST(value);
    controlPort2.pokeJOYTEST(value);
}

u16
Denise::peekDENISEID()
{
    u16 result = isAGA() ? 0x00F8 : isECS() ? 0xFFFC : 0xFFFF;

    logmsg(LOG_ECSREG, "peekDENISEID() = $%04X (%d)\n", result, result);
    return result;
}

u16
Denise::spypeekDENISEID() const
{
    return isAGA() ? 0x00F8 : isECS() ? 0xFFFC : 0xFFFF;
}

Pixel
Denise::borderChangePixel() const
{
    return std::max(agnus.pos.pixel() + BORDER_LATENCY, Pixel(0));
}

template <Accessor s> void
Denise::pokeBPLCON0(u16 value)
{
    logmsg(LOG_BPLREG, "pokeBPLCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON0, value, Accessor::DENISE);
}

void
Denise::setBPLCON0(u16 oldValue, u16 newValue)
{
    logmsg(LOG_BPLREG, "setBPLCON0(%04x,%04x)\n", oldValue, newValue);

    auto pixel  = std::max(agnus.pos.pixel() - 8, Pixel(0));
    auto change = RegChange { .reg = Reg::BPLCON0, .value = newValue };

    // Record the register change
    conChanges.insert(pixel, change);
    
    // If the change affects color, record it the change in the pixel engine, too
    if ((oldValue ^ newValue) & 0xFC50) {
        
        change.accessor = Accessor::DENISE;
        pixelEngine.colChanges.insert(pixel, change);
    }
        
    // Update value
    bplcon0 = newValue;

    // Determine the new bitmap resolution
    res = resolution(newValue);

    updateScrollOffsets();
    
    /* ECSENA gates BRDRBLNK, so it changes the border for the same reason a
     * BRDRBLNK write does, and it takes effect at the same place (see
     * setBPLCON3).
     */
    if (ecsena(oldValue) != ecsena(newValue)) {

        diwChanges.insert(borderChangePixel(), change);
        markBorderBufferAsDirty();
    }
}

template <Accessor s> void
Denise::pokeBPLCON1(u16 value)
{
    logmsg(LOG_BPLREG, "pokeBPLCON1(%X)\n", value);

    // Record the register change
    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON1, value, Accessor::DENISE);
}

void
Denise::setBPLCON1(u16 oldValue, u16 newValue)
{
    logmsg(LOG_BPLREG, "setBPLCON1(%x,%x)\n", oldValue, newValue);

    // In AGA, the upper byte holds the extended scroll bits
    bplcon1 = newValue & (isAGA() ? 0xFFFF : 0x00FF);

    updateScrollOffsets();

    // pixelOffsetOdd  = (i8)((bplcon1 & 0b00000001) << 1);
    // pixelOffsetEven = (i8)((bplcon1 & 0b00010000) >> 3);
}

template <Accessor s> void
Denise::pokeBPLCON2(u16 value)
{
    logmsg(LOG_BPLREG, "pokeBPLCON2(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON2, value);
}

void
Denise::setBPLCON2(u16 newValue)
{
    logmsg(LOG_BPLREG, "setBPLCON2(%X)\n", newValue);
    
    auto oldValue = bplcon2;
    bplcon2 = newValue;
    
    if (pf1px() > 4) { xfiles("BPLCON2: PF1P = %d\n", pf1px()); }
    if (pf2px() > 4) { xfiles("BPLCON2: PF2P = %d\n", pf2px()); }
    
    // Record the register change
    i64 pixel = agnus.pos.pixel() + 8;
    conChanges.insert(pixel, RegChange { .reg = Reg::BPLCON2, .value = newValue });

    // Check if the KILLEHB bit has changed
    if (killehb(oldValue) ^ killehb(newValue)) {

        /* The offset is measured, not derived: killehb2 puts the A1200
         * switch within +0.3 columns of this in LORES, HIRES and SHRES
         * alike, so one constant serves all three.
         */
        auto colPixel = std::max(agnus.pos.pixel() - 2, Pixel(0));
        pixelEngine.colChanges.insert(colPixel, RegChange { .reg = Reg::BPLCON2, .value = newValue });
    }
}

template <Accessor s> void
Denise::pokeBPLCON3(u16 value)
{
    logmsg(LOG_BPLREG, "pokeBPLCON3(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON3, value);
}

void
Denise::setBPLCON3(u16 value)
{
    logmsg(LOG_BPLREG, "setBPLCON3(%X)\n", value);

    auto oldValue = bplcon3;
    bplcon3 = value;

    /* BRDRBLNK decides whether the border is painted in the background color
     * or in black, and it does so from the pixel it is written at rather
     * than for the whole line. The change is therefore recorded together
     * with the DIW changes, which is where the border mask is built, and the
     * mask is invalidated so that it is actually rebuilt. Without the record
     * the bit would only ever be sampled at the start of a line; without the
     * invalidation the mask would be recomputed only when the display window
     * moves, and BRDRBLNK would appear frozen at whatever it was then.
     *
     * Relevant tests: Denise/Registers/BPLCON3/brdrblnk1 and brdrblnk2
     */
    if (brdrblnk(oldValue) != brdrblnk(value)) {

        diwChanges.insert(borderChangePixel(),
                          RegChange { .reg = Reg::BPLCON3, .value = value });
        markBorderBufferAsDirty();
    }
}

template <Accessor s> void
Denise::pokeBPLCON4(u16 value)
{
    logmsg(LOG_BPLREG, "pokeBPLCON4(%X)\n", value);
    
    setBPLCON4(value);
}

void
Denise::setBPLCON4(u16 value)
{
    logmsg(LOG_BPLREG, "setBPLCON4(%X)\n", value);

    bplcon4 = isAGA() ? value : 0;

    /* Record the register change. BPLAM, the color XOR in the high byte, is
     * applied while the rasterline is translated, so a change has to take
     * effect at the pixel where it was written instead of retroactively
     * covering the whole line (see translate).
     *
     * Relevant test in the vAmigaTS test suite: Denise/Modes/bplam2
     */
    i64 pixel = agnus.pos.pixel() + 8;
    conChanges.insert(pixel, RegChange { .reg = Reg::BPLCON4, .value = bplcon4 });
}

u16
Denise::peekCLXDAT()
{
    u16 result = clxdat | 0x8000;
    clxdat = 0;
    
    logmsg(LOG_CLXREG, "peekCLXDAT() = %x\n", result);
    return result;
}

u16
Denise::spypeekCLXDAT() const
{
    return clxdat | 0x8000;
}

void
Denise::pokeCLXCON(u16 value)
{
    logmsg(LOG_CLXREG, "pokeCLXCON(%x)\n", value);
    clxcon = value;
}

void
Denise::pokeCLXCON2(u16 value)
{
    logmsg(LOG_CLXREG, "pokeCLXCON2(%x)\n", value);

    if (!isAGA()) return;
    clxcon2 = value;
}

template <isize x, Accessor s> void
Denise::pokeBPLxDAT(u16 value)
{
    assert(x < 8);
    logmsg(LOG_BPLREG, "pokeBPL%ldDAT(%X)\n", x + 1, value);

    if constexpr (s == Accessor::AGNUS) {
        /*
         logmsg("BPL%dDAT written by Agnus (%x)\n", x, value);
         */
    }
    
    setBPLxDAT<x>(value);
}

/*
template <isize x> void
Denise::setBPLxDAT(u16 value)
{
    assert(x < 8);
    logmsg(LOG_BPLDAT, "setBPL%ldDAT(%X)\n", x + 1, value);

    bpldat[x] = value;

    if constexpr (x == 0) {
        
        // Feed data registers into pipe
        for (isize i = 0; i < 8; i++) bpldatPipe[i] = bpldat[i];

        armedOdd = true;
        armedEven = true;

        spriteClipBegin = std::min(spriteClipBegin, Pixel(agnus.pos.pixel() + SPRITE_LATENCY));
    }
}
*/

template <isize x> void
Denise::setBPLxDAT(u16 value)
{
    assert(x < 8);
    logmsg(LOG_BPLDAT, "setBPL%ldDAT(%X)\n", x + 1, value);

    bpldat[x] = value;

    if constexpr (x == 0) {

        if (agnus.bplFetchWords() > 1) {

            /* In AGA, a single fetch provides data for several drawing cycles.
             * The pipeline is not reloaded here, but at the drawing cycle
             * selected by the extended scroll bits (see prepareOdd). Take a
             * snapshot of the fetched data, because the next fetch may overwrite
             * the data registers before that cycle is reached.
             */
            for (isize i = 0; i < 8; i++) bpldatLatch[i] = bpldat[i];
            for (isize i = 0; i < 8; i++) bpldatLatchExt[i] = bpldatExt[i];
            latchExtCnt = bpldatExtCnt;

            latchedOdd = true;
            latchedEven = true;

        } else {

            // Feed data registers into pipe
            for (isize i = 0; i < 8; i++) bpldatPipe[i] = bpldat[i];
            for (isize i = 0; i < 8; i++) bpldatPipeExt[i] = bpldatExt[i];
            extCntOdd = extCntEven = bpldatExtCnt;

            armedOdd = true;
            armedEven = true;
        }

        /* The first write of a line is what opens the display window, and the
         * same write arms the sprites two entries earlier. See BPLDAT_LATENCY
         * and SPRITE_LATENCY for the photographs that pin both.
         */
        spriteClipBegin = std::min(spriteClipBegin, Pixel(agnus.pos.pixel() + SPRITE_LATENCY));
        bplDatBegin = std::min(bplDatBegin, Pixel(agnus.pos.pixel() + BPLDAT_LATENCY));
    }
}

template <isize x> void
Denise::setBPLxDATExt(u64 value, u8 count)
{
    assert(x < 8);

    bpldatExt[x] = value;
    bpldatExtCnt = count;
}

template <isize x> void
Denise::pokeSPRxPOS(u16 value)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "pokeSPR%ldPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Record the register change
    i64 pos = agnus.pos.pixel() + 12;
    constexpr auto reg = Reg(isize(Reg::SPR0POS) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

template <isize x> void
Denise::pokeSPRxCTL(u16 value)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "pokeSPR%ldCTL(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)

    // Record the register change
    i64 pos = agnus.pos.pixel() + 12;
    constexpr auto reg = Reg(isize(Reg::SPR0CTL) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

/*
template <isize x> void
Denise::pokeSPRxDATA(u16 value)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "pokeSPR%ldDATA(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;
    
    // Remember that the sprite was armed at least once in this rasterline
    SET_BIT(wasArmed, x);

    // Record the register change
    i64 pos = agnus.pos.pixel() + 8;
    constexpr auto reg = Reg(isize(Reg::SPR0DATA) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

template <isize x> void
Denise::pokeSPRxDATB(u16 value)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "pokeSPR%ldDATB(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;

    // Record the register change
    i64 pos = agnus.pos.pixel() + 8;
    constexpr auto reg = Reg(isize(Reg::SPR0DATB) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value });
}
*/

template <isize x> void
Denise::pokeSPRxDATA(u16 value)
{
    setSPRxDATA<x>(value, 0);
}

template <isize x> void
Denise::setSPRxDATA(u16 value, u64 ext)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "setSPR%ldDATA(%X,%llX)\n", x, value, ext);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) { value = 0; ext = 0; }
    
    // Remember that the sprite was armed at least once in this rasterline
    SET_BIT(wasArmed, x);

    /* Store the AGA extension. Only the first word takes part in the register
     * change history, because that history carries 16 bit values. The pairing
     * is unambiguous as long as the sprite is fed by DMA, which performs a
     * single data fetch per sprite and rasterline.
     */
    sprdataExt[x] = ext;

    // Record the register change
    i64 pos = agnus.pos.pixel() + 8;
    constexpr auto reg = Reg(isize(Reg::SPR0DATA) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

template <isize x> void
Denise::pokeSPRxDATB(u16 value)
{
    setSPRxDATB<x>(value, 0);
}

template <isize x> void
Denise::setSPRxDATB(u16 value, u64 ext)
{
    assert(x < 8);
    logmsg(LOG_SPRREG, "setSPR%ldDATB(%X,%llX)\n", x, value, ext);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) { value = 0; ext = 0; }

    // Store the AGA extension (see setSPRxDATA)
    sprdatbExt[x] = ext;

    // Record the register change
    i64 pos = agnus.pos.pixel() + 8;
    constexpr auto reg = Reg(isize(Reg::SPR0DATB) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value });
}

u16
Denise::peekCOLORxx(isize xx)
{
    u16 result;
    
    // OCS and ECS chipsets do not support reading back color registers.
    // On AGA chipsets, color registers are readable when the RDRAM bit is set.

    if (isAGA() && denise.rdram()) {
        result = colorRegValue(xx);
    } else {
        result = mem.peekCustomFaulty16(u32(0x180 + 2 * xx));
    }
    
    logmsg(LOG_COLREG, "peekCOLOR%02ld = %x\n", xx, result);
    return result;
}

u16
Denise::spypeekCOLORxx(isize xx) const
{
    return isAGA() && rdram() ? colorRegValue(xx) : 0;
}

u16
Denise::colorRegValue(isize xx) const
{
    /* A read is addressed exactly like a write: BPLCON3 selects one of the
     * eight 32-color banks, and LOCT selects which nibble of each component
     * comes back.
     *
     * The register file is not updated as a write is issued. Color writes
     * queue in colChanges and are replayed once per line (see
     * PixelEngine::colorize and replayColRegChanges), so the file holds the
     * state as of the end of the previous line. A read therefore starts from
     * the file and applies, in order, every queued write to this register
     * recorded AT OR BEFORE the read position. Position matters: a write
     * recorded later in the same line has not happened yet and must stay
     * invisible, which is why this cannot simply take the newest entry.
     *
     * Relevant test: Denise/Registers/COLOR/rdram2, whose three sections are
     * write-before-read, write-after-read, and a write a line earlier.
     */
    auto reg = isize(colorBank()) * 32 + xx;
    auto now = agnus.pos.pixel();

    AmigaColor c = pixelEngine.color[reg];

    for (isize i = 0, end = pixelEngine.colChanges.end(); i < end; i++) {

        // The buffer is sorted by position, so the rest is in the future
        if (pixelEngine.colChanges.keys[i] > now) break;

        const RegChange &change = pixelEngine.colChanges.elements[i];

        // Skip the entries that are not color registers (see applyRegisterChange)
        if (change.reg == Reg(0)) continue;
        if (change.reg == Reg::BPLCON0) continue;
        if (change.reg == Reg::BPLCON2) continue;

        // The register number and the LOCT bit are encoded in the reg field
        if (((isize(change.reg) >> 8) & 0xFF) != reg) continue;

        if (!((isize(change.reg) >> 16) & 0xFF)) c.setHiNibbles(change.value);
        c.setLoNibbles(change.value);
    }

    return loct() ? c.getLoNibbles() : c.getHiNibbles();
}

template <isize xx, Accessor s> void
Denise::pokeCOLORxx(u16 value)
{
    logmsg(LOG_COLREG, "pokeCOLOR%02ld(%X)\n", xx, value);

    // Record the color change
    recordColorChange(xx, value);
    
    // constexpr auto reg = Reg(isize(Reg::COLOR00) + xx);
    // pixelEngine.colChanges.insert(agnus.pos.pixel(), RegChange { .reg = reg, .value = value } );
}

void
Denise::recordColorChange(isize nr, u16 value)
{
    assert(nr >= 0 && nr < 32);

    auto reg = isize(Reg::COLOR00) + nr;
    
    if (isAGA()) {

        // With RDRAM set, the color registers are read-only
        if (rdram()) return;

        /* AGA maintains 256 color registers with 8 bit per component. Writes
         * are directed to one of eight 32-color banks (BPLCON3 bits 13-15).
         * A write with the LOCT bit set carries the lower nibbles of the
         * components, which is how a program supplies the full 8 bit range.
         * We encode the actual target register and the value of the LOCT bit
         * in the higher bytes of the recorded reg value.
         */
        reg |= isize(loct()) << 16 | colorBank() << 13 | nr << 8;

    } else {

        assert(reg < 256);
        reg |= nr << 8;
    }

    pixelEngine.colChanges.insert(agnus.pos.pixel() + (isAGA() ? 2 : 0),
                                  RegChange { .reg = Reg(reg), .value = value } );
}

void
Denise::updateScrollOffsets()
{
    pixelOffsetOdd  = Pixel((bplcon1 & 0b00000001) << 2);
    pixelOffsetEven = Pixel((bplcon1 & 0b00010000) >> 2);
    
    /* AGA widens the scroll range from 16 to 64 lores pixels. The additional
     * bits are PF1H6 and PF1H7 in bits 11-10, and PF2H6 and PF2H7 in bits
     * 15-14, i.e. they extend the delay by two high-order bits (delay1 and
     * delay2 in Amiberry).
     *
     * The lower part of the delay is handled elsewhere: bits 3-1 shift the
     * drawing cycle (Agnus::scrollOdd / scrollEven), bit 0 becomes a pixel
     * offset. Everything above the length of one drawing cycle selects the
     * drawing cycle that reloads the shift registers, which is applied in
     * prepareOdd() and prepareEven() (see there).
     *
     * A drawing cycle emits 16 pixels, which corresponds to 16, 8, or 4 lores
     * pixels, depending on the resolution. Hence, the delay has to be divided
     * by that amount to obtain the number of words to wait.
     */
    u8 pf1h = u8( (bplcon1        & 0x000F) | ((bplcon1 & 0x0C00) >> 6));
    u8 pf2h = u8(((bplcon1 >> 4)  & 0x000F) | ((bplcon1 & 0xC000) >> 10));

    auto shift = res == Resolution::LORES ? 4 : res == Resolution::HIRES ? 3 : 2;

    scrollWordOdd  = isAGA() ? u8(pf1h >> shift) : 0;
    scrollWordEven = isAGA() ? u8(pf2h >> shift) : 0;
}

#if 0
void
Denise::updateScrollOffsets()
{
    pixelOffsetOdd  = Pixel((bplcon1 & 0b00000001) << 2);
    pixelOffsetEven = Pixel((bplcon1 & 0b00010000) >> 2);

    /* AGA widens the scroll range from 16 to 64 lores pixels. The additional
     * bits are PF1H6 and PF1H7 in bits 11-10, and PF2H6 and PF2H7 in bits
     * 15-14. They delay the output by whole 16 pixel words.
     *
     * This delay is not a pixel offset. It selects the drawing cycle that
     * reloads the shift registers, which is why it is applied in prepareOdd()
     * and prepareEven() instead.
     */
    scrollWordOdd  = isAGA() ? u8((bplcon1 & 0x0C00) >> 10) : 0;
    scrollWordEven = isAGA() ? u8((bplcon1 & 0xC000) >> 14) : 0;

}
#endif

Resolution
Denise::resolution(u16 v)
{
    if (GET_BIT(v,6) && !isOCS()) {
        return Resolution::SHRES;
    } else if (GET_BIT(v,15)) {
        return Resolution::HIRES;
    } else {
        return Resolution::LORES;
    }
}

u16
Denise::zPF(u16 prioBits)
{
    switch (prioBits) {

        case 0: return Z_0;
        case 1: return Z_1;
        case 2: return Z_2;
        case 3: return Z_3;
        case 4: return Z_4;
    }

    return 0;
}

u8
Denise::bpu(u16 v) const
{
    // Extract the three BPU bits
    u8 bpu = (v >> 12) & 0b111;
 
    if (isAGA()) {
        
        // Add the fourth bit
        if (GET_BIT(v, 4)) bpu |=0b1000;
        
        // An invalid value disabled all planes
        return bpu <= 8 ? bpu : 0;
        
    } else {
     
        // An invalid value enables all 6 planes
        return  bpu <= 6 ? bpu : 6;
    }
}

u8
Denise::sprBase(isize x) const
{
    /* On OCS and ECS machines, the value is hard-wired to 16. AGA lets
     * BPLCON4 select one of 16 banks, separately for the even and the
     * odd sprite of a pair (ESPRM and OSPRM). The reset value 0x0011
     * selects bank 1 for both, which yields 16 again.
     */
    if (isAGA()) {
        return u8((IS_ODD(x) ? osprm() : esprm()) << 4);
    } else {
        return 0x10;
    }
}

bool
Denise::borderSprites() const
{
    return isAGA() && GET_BIT(bplcon3, 1) && ecsena() && !brdrblnk();
}

Pixel
Denise::sprStrt(isize x) const
{
    /* With AGA scan doubling enabled, SPRxPOS bit 7 no longer contributes
     * to the horizontal position, because it has been taken over by the
     * scan doubling flag. The comparator ignores the corresponding bit
     * instead of evaluating it, which is why the sprite is matched twice
     * per line (see drawSpritePair).
     *
     * The bit is read from sprhpos, which is in hires units, but subtracted
     * from sprhppos, which is a buffer coordinate at super-hires resolution.
     * Hence the factor of two.
     */
    return agnus.sscan2() ? sprhppos[x] - 2 * (sprhpos[x] & 512) : sprhppos[x];
}

isize
Denise::sprPixelWidth() const
{
    /* Width of a sprite pixel in buffer entries, i.e. in super-hires pixels.
     * In AGA, BPLCON3 bits 7-6 (SPRES) set it independently of the playfield
     * resolution and of how many bits of data the sprite fetches.
     *
     * Neither the live bplcon0 nor initialBplcon0 is the right thing to read
     * here, and the two fail in opposite directions. drawSprites runs from the
     * hsync handler at h = $12, so the live register already carries the NEXT
     * line's Copper writes -- reading it applies them backwards and loses
     * SHRES for the whole line. initialBplcon0 is a snapshot taken before THIS
     * line's Copper writes, so on the one line where a list switches
     * resolution it hands back the PREVIOUS line's setting, which is the same
     * error one line earlier. spriteBplcon0 is initialBplcon0 advanced by
     * translate() up to spriteClipBegin, which is the value actually in force
     * where a sprite pixel can first appear.
     *
     * SPRES is still read from the start of the line. It is not tracked
     * positionally anywhere -- setBPLCON3 only records a change when BRDRBLNK
     * flips -- so a mid-line SPRES write is not modelled at all. Nothing in
     * the test suite exercises one.
     *
     * Relevant tests in the vAmigaTS test suite:
     * Denise/Sprites/aga/simple2, Denise/Registers/BPLCON4/bplam5 (line $D8)
     */
    if (isAGA()) {

        switch ((initialBplcon3 >> 6) & 0b11) {

            case 0b01: return 4;    // Lores
            case 0b10: return 2;    // Hires
            case 0b11: return 1;    // Super Hires
            default:   break;       // Same as OCS/ECS
        }
    }

    return shres(spriteBplcon0) ? 2 : 4;
}

isize
Denise::colorOffset(u16 bits) const
{
    /* PF2OF is an encoded field:
     *
     *   000 : none
     *   001 : 2    (plane 2 affected)
     *   010 : 4    (plane 3 affected)
     *   011 : 8    (plane 3 affected) (default)
     *   100 : 16   (plane 5 affected)
     *   101 : 32   (plane 6 affected)
     *   110 : 64   (plane 7 affected)
     *   111 : 128  (plane 8 affected)
     */
    
    if (isAGA()) return bits ? (1 << bits) : 0;
    
    // On OCS/ECS, the color offset is hard-coded to 8
    return 8;
}

template void Denise::pokeBPLCON0<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON0<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON1<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON1<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON2<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON2<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON3<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON3<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON4<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON4<Accessor::AGNUS>(u16 value);

template void Denise::pokeBPLxDAT<0,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<0,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<1,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<1,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<2,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<2,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<3,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<3,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<4,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<4,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<5,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<5,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<6,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<6,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<7,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<7,Accessor::AGNUS>(u16 value);

template void Denise::setBPLxDAT<0>(u16 value);
template void Denise::setBPLxDAT<1>(u16 value);
template void Denise::setBPLxDAT<2>(u16 value);
template void Denise::setBPLxDAT<3>(u16 value);
template void Denise::setBPLxDAT<4>(u16 value);
template void Denise::setBPLxDAT<5>(u16 value);
template void Denise::setBPLxDAT<6>(u16 value);
template void Denise::setBPLxDAT<7>(u16 value);

template void Denise::setBPLxDATExt<0>(u64 value, u8 count);
template void Denise::setBPLxDATExt<1>(u64 value, u8 count);
template void Denise::setBPLxDATExt<2>(u64 value, u8 count);
template void Denise::setBPLxDATExt<3>(u64 value, u8 count);
template void Denise::setBPLxDATExt<4>(u64 value, u8 count);
template void Denise::setBPLxDATExt<5>(u64 value, u8 count);
template void Denise::setBPLxDATExt<6>(u64 value, u8 count);
template void Denise::setBPLxDATExt<7>(u64 value, u8 count);

template void Denise::pokeSPRxPOS<0>(u16 value);
template void Denise::pokeSPRxPOS<1>(u16 value);
template void Denise::pokeSPRxPOS<2>(u16 value);
template void Denise::pokeSPRxPOS<3>(u16 value);
template void Denise::pokeSPRxPOS<4>(u16 value);
template void Denise::pokeSPRxPOS<5>(u16 value);
template void Denise::pokeSPRxPOS<6>(u16 value);
template void Denise::pokeSPRxPOS<7>(u16 value);

template void Denise::pokeSPRxCTL<0>(u16 value);
template void Denise::pokeSPRxCTL<1>(u16 value);
template void Denise::pokeSPRxCTL<2>(u16 value);
template void Denise::pokeSPRxCTL<3>(u16 value);
template void Denise::pokeSPRxCTL<4>(u16 value);
template void Denise::pokeSPRxCTL<5>(u16 value);
template void Denise::pokeSPRxCTL<6>(u16 value);
template void Denise::pokeSPRxCTL<7>(u16 value);

template void Denise::pokeSPRxDATA<0>(u16 value);
template void Denise::pokeSPRxDATA<1>(u16 value);
template void Denise::pokeSPRxDATA<2>(u16 value);
template void Denise::pokeSPRxDATA<3>(u16 value);
template void Denise::pokeSPRxDATA<4>(u16 value);
template void Denise::pokeSPRxDATA<5>(u16 value);
template void Denise::pokeSPRxDATA<6>(u16 value);
template void Denise::pokeSPRxDATA<7>(u16 value);

template void Denise::pokeSPRxDATB<0>(u16 value);
template void Denise::pokeSPRxDATB<1>(u16 value);
template void Denise::pokeSPRxDATB<2>(u16 value);
template void Denise::pokeSPRxDATB<3>(u16 value);
template void Denise::pokeSPRxDATB<4>(u16 value);
template void Denise::pokeSPRxDATB<5>(u16 value);
template void Denise::pokeSPRxDATB<6>(u16 value);
template void Denise::pokeSPRxDATB<7>(u16 value);

template void Denise::setSPRxDATA<0>(u16 value, u64 ext);
template void Denise::setSPRxDATA<1>(u16 value, u64 ext);
template void Denise::setSPRxDATA<2>(u16 value, u64 ext);
template void Denise::setSPRxDATA<3>(u16 value, u64 ext);
template void Denise::setSPRxDATA<4>(u16 value, u64 ext);
template void Denise::setSPRxDATA<5>(u16 value, u64 ext);
template void Denise::setSPRxDATA<6>(u16 value, u64 ext);
template void Denise::setSPRxDATA<7>(u16 value, u64 ext);

template void Denise::setSPRxDATB<0>(u16 value, u64 ext);
template void Denise::setSPRxDATB<1>(u16 value, u64 ext);
template void Denise::setSPRxDATB<2>(u16 value, u64 ext);
template void Denise::setSPRxDATB<3>(u16 value, u64 ext);
template void Denise::setSPRxDATB<4>(u16 value, u64 ext);
template void Denise::setSPRxDATB<5>(u16 value, u64 ext);
template void Denise::setSPRxDATB<6>(u16 value, u64 ext);
template void Denise::setSPRxDATB<7>(u16 value, u64 ext);

template void Denise::pokeCOLORxx<0, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<0, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<1, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<1, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<2, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<2, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<3, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<3, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<4, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<4, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<5, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<5, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<6, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<6, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<7, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<7, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<8, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<8, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<9, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<9, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<10, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<10, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<11, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<11, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<12, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<12, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<13, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<13, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<14, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<14, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<15, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<15, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<16, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<16, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<17, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<17, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<18, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<18, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<19, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<19, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<20, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<20, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<21, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<21, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<22, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<22, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<23, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<23, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<24, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<24, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<25, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<25, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<26, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<26, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<27, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<27, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<28, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<28, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<29, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<29, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<30, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<30, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<31, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<31, Accessor::AGNUS>(u16 value);

}
