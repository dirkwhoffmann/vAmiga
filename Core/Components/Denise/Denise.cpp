// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Emulator.h"
#include "Denise.h"
#include "Agnus.h"
#include "Amiga.h"

namespace vamiga {

Denise::Denise(Amiga& ref) : SubComponent(ref)
{    
    subComponents = std::vector<CoreComponent *> {
        
        &pixelEngine,
    };

    info.bind([this] { return cacheInfo(); } );
}

void
Denise::_didReset(bool hard)
{
    std::memset(bBuffer, 0, sizeof(bBuffer));
    std::memset(dBuffer, 0, sizeof(dBuffer));
    std::memset(iBuffer, 0, sizeof(iBuffer));
    std::memset(mBuffer, 0, sizeof(mBuffer));
    std::memset(zBuffer, 0, sizeof(zBuffer));

    // No BPL1DAT write has happened yet, so the display window is closed
    bplDatBegin = PIXEL_CNT;
    bBufferBplDatBegin = PIXEL_CNT;
    bBufferDiwOpen = PIXEL_CNT;

    // AGA: Reset color offset for the second playfield (PF2OF = b011)
    bplcon3 = 0x0C00;

    // AGA: Reset sprite color banks (ESPRM = b0001, OSPRM = b0001)
    bplcon4 = 0x0011;
}

i64
Denise::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::DENISE_REVISION:           return (i64)config.revision;
        case Opt::DENISE_SHRES_BLEND:        return config.shresBlend;
        case Opt::DENISE_VIEWPORT_TRACKING:  return config.viewportTracking;
        case Opt::DENISE_FRAME_SKIPPING:     return config.frameSkipping;
        case Opt::DENISE_HIDDEN_BITPLANES:   return config.hiddenBitplanes;
        case Opt::DENISE_HIDDEN_SPRITES:     return config.hiddenSprites;
        case Opt::DENISE_HIDDEN_LAYERS:      return config.hiddenLayers;
        case Opt::DENISE_HIDDEN_LAYER_ALPHA: return config.hiddenLayerAlpha;
        case Opt::DENISE_CLX_SPR_SPR:        return config.clxSprSpr;
        case Opt::DENISE_CLX_SPR_PLF:        return config.clxSprPlf;
        case Opt::DENISE_CLX_PLF_PLF:        return config.clxPlfPlf;
            
        default:
            fatalError;
    }
}

void
Denise::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::DENISE_REVISION:

            if (!DeniseRevEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, DeniseRevEnum::keyList());
            }
            return;

        case Opt::DENISE_SHRES_BLEND:
        case Opt::DENISE_VIEWPORT_TRACKING:
        case Opt::DENISE_FRAME_SKIPPING:
        case Opt::DENISE_HIDDEN_BITPLANES:
        case Opt::DENISE_HIDDEN_SPRITES:
        case Opt::DENISE_HIDDEN_LAYERS:
        case Opt::DENISE_HIDDEN_LAYER_ALPHA:
        case Opt::DENISE_CLX_SPR_SPR:
        case Opt::DENISE_CLX_SPR_PLF:
        case Opt::DENISE_CLX_PLF_PLF:

            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
Denise::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::DENISE_REVISION:

            config.revision = (DeniseRev)value;
            return;

        case Opt::DENISE_SHRES_BLEND:

            config.shresBlend = (bool)value;
            return;

        case Opt::DENISE_VIEWPORT_TRACKING:
            
            config.viewportTracking = (bool)value;
            debugger.resetDIWTracker();
            return;

        case Opt::DENISE_FRAME_SKIPPING:

            config.frameSkipping = (isize)value;
            return;

        case Opt::DENISE_HIDDEN_BITPLANES:
            
            config.hiddenBitplanes = (u8)value;
            return;

        case Opt::DENISE_HIDDEN_SPRITES:
            
            config.hiddenSprites = (u8)value;
            return;
            
        case Opt::DENISE_HIDDEN_LAYERS:
            
            config.hiddenLayers = (u16)value;
            return;
            
        case Opt::DENISE_HIDDEN_LAYER_ALPHA:
            
            config.hiddenLayerAlpha = (u8)value;
            return;

        case Opt::DENISE_CLX_SPR_SPR:
            
            config.clxSprSpr = (bool)value;
            return;
            
        case Opt::DENISE_CLX_SPR_PLF:
            
            config.clxSprPlf = (bool)value;
            return;
            
        case Opt::DENISE_CLX_PLF_PLF:
            
            config.clxPlfPlf = (bool)value;
            return;

        default:
            fatalError;
    }
}

void
Denise::updateSprHCoords(isize x)
{
    // Assemble the horizontal sprite coordinate
    sprhpos[x] = u16((sprpos[x] & 0xFF) << 2 |  // SH1 - SH8
                     (sprctl[x] & 0x01) << 1 |  // SH0
                     (sprctl[x] & 0x10) >> 4);  // SHSH1 (ECS only)

    // Convert to a buffer position (super-hires resolution)
    sprhppos[x] = 2 * sprhpos[x] + 4 - 8 * HBLANK_MIN;
}

bool
Denise::spritePixelIsVisible(Pixel hpos) const
{
    u16 z = zBuffer[hpos];
    return (z & Z_SP01234567) > (z & ~Z_SP01234567);
}

void
Denise::updateShiftRegistersOdd()
{
    switch (bpu()) {
            
        case 8:
        case 7: shiftReg[6] = bpldatPipe[6];
        case 6:
        case 5: shiftReg[4] = bpldatPipe[4];
        case 4:
        case 3: shiftReg[2] = bpldatPipe[2];
        case 2:
        case 1: shiftReg[0] = bpldatPipe[0];
    }
}

void
Denise::updateShiftRegistersEven()
{
    switch (bpu()) {
            
        case 8: shiftReg[7] = bpldatPipe[7];
        case 7:
        case 6: shiftReg[5] = bpldatPipe[5];
        case 5:
        case 4: shiftReg[3] = bpldatPipe[3];
        case 3:
        case 2: shiftReg[1] = bpldatPipe[1];
    }
}

void
Denise::extractSlices(u8 slices[16])
{
    u16 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slices[i] = (u8) ((!!(shiftReg[0] & mask) << 0) |
                          (!!(shiftReg[1] & mask) << 1) |
                          (!!(shiftReg[2] & mask) << 2) |
                          (!!(shiftReg[3] & mask) << 3) |
                          (!!(shiftReg[4] & mask) << 4) |
                          (!!(shiftReg[5] & mask) << 5) |
                          (!!(shiftReg[6] & mask) << 6) |
                          (!!(shiftReg[7] & mask) << 7) );
    }
}

void
Denise::extractSlicesOdd(u8 slices[16])
{
    u16 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slices[i] = (u8) ((!!(shiftReg[0] & mask) << 0) |
                          (!!(shiftReg[2] & mask) << 2) |
                          (!!(shiftReg[4] & mask) << 4) |
                          (!!(shiftReg[6] & mask) << 6) );
    }
}

void
Denise::extractSlicesEven(u8 slices[16])
{
    u16 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slices[i] = (u8) ((!!(shiftReg[1] & mask) << 1) |
                          (!!(shiftReg[3] & mask) << 3) |
                          (!!(shiftReg[5] & mask) << 5) |
                          (!!(shiftReg[7] & mask) << 7));
    }
}

bool
Denise::isReloadCycle(u8 scrollWord) const
{
    /* The extended scroll bits delay the output by whole words. Real hardware
     * implements this delay by reloading the shift registers at a drawing cycle
     * whose position matches the scroll value. The relevant positions repeat
     * with the length of a fetch unit.
     */
    // Length of a drawing cycle in DMA cycles
    auto step = res == Resolution::LORES ? 8 : res == Resolution::HIRES ? 4 : 2;

    /* Number of words delivered by a single fetch. The delay cannot exceed this
     * amount, which is why the extended bits stay without effect in 16 bit
     * fetch mode (bplcon1_shift_mask in Amiberry).
     */
    auto words = isize(agnus.bplFetchWords()); //  sequencer.fetchWords);
    if (words < 2) return true;

    // The scroll value selects one of the words of the fetch
    auto slot = scrollWord & (words - 1);

    return ((agnus.pos.h / step) % words) == slot;
}

#if 0
bool
Denise::isReloadCycle(u8 scrollWord) const
{
    /* The shift registers have to be reloadable as often as a new fetch
     * completes, because data that finds no reload cycle before the next
     * fetch arrives is simply lost. One fetch delivers as many words as FMODE
     * selects and each word feeds one drawing cycle, so a fetch spans
     *
     *     words * step
     *
     * DMA cycles, where step is the length of a drawing cycle. Note that this
     * is not the fetch unit size reported by the sequencer: that one counts
     * in units of eight DMA cycles and is always 1 for super hires, which
     * says nothing about how long a super hires fetch actually lasts.
     *
     * Relevant tests in the vAmigaTS test suite:
     * Denise/Modes/shres/shres00 to shres11 and Agnus/Registers/FMODE
     */
    auto fm = agnus.fmode & 0b11;

    // Number of words delivered by a single fetch
    auto words = fm == 0b11 ? 4 : fm == 0b00 ? 1 : 2;

    // Length of a drawing cycle in DMA cycles
    auto step = res == Resolution::LORES ? 8 : res == Resolution::HIRES ? 4 : 2;

    /* Convert the delay from words into drawing cycles. The scroll delay is
     * measured in whole 16 lores pixel words, hence in units of eight DMA
     * cycles, so a delay that exceeds the length of a fetch wraps around.
     */
    auto slot = (scrollWord * 8 / step) % words;

    return ((agnus.pos.h % (words * step)) / step) == slot;
}
#endif

void
Denise::prepareOdd()
{
    if (latchedOdd && isReloadCycle(scrollWordOdd)) {

        for (isize i = 0; i < 8; i += 2) {

            bpldatPipe[i] = bpldatLatch[i];
            bpldatPipeExt[i] = bpldatLatchExt[i];
        }
        extCntOdd = latchExtCnt;
        latchedOdd = false;
        armedOdd = true;

    } else {

        // Keep emitting the remaining words of the current fetch. Latched data
        // has to wait for its reload cycle, just like the shift registers of
        // the real hardware keep shifting until they are reloaded.

        feedPipeOdd();
    }
}

void
Denise::prepareEven()
{
    if (latchedEven && isReloadCycle(scrollWordEven)) {

        for (isize i = 1; i < 8; i += 2) {

            bpldatPipe[i] = bpldatLatch[i];
            bpldatPipeExt[i] = bpldatLatchExt[i];
        }
        extCntEven = latchExtCnt;
        latchedEven = false;
        armedEven = true;

    } else {

        feedPipeEven();
    }
}

void
Denise::feedPipeOdd()
{
    if (extCntOdd == 0) return;

    for (isize i = 0; i < 8; i += 2) {

        bpldatPipe[i] = (u16)(bpldatPipeExt[i] & 0xFFFF);
        bpldatPipeExt[i] >>= 16;
    }
    extCntOdd--;
    armedOdd = true;
}

void
Denise::feedPipeEven()
{
    if (extCntEven == 0) return;

    for (isize i = 1; i < 8; i += 2) {

        bpldatPipe[i] = (u16)(bpldatPipeExt[i] & 0xFFFF);
        bpldatPipeExt[i] >>= 16;
    }
    extCntEven--;
    armedEven = true;
}

template <Resolution mode> void
Denise::drawOdd(Pixel offset)
{
    static constexpr u16 masks[9] = {
        
        0b00000000, // 0 bitplanes
        0b00000001, // 1 bitplanes
        0b00000001, // 2 bitplanes
        0b00000101, // 3 bitplanes
        0b00000101, // 4 bitplanes
        0b00010101, // 5 bitplanes
        0b00010101, // 6 bitplanes
        0b01010101, // 7 bitplanes
        0b01010101  // 8 bitplanes
    };
    
    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 4;

    u8 slices[16];
    extractSlicesOdd(slices);

    for (isize i = 0; i < 16; i++) {

        u8 index = slices[i] & mask;

        switch (mode) {

            case Resolution::LORES:

                // Synthesize a lores pixel (four buffer entries)
                assert(pixel + 3 < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                break;

            case Resolution::HIRES:

                // Synthesize a hires pixel (two buffer entries)
                assert(pixel + 1 < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                break;

            case Resolution::SHRES:

                // Synthesize a superHires pixel (one buffer entry)
                assert(pixel < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b10101010) | index;
                pixel++;
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    shiftReg[0] = shiftReg[2] = shiftReg[4] = shiftReg[6] = 0;
}

template <Resolution mode> void
Denise::drawEven(Pixel offset)
{    
    static constexpr u16 masks[9] = {
        
        0b00000000, // 0 bitplanes
        0b00000000, // 1 bitplanes
        0b00000010, // 2 bitplanes
        0b00000010, // 3 bitplanes
        0b00001010, // 4 bitplanes
        0b00001010, // 5 bitplanes
        0b00101010, // 6 bitplanes
        0b10101010, // 7 bitplanes
        0b10101010  // 8 bitplanes
    };
    
    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 4;

    u8 slices[16];
    extractSlicesEven(slices);

    for (isize i = 0; i < 16; i++) {

        u8 index = slices[i] & mask;

        switch (mode) {

            case Resolution::LORES:

                // Synthesize a lores pixel (four buffer entries)
                assert(pixel + 3 < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                break;

            case Resolution::HIRES:

                // Synthesize a hires pixel (two buffer entries)
                assert(pixel + 1 < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                break;

            case Resolution::SHRES:

                // Synthesize a superHires pixel (one buffer entry)
                assert(pixel < BUF_CNT);
                dBuffer[pixel] = (dBuffer[pixel] & 0b01010101) | index;
                pixel++;
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    shiftReg[1] = shiftReg[3] = shiftReg[5] = shiftReg[7] = 0;
}

template <Resolution mode> void
Denise::drawBoth(Pixel offset)
{
    if CONSTEXPR (BPL_ON_STEROIDS) {

        drawOdd <mode> (offset);
        drawEven <mode> (offset);
        return;
    }

    static constexpr u16 masks[9] = {

        0b00000000, // 0 bitplanes
        0b00000001, // 1 bitplanes
        0b00000011, // 2 bitplanes
        0b00000111, // 3 bitplanes
        0b00001111, // 4 bitplanes
        0b00011111, // 5 bitplanes
        0b00111111, // 6 bitplanes
        0b01111111, // 7 bitplanes
        0b11111111  // 8 bitplanes
    };

    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 4;

    u8 slices[16];
    extractSlices(slices);

    for (isize i = 0; i < 16; i++) {

        u8 index = slices[i] & mask;

        switch (mode) {

            case Resolution::LORES:

                // Synthesize a lores pixel (four buffer entries)
                assert(pixel + 3 < BUF_CNT);
                dBuffer[pixel] = index;
                pixel++;
                dBuffer[pixel] = index;
                pixel++;
                dBuffer[pixel] = index;
                pixel++;
                dBuffer[pixel] = index;
                pixel++;
                break;

            case Resolution::HIRES:

                // Synthesize a hires pixel (two buffer entries)
                assert(pixel + 1 < BUF_CNT);
                dBuffer[pixel] = index;
                pixel++;
                dBuffer[pixel] = index;
                pixel++;
                break;

            case Resolution::SHRES:

                // Synthesize a superHires pixel (one buffer entry)
                assert(pixel < BUF_CNT);
                dBuffer[pixel] = index;
                pixel++;
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    for (isize i = 0; i < 8; i++) shiftReg[i] = 0;
}

void
Denise::drawLoresOdd()
{
    if (isAGA()) prepareOdd();
    
    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <Resolution::LORES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawLoresEven()
{
    if (isAGA()) prepareEven();
    
    if (armedEven) {
        
        updateShiftRegistersEven();
        drawEven <Resolution::LORES> (pixelOffsetEven);
        armedEven = false;
    }
}

void
Denise::drawLoresBoth()
{
    drawLoresOdd();
    drawLoresEven();
}

void
Denise::drawHiresOdd()
{
    if (isAGA()) prepareOdd();
    
    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <Resolution::HIRES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawHiresEven()
{
    if (isAGA()) prepareEven();
    
    if (armedEven) {

        updateShiftRegistersEven();
        drawEven <Resolution::HIRES> (pixelOffsetEven);
        armedEven = false;
    }
}

void
Denise::drawHiresBoth()
{
    drawHiresOdd();
    drawHiresEven();
}

void
Denise::drawShresOdd()
{
    if (isAGA()) prepareOdd();

    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <Resolution::SHRES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawShresEven()
{
    if (isAGA()) prepareEven();
    
    if (armedEven) {

        updateShiftRegistersEven();
        drawEven <Resolution::SHRES> (pixelOffsetEven);
        armedEven = false;
    }
}

void
Denise::drawShresBoth()
{
    drawShresOdd();
    drawShresEven();
}

void
Denise::translate()
{
    Pixel pixel = 0;

    // Wipe out some bitplane data if requested
    if (config.hiddenBitplanes) {

        for (isize i = 0; i < BUF_CNT; i++) {
            dBuffer[i] &= ~config.hiddenBitplanes;
        }
    }
    
    // Initialize the mBuffer with a copy of the dBuffer
    std::memcpy(mBuffer, dBuffer, sizeof(mBuffer));

    // Start with the playfield state as it was at the beginning of the line
    PFState state;
    state.zpf1 = zPF1(initialBplcon2);
    state.zpf2 = zPF2(initialBplcon2);
    state.prio = pf2pri(initialBplcon2);
    state.ham = ham(initialBplcon0);
    state.colxor = bplam(initialBplcon4);

    bool dual = dbplf(initialBplcon0);
    bool hamLine = state.ham;

    /* The sprites of this line are drawn straight after this walk, and they
     * need the BPLCON0 in force where they appear rather than the one from the
     * start of the line. This loop already visits every BPLCON0 change in
     * order, so it is the cheapest place to pick that up. See spriteBplcon0.
     */
    spriteBplcon0 = initialBplcon0;

    // Add a dummy register change to ensure we draw until the line ends
    conChanges.insert(BUF_CNT, RegChange { .reg = Reg(0), .value = 0 });

    // Iterate over all recorded register changes
    for (isize i = 0, end = conChanges.end(); i < end; i++) {

        Pixel trigger = (Pixel)conChanges.keys[i];
        RegChange &change = conChanges.elements[i];

        // Translate a chunk of bitplane data
        if (dual) {
            translateDPF(pixel, trigger, state);
        } else {
            translateSPF(pixel, trigger, state);
        }
        pixel = trigger;

        // Apply the register change
        switch (change.reg) {

            case Reg::BPLCON0:

                dual = dbplf(bplcon0);
                state.ham = ham(change.value);
                hamLine |= state.ham;
                if (trigger <= spriteClipBegin) spriteBplcon0 = change.value;
                break;

            case Reg::BPLCON2:
                
                state.prio = pf2pri(change.value);
                state.zpf1 = zPF1(change.value);
                state.zpf2 = zPF2(change.value);
                break;

            case Reg::BPLCON4:

                state.colxor = bplam(change.value);
                break;

            default:
                
                assert(change.reg == Reg(0));
                break;
        }
    }

    // Clear the history cache
    conChanges.clear();

    // Create the iBuffer (only needed for lines with HAM mode enabled)
    if (hamLine) std::memcpy(iBuffer, mBuffer, sizeof(iBuffer));
}

void
Denise::translateSPF(Pixel from, Pixel to, PFState &state)
{
    u8 colxor = state.colxor;
    
    /* Check for invalid bitplane modes. If the priority of the second bitplane
     * is set to an invalid value (> 4), Denise ignores the data from the first
     * four bitplanes whereever the fifth bitplane is set to 1. Some demos such
     * as "Planet Rocklobster" (Oxyron) show that this kind of bitplane
     * elimination does not happen in HAM mode.
     *
     * Relevant tests in the vAmigaTS test suite:
     * Denise/BPLCON0/invprio0 to Denise/BPLCON0/invprio3
     */
    
    if (!isAGA() && !state.zpf2 && !state.ham) {
        
        for (Pixel i = from; i < to; i++) {

            u8 s = dBuffer[i];

            assert(PixelEngine::isPaletteIndex(s));
            mBuffer[i] = (s & 0x10) ? (s & 0x30) : s;
            zBuffer[i] = 0;
        }
        return;
    }
    
    // Translate the usual way
    for (Pixel i = from; i < to; i++) {
        
        u8 s = dBuffer[i];
        u8 c = s ^ colxor;

        mBuffer[i] = c;
        zBuffer[i] = s ? state.zpf2 : 0;
    }
}

void
Denise::translateDPF(Pixel from, Pixel to, PFState &state)
{
    if (state.prio) {
        translateDPF <true> (from, to, state);
    } else {
        translateDPF <false> (from, to, state);
    }
}

template <bool prio> void
Denise::translateDPF(Pixel from, Pixel to, PFState &state)
{
    u8 colxor = state.colxor;
    
    /* If the priority of a playfield is set to an illegal value (zpf1 or
     * zpf2 will be 0 in that case), all pixels are drawn transparent.
     */
    u8 mask1 = state.zpf1 ? 0xFF : 0x00;
    u8 mask2 = state.zpf2 ? 0xFF : 0x00;
    
    // u8 mask1 = state.zpf1 ? 0b1111 : 0b0000;
    // u8 mask2 = state.zpf2 ? 0b1111 : 0b0000;

    /* Contribution of the two additional bitplanes. In AGA, dual playfield
     * mode can use all eight bitplanes, which widens the color index of each
     * playfield from three to four bits. OCS and ECS never set these bits, so
     * masking them out keeps their index computation unchanged.
     */
    // u8 bpl7 = isAGA() ? 0x40 : 0x00;
    // u8 bpl8 = isAGA() ? 0x80 : 0x00;

    // Mask to blend out bitplanes 7 and 8 on OCS and ECS.
    u8 bpMask = isAGA() ? 0xFF : 0x3F;
    
    // Color offset of the second playfield
    u8 ofs2 = (u8)colorOffset(pf2of());
    
    for (Pixel i = from; i < to; i++) {

        u8 s = dBuffer[i] & bpMask;

        // Determine color indices for both playfields
        u8 index1 = u8(((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2) | ((s & 64) >> 3));
        u8 index2 = u8(((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3) | ((s & 128) >> 4));
        
        // u8 index1 = (((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2));
        // u8 index2 = (((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3));

        // Apply the color offset of the second playfield
        u8 col2 = u8((index2 + ofs2) & mask2);

        if (index1) {
            
            if (index2) {

                // PF1 is solid, PF2 is solid
                if (prio) {
                    mBuffer[i] = col2 ^ colxor;
                    zBuffer[i] = state.zpf2 | Z_DPF21;
                } else {
                    mBuffer[i] = (index1 & mask1) ^ colxor;
                    zBuffer[i] = state.zpf1 | Z_DPF12;
                }

            } else {

                // PF1 is solid, PF2 is transparent
                mBuffer[i] = (index1 & mask1) ^ colxor;
                zBuffer[i] = state.zpf1 | Z_DPF1;
            }

        } else {
            
            if (index2) {

                // PF1 is transparent, PF2 is solid
                mBuffer[i] = col2 ^ colxor;
                zBuffer[i] = state.zpf2 | Z_DPF2;

            } else {

                // PF1 is transparent, PF2 is transparent
                mBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }
        }
    }
}

void
Denise::drawSprites()
{
    switch (sprPixelWidth()) {

        case 1:  drawSprites<Resolution::SHRES>(); break;
        case 2:  drawSprites<Resolution::HIRES>(); break;
        default: drawSprites<Resolution::LORES>(); break;
    }
}

template <Resolution R> void
Denise::drawSprites()
{
    if (wasArmed) {
        
        if (wasArmed & 0b11000000) drawSpritePair<3, R>();
        if (wasArmed & 0b00110000) drawSpritePair<2, R>();
        if (wasArmed & 0b00001100) drawSpritePair<1, R>();
        if (wasArmed & 0b00000011) drawSpritePair<0, R>();
        
        // Record sprite data in debug mode
        if (emulator.isTracking()) debugger.recordSprites(wasArmed);
    }
    
    /* If a sprite was armed, the code above has been executed which means
     * that all recorded register changes have been applied and the relevant
     * sprite registers are all up to date at this time. For unarmed sprites,
     * however, the register change buffers may contain unprocessed entried.
     * We replay those to get the sprite registers up to date.
     */
    if (!sprChanges[3].isEmpty()) replaySpriteRegChanges<3>();
    if (!sprChanges[2].isEmpty()) replaySpriteRegChanges<2>();
    if (!sprChanges[1].isEmpty()) replaySpriteRegChanges<1>();
    if (!sprChanges[0].isEmpty()) replaySpriteRegChanges<0>();
}

template <isize pair, Resolution R> void
Denise::drawSpritePair()
{
    constexpr isize sprite1 = 2 * pair;
    constexpr isize sprite2 = 2 * pair + 1;
    constexpr Pixel hposMask = sprPixelMask<R>();

    Pixel strt = 0;
    Pixel strt1 = sprStrt(sprite1) & hposMask;
    Pixel strt2 = sprStrt(sprite2) & hposMask;
    // Pixel strt1 = sprhppos[sprite1] & hposMask;
    // Pixel strt2 = sprhppos[sprite2] & hposMask;
    
    // Iterate over all recorded register changes
    if (!sprChanges[pair].isEmpty()) {

        for (isize i = 0, end = sprChanges[pair].end(); i < end; i++) {
            
            Pixel trigger = (Pixel)sprChanges[pair].keys[i];
            RegChange &change = sprChanges[pair].elements[i];
            
            // Draw a chunk of pixels
            drawSpritePair <pair,R> (strt, trigger, strt1, strt2);
            strt = trigger;
            
            // Apply the recorded register change
            switch (isize(change.reg)) {
                    
                case isize(Reg::SPR0DATA) + 4 * sprite1:
                    
                    sprdata[sprite1] = change.value;
                    SET_BIT(armed, sprite1);
                    break;
                    
                case isize(Reg::SPR0DATA) + 4 * sprite2:
                    
                    sprdata[sprite2] = change.value;
                    SET_BIT(armed, sprite2);
                    break;
                    
                case isize(Reg::SPR0DATB) + 4 * sprite1:
                    
                    sprdatb[sprite1] = change.value;
                    break;
                    
                case isize(Reg::SPR0DATB) + 4 * sprite2:
                    
                    sprdatb[sprite2] = change.value;
                    break;

                case isize(Reg::SPR0POS) + 4 * sprite1:

                    setSPRxPOS(sprite1, change.value);
                    strt1 = sprhppos[sprite1] & hposMask;
                    break;
                    
                case isize(Reg::SPR0POS) + 4 * sprite2:
                    
                    setSPRxPOS(sprite2, change.value);
                    strt2 = sprhppos[sprite2] & hposMask;
                    break;
                    
                case isize(Reg::SPR0CTL) + 4 * sprite1:

                    setSPRxCTL(sprite1, change.value);
                    strt1 = sprhppos[sprite1] & hposMask;
                    CLR_BIT(armed, sprite1);
                    break;
                    
                case isize(Reg::SPR0CTL) + 4 * sprite2:

                    setSPRxCTL(sprite2, change.value);
                    strt2 = sprhppos[sprite2] & hposMask;
                    CLR_BIT(armed, sprite2);
                    break;

                default:
                    fatalError;
            }
        }
    }
    
    // Draw until the end of the line
    drawSpritePair <pair,R> (strt, BUF_CNT - 1, strt1, strt2);
    
    sprChanges[pair].clear();
}

template <isize pair> void
Denise::replaySpriteRegChanges()
{
    constexpr isize sprite1 = 2 * pair;
    constexpr isize sprite2 = 2 * pair + 1;
    
    for (isize i = 0, end = sprChanges[pair].end(); i < end; i++) {
        
        RegChange &change = sprChanges[pair].elements[i];
        
        // Apply the recorded register change
        switch (isize(change.reg)) {
                
            case isize(Reg::SPR0DATA) + 4 * sprite1:
                
                sprdata[sprite1] = change.value;
                break;
                
            case isize(Reg::SPR0DATA) + 4 * sprite2:
                
                sprdata[sprite2] = change.value;
                break;
                
            case isize(Reg::SPR0DATB) + 4 * sprite1:
                
                sprdatb[sprite1] = change.value;
                break;
                
            case isize(Reg::SPR0DATB) + 4 * sprite2:
                
                sprdatb[sprite2] = change.value;
                break;
                
            case isize(Reg::SPR0POS) + 4 * sprite1:

                setSPRxPOS(sprite1, change.value);
                assert(sprpos[sprite1] == change.value);
                break;
                
            case isize(Reg::SPR0POS) + 4 * sprite2:

                setSPRxPOS(sprite2, change.value);
                assert(sprpos[sprite2] == change.value);
                break;
                
            case isize(Reg::SPR0CTL) + 4 * sprite1:

                setSPRxCTL(sprite1, change.value);
                assert(sprctl[sprite1] == change.value);
                break;
                
            case isize(Reg::SPR0CTL) + 4 * sprite2:

                setSPRxCTL(sprite2, change.value);
                assert(sprctl[sprite2] == change.value);
                break;
                
            default:
                fatalError;
        }
    }
    
    sprChanges[pair].clear();
}

template <isize pair, Resolution R> void
Denise::drawSpritePair(Pixel hstrt, Pixel hstop, Pixel strt1, Pixel strt2)
{
    assert(pair < 4);
    
    // Only proceeed if we are outside the VBLANK area
    if (agnus.inVBlankArea()) return;
    
    constexpr isize sprite1 = 2 * pair;
    constexpr isize sprite2 = 2 * pair + 1;

    assert(hstrt <= BUF_CNT);
    assert(hstop <= BUF_CNT);

    bool armed1 = GET_BIT(armed, sprite1);
    bool armed2 = GET_BIT(armed, sprite2);

    bool attached = GET_BIT(sprctl[sprite2], 7);
    constexpr Pixel offset = sprPixelSize<R>();

    /* Second trigger position of the horizontal comparator. Scan doubling
     * shortens the comparator by one bit (see sprStrt), so the sprite is
     * matched a second time half a comparator period further to the right.
     * A value of zero disables the second trigger.
     */
    Pixel wrap = agnus.sscan2() ? 1024 : 0;

    for (Pixel hpos = hstrt; hpos < hstop; hpos += offset) {

        if (armed1 && (hpos == strt1 || (wrap && hpos == strt1 + wrap))) {
                    
            ssra[sprite1] = loadSSR(sprdata[sprite1], sprdataExt[sprite1]);
            ssrb[sprite1] = loadSSR(sprdatb[sprite1], sprdatbExt[sprite1]);
        }
        if (armed2 && (hpos == strt2 || (wrap && hpos == strt2 + wrap))) {
                    
            ssra[sprite2] = loadSSR(sprdata[sprite2], sprdataExt[sprite2]);
            ssrb[sprite2] = loadSSR(sprdatb[sprite2], sprdatbExt[sprite2]);
        }
        /*
        if (hpos == strt1 && armed1) {
            
            ssra[sprite1] = sprdata[sprite1];
            ssrb[sprite1] = sprdatb[sprite1];
        }
        if (hpos == strt2 && armed2) {
            
            ssra[sprite2] = sprdata[sprite2];
            ssrb[sprite2] = sprdatb[sprite2];
        }
        */

        if (ssra[sprite1] | ssrb[sprite1] | ssra[sprite2] | ssrb[sprite2]) {

            /* A sprite pixel covers several buffer entries -- four in lores,
             * two in hires, one in super hires -- and the window edge does
             * not land on one of its boundaries. bplDatBegin is the first
             * BPL1DAT write plus BPLDAT_LATENCY, which puts it two entries
             * into a four entry lores pixel. Testing hpos alone discards that
             * straddling pixel whole and starts the sprite a grid step later,
             * leaving one screen column of playfield between the border and
             * the sprite. Admitting the pixel as soon as ANY of its entries is
             * inside the range, and dropping the outside entries one by one
             * below, draws the part that belongs on screen.
             *
             * An A1200 and an A500+ show no such column: the sprite runs
             * straight into the border. Measured in
             * Denise/Sprites/clip/sprgate, whose clipped bands read 0 in
             * lores and hires on both machines. This is what makes them 0 --
             * SPRITE_LATENCY alone does not. With the latency at zero but the
             * pixel discarded whole they still read 1.
             *
             * NOTE: only the left edge is governed from here. spriteClipEnd is
             * PIXEL_CNT + 64, so the right hand test never fires; a sprite is
             * bounded on the right by hstop in the loop above, which cuts on
             * the same pixel granularity and has the same defect unaddressed.
             */
            if (hpos + offset > spriteClipBegin && hpos < spriteClipEnd) {

                if (attached) {
                    
                    drawAttachedSpritePixelPair <sprite2,R> (hpos);
                    
                } else {
                    
                    drawSpritePixel <sprite1,R> (hpos);
                    drawSpritePixel <sprite2,R> (hpos);
                }
            }
            
            ssra[sprite1] <<= 1;
            ssrb[sprite1] <<= 1;
            ssra[sprite2] <<= 1;
            ssrb[sprite2] <<= 1;
            /*
            ssra[sprite1] = (u16)(ssra[sprite1] << 1);
            ssrb[sprite1] = (u16)(ssrb[sprite1] << 1);
            ssra[sprite2] = (u16)(ssra[sprite2] << 1);
            ssrb[sprite2] = (u16)(ssrb[sprite2] << 1);
            */
        }
    }

    // Perform collision checks (if enabled)
    Pixel width = isAGA() ? Pixel(agnus.spriteWidth() * offset) : 64;
    
    if (config.clxSprSpr) {
        
        checkS2SCollisions<2 * pair>(strt1, strt1 + width - 1);
        checkS2SCollisions<2 * pair + 1>(strt2, strt2 + width - 1);
        // checkS2SCollisions<2 * pair>(strt1, strt1 + 31);
        // checkS2SCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
    if (config.clxSprPlf) {
        
        checkS2PCollisions<2 * pair>(strt1, strt1 + width - 1);
        checkS2PCollisions<2 * pair + 1>(strt2, strt2 + width - 1);
        // checkS2PCollisions<2 * pair>(strt1, strt1 + 31);
        // checkS2PCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
}

template <isize x, Resolution R> void
Denise::drawSpritePixel(Pixel hpos)
{
    // The pixel may straddle either end of the range; see drawSpritePair
    assert(hpos + sprPixelSize<R>() > spriteClipBegin && hpos < spriteClipEnd);

    // u8 a = (ssra[x] >> 15);
    // u8 b = (ssrb[x] >> 14) & 2;
    u8 a = u8(ssra[x] >> 63);
    u8 b = u8(ssrb[x] >> 62) & 2;
    u8 col = a | b;

    if (col) {

        u16 z = Z_SP[x];
        u8 base = u8(sprBase(x) + 2 * (x & 6));
        // u8 base = 16 + 2 * (x & 6);

        /* A sprite pixel covers as many buffer entries as it is wide (see
         * drawSpritePair, which advances hpos by the same amount).
         */
        constexpr Pixel width = sprPixelSize<R>();

        for (Pixel i = 0; i < width; i++) {

            // Drop the entries of this pixel that fall outside the range
            if (hpos + i < spriteClipBegin) continue;
            if (hpos + i >= spriteClipEnd) break;

            if (z > zBuffer[hpos + i]) mBuffer[hpos + i] = base | col;
            zBuffer[hpos + i] |= z;
        }
    }
}

template <isize x, Resolution R> void
Denise::drawAttachedSpritePixelPair(Pixel hpos)
{
    assert(IS_ODD(x));
    // The pixel may straddle either end of the range; see drawSpritePair
    assert(hpos + sprPixelSize<R>() > spriteClipBegin && hpos < spriteClipEnd);

    u8 col =
    u8((ssra[x-1] >> 63) & 0b0001) |
    u8((ssrb[x-1] >> 62) & 0b0010) |
    u8((ssra[x]   >> 61) & 0b0100) |
    u8((ssrb[x]   >> 60) & 0b1000) ;
    // ((ssra[x-1] >> 15) & 0b0001) |
    // ((ssrb[x-1] >> 14) & 0b0010) |
    // ((ssra[x]   >> 13) & 0b0100) |
    // ((ssrb[x]   >> 12) & 0b1000) ;
    
    if (col) {

        u16 z = Z_SP[x];

        // See drawSpritePixel for the width of a sprite pixel
        constexpr Pixel width = sprPixelSize<R>();

        for (Pixel i = 0; i < width; i++) {

            // Drop the entries of this pixel that fall outside the range
            if (hpos + i < spriteClipBegin) continue;
            if (hpos + i >= spriteClipEnd) break;

            if (z > zBuffer[hpos + i]) {

                mBuffer[hpos + i] = sprBase(x) | col;
                zBuffer[hpos + i] |= z;
            }
        }
    }
}

u8
Denise::borderColor(u16 con0, u16 con3) const
{
    /* The caller passes the register values in force at the start of the
     * rasterline, not the live ones. updateBorderBuffer runs in the hsync
     * handler, which finishes the line that just ended, so reading the
     * registers directly would pick up writes the Copper has already made
     * at the top of the NEXT line and leak them one line backwards.
     *
     * Relevant test in the vAmigaTS test suite: Denise/Modes/brdrblnk
     */
    if CONSTEXPR (BORDER_DEBUG) {
        return PixelEngine::BORDER_DEBUG; // Debug color
    }

    if (!isOCS() && ecsena(con0) && brdrblnk(con3)) {
        return PixelEngine::BORDER_BLNK; // Pure black
    }

    return PixelEngine::BORDER_BG; // Background color
}

void
Denise::updateBorderBuffer()
{
    /* The buffer depends on where the display window was opened, which is a
     * property of the line that just ended and not of the DIW registers. A
     * line that opens it at a different position than the previous one needs
     * a rebuild even if nothing else has changed.
     */
    if (bplDatBegin != bBufferBplDatBegin) markBorderBufferAsDirty(1);
    bBufferBplDatBegin = bplDatBegin;

    // Only proceed if the buffer is dirty
    if (!borderBufferIsDirty) return;
    denise.borderBufferIsDirty--;

    /* Determine the border color at the start of this rasterline. BRDRBLNK
     * and ECSENA are not line-wide quantities: a write in the middle of a
     * line changes the border from that pixel onwards. Both registers are
     * therefore tracked while the mask is built, and the values are seeded
     * with the ones in force when the line began.
     */
    auto con0 = initialBplcon0;
    auto con3 = initialBplcon3;
    auto borderColor = this->borderColor(con0, con3);

    // Get the current value of the horizontal DIW flipflop
    auto hf = hflop;

    // Remember where it opens the window, for the benefit of the sprites
    bBufferDiwOpen = hf ? 0 : PIXEL_CNT;

    // Print some debug info if requested
    if CONSTEXPR (LOG_DIW != LogLevel::LV_OFF) {

        logme(LOG_DIW, "updateBorderBuffer (%ld,%ld)\n", hstrt, hstop);
        diwChanges.dump();
    }

    // Determine the initial value of Denise's horizontal counter
    isize counter = HBLANK_MIN * 2;
    
    // OCS Denise does not reset the counter in lines 0 - 8
    if (agnus.pos.v < 9 && isOCS()) counter = (HBLANK_MIN * 2 + agnus.pos.v * 0x1C6) & 0x1FF;

    // Initialize trigger position (position of first register change if any)
    auto trigger = diwChanges.trigger();

    for (isize i = 0; i < LINE_CNT; i++) {

        // Update comparison values if needed
        if (i == trigger) {

            while (i == trigger) {

                RegChange &r = diwChanges.read();
                trigger = diwChanges.trigger();

                switch (r.reg) {

                    case Reg::DIWSTRT:

                        hstrt = r.value;
                        logme(LOG_DIW, "hstrt -> %ld (%lx)\n", hstrt, hstrt);
                        debugger.updateDiwH(hstrt, hstop);
                        break;

                    case Reg::DIWSTOP:

                        hstop = r.value;
                        logme(LOG_DIW, "hstop -> %ld (%lx)\n", hstop, hstop);
                        debugger.updateDiwH(hstrt, hstop);
                        break;

                    case Reg::BPLCON0:

                        con0 = r.value;
                        borderColor = this->borderColor(con0, con3);
                        break;

                    case Reg::BPLCON3:

                        con3 = r.value;
                        borderColor = this->borderColor(con0, con3);
                        break;

                    default:
                        break;
                }
            }
        }

        // Set or clear the horizontal DIW flipflop
        if (counter == hstrt) {

            logme(LOG_DIW, "hflop -> 1 at %ld (%lx)\n", counter, counter);
            hf = true;
            bBufferDiwOpen = std::min(bBufferDiwOpen, Pixel(i));
        }
        if (counter == hstop) {

            logme(LOG_DIW, "hflop -> 0 at %ld (%lx)\n", counter, counter);
            hf = false;
        }

        if (i % 4 == 3) {

            // Advance the horizontal counter
            counter = (counter + 1) & 0x1FF;

            // Wrap over at the end of a line
            if (counter == 0x1C8 && (agnus.pos.v >= 9 || !isOCS())) counter = 2;
        }

        /* Set the border mask. The display window needs both a set flipflop
         * and a BPL1DAT write to open; everything before the first write is
         * border no matter what DIWSTRT says. The write is not undone until
         * the end of the line, so the area between the last bitplane pixel
         * and DIWSTOP stays inside the window and keeps showing COLOR00.
         */
        bBuffer[i] = hf && i >= bplDatBegin ? PixelEngine::BORDER_NONE : borderColor;
    }

    // Check if the hflop has a different value at the end of the line
    if (hflop != hf) {

        // Remember the new value
        hflop = hf;

        // Recalculate the mask in the next line
        markBorderBufferAsDirty(1);
    }

    diwChanges.clear();
}

void 
Denise::markBorderBufferAsDirty(isize lines)
{
    borderBufferIsDirty = std::max(borderBufferIsDirty, lines);
}

template <int x> void
Denise::checkS2SCollisions(Pixel start, Pixel end)
{
    if (start < 0) return;

    // For odd sprites, only proceed if collision detection is enabled
    if constexpr (IS_ODD(x)) if (!GET_BIT(clxcon, 12 + (x/2))) return;

    // Set up the sprite comparison masks
    u16 comp01 = Z_SP0 | (GET_BIT(clxcon, 12) ? Z_SP1 : 0);
    u16 comp23 = Z_SP2 | (GET_BIT(clxcon, 13) ? Z_SP3 : 0);
    u16 comp45 = Z_SP4 | (GET_BIT(clxcon, 14) ? Z_SP5 : 0);
    u16 comp67 = Z_SP6 | (GET_BIT(clxcon, 15) ? Z_SP7 : 0);

    // Iterate over all sprite pixels
    for (Pixel pos = end; pos >= start; pos -= 4) {

        u16 z = zBuffer[pos];
        
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
        
        if CONSTEXPR (LOG_CLX != LogLevel::LV_OFF) {
            
            if ((z & comp45) && (z & comp67)) logme(LOG_CLX, "Coll: 45 and 67\n");
            if ((z & comp23) && (z & comp67)) logme(LOG_CLX, "Coll: 23 and 67\n");
            if ((z & comp23) && (z & comp45)) logme(LOG_CLX, "Coll: 23 and 45\n");
            if ((z & comp01) && (z & comp67)) logme(LOG_CLX, "Coll: 01 and 67\n");
            if ((z & comp01) && (z & comp45)) logme(LOG_CLX, "Coll: 01 and 45\n");
            if ((z & comp01) && (z & comp23)) logme(LOG_CLX, "Coll: 01 and 23\n");
        }
    }
}

template <int x> void
Denise::checkS2PCollisions(Pixel start, Pixel end)
{
    if (start < 0) return;

    // For the odd sprites, only proceed if collision detection is enabled
    if constexpr (IS_ODD(x)) if (!ensp<x>()) return;
    
    u8 enabled1 = enbp1();
    u8 enabled2 = enbp2();
    u8 compare1 = mvbp1() & enabled1;
    u8 compare2 = mvbp2() & enabled2;

    // Check for sprite-playfield collisions
    for (Pixel pos = end; pos >= start; pos -= 4) {

        u16 z = zBuffer[pos];

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // Skip if the pixel lies outside the display window
        if (bBuffer[pos] != PixelEngine::BORDER_NONE) continue;

        // Check for a collision with playfield 2
        if ((dBuffer[pos] & enabled2) == compare2) {
            
            logme(LOG_CLX, "S%d collides with PF2\n", x);
            SET_BIT(clxdat, 5 + (x / 2));

        } else {
            
            /* There is a hardware oddity in single-playfield mode. If PF2
             * doesn't match, PF1 doesn't match either. No matter what.
             * See http://eab.abime.net/showpost.php?p=965074&postcount=2
             */
            if (!(zBuffer[pos] & Z_DPF)) continue;
        }

        // Check for a collision with playfield 1
        if ((dBuffer[pos] & enabled1) == compare1) {
            
            logme(LOG_CLX, "S%d collides with PF1\n", x);
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
    u8 enabled1 = enbp1();
    u8 enabled2 = enbp2();
    u8 compare1 = mvbp1() & enabled1;
    u8 compare2 = mvbp2() & enabled2;

    // Check all pixels one by one
    for (isize pos = 0; pos < PIXEL_CNT; pos++) {

        // Skip pixels outside the display window
        if (bBuffer[pos] != PixelEngine::BORDER_NONE) continue;

        u16 b = dBuffer[pos];

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
Denise::vsyncHandler()
{
    // Run the frame skip logic
    if (frameSkips == 0) {

        pixelEngine.swapBuffers();
        frameSkips = emulator.isWarping() ? config.frameSkipping : 0;

    } else {

        frameSkips--;
    }
    
    hflop = true; // ???
    markBorderBufferAsDirty();
    pixelEngine.vsyncHandler();
    debugger.vsyncHandler();
}

void
Denise::hsyncHandler(isize vpos)
{
    assert(agnus.pos.h == 0x12);
    assert(vpos >= 0 && vpos <= VPOS_MAX);

    //
    // Finish the current line
    //

    // Update border buffer if neccessary
    updateBorderBuffer();

    /* Seed the value the sprites are drawn at. translate() refines it by
     * walking this line's BPLCON0 changes; the VBLANK path below skips
     * translate(), so it keeps the start-of-line value.
     */
    spriteBplcon0 = initialBplcon0;

    // Check if we are below the VBLANK area
    if (!agnus.inVBlankArea(vpos) && !frameSkips) {

        // Translate bitplane data to color register indices
        translate();

        // Draw sprites
        drawSprites();

        // Perform playfield-playfield collision check (if enabled)
        if (config.clxPlfPlf) checkP2PCollisions();

        // Synthesize RGBA values and write the result into the frame buffer
        pixelEngine.colorize(vpos);

        // Remove certain graphics layers if requested
        if (config.hiddenLayers) {
            pixelEngine.hide(vpos, config.hiddenLayers, config.hiddenLayerAlpha);
        }
        
    } else {
        
        drawSprites();
        pixelEngine.replayColRegChanges();
        conChanges.clear();
    }

    assert(conChanges.isEmpty());
    assert(pixelEngine.colChanges.isEmpty());
    assert(sprChanges[0].isEmpty());
    assert(sprChanges[1].isEmpty());
    assert(sprChanges[2].isEmpty());
    assert(sprChanges[3].isEmpty());
    assert(diwChanges.isEmpty());
    
    // Clear the last pixel if this line was a short line
    if (agnus.pos.hLatched == PAL::HPOS_CNT) pixelEngine.getWorkingBuffer().clear(vpos, HPOS_MAX);

    // Clear the dBuffer
    std::memset(dBuffer, 0, sizeof(dBuffer));

    // Remember whether sprites were armed in this line
    wasArmed = armed;

    // Drop bitplane data that has never reached its reload cycle
    latchedOdd = false;
    latchedEven = false;

    // Reset the sprite clipping range
    // spriteClipBegin = PIXEL_CNT;
    spriteClipBegin = borderSprites() ? 0 : PIXEL_CNT;
    spriteClipEnd = PIXEL_CNT + 64;

    // Close the display window again (BPL1DAT reopens it)
    bplDatBegin = PIXEL_CNT;

    // Save the current values of various Denise registers
    initialBplcon0 = bplcon0;
    initialBplcon1 = bplcon1;
    initialBplcon2 = bplcon2;
    initialBplcon3 = bplcon3;
    initialBplcon4 = bplcon4;

    // Hand control over to the debugger
    debugger.hsyncHandler(vpos);
}

void
Denise::eolHandler()
{

}

void
Denise::eofHandler()
{
    // OCS Denise does not reset the hpos counter in the first 9 scanlines.
    // In this area, the border mask has to be rebuild in each line.
    if (isOCS()) markBorderBufferAsDirty(10);

    pixelEngine.eofHandler();
    debugger.eofHandler();
}

template void Denise::translateDPF<true>(Pixel from, Pixel to, PFState &state);
template void Denise::translateDPF<false>(Pixel from, Pixel to, PFState &state);

}
