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
#include "IOUtils.h"

namespace vamiga {

Denise::Denise(Amiga& ref) : SubComponent(ref)
{    
    subComponents = std::vector<CoreComponent *> {
        
        &pixelEngine,
        &screenRecorder
    };
}

void
Denise::_didReset(bool hard)
{
    std::memset(bBuffer, 0xFF, sizeof(bBuffer));
    std::memset(dBuffer, 0, sizeof(dBuffer));
    std::memset(iBuffer, 0, sizeof(iBuffer));
    std::memset(mBuffer, 0, sizeof(mBuffer));
    std::memset(zBuffer, 0, sizeof(zBuffer));
}

i64
Denise::getOption(Option option) const
{
    switch (option) {
            
        case OPT_DENISE_REVISION:           return config.revision;
        case OPT_DENISE_VIEWPORT_TRACKING:  return config.viewportTracking;
        case OPT_DENISE_FRAME_SKIPPING:     return config.frameSkipping;
        case OPT_DENISE_HIDDEN_BITPLANES:   return config.hiddenBitplanes;
        case OPT_DENISE_HIDDEN_SPRITES:     return config.hiddenSprites;
        case OPT_DENISE_HIDDEN_LAYERS:      return config.hiddenLayers;
        case OPT_DENISE_HIDDEN_LAYER_ALPHA: return config.hiddenLayerAlpha;
        case OPT_DENISE_CLX_SPR_SPR:        return config.clxSprSpr;
        case OPT_DENISE_CLX_SPR_PLF:        return config.clxSprPlf;
        case OPT_DENISE_CLX_PLF_PLF:        return config.clxPlfPlf;
            
        default:
            fatalError;
    }
}

void
Denise::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_DENISE_REVISION:

            if (!DeniseRevisionEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, DeniseRevisionEnum::keyList());
            }
            return;

        case OPT_DENISE_VIEWPORT_TRACKING:
        case OPT_DENISE_FRAME_SKIPPING:
        case OPT_DENISE_HIDDEN_BITPLANES:
        case OPT_DENISE_HIDDEN_SPRITES:
        case OPT_DENISE_HIDDEN_LAYERS:
        case OPT_DENISE_HIDDEN_LAYER_ALPHA:
        case OPT_DENISE_CLX_SPR_SPR:
        case OPT_DENISE_CLX_SPR_PLF:
        case OPT_DENISE_CLX_PLF_PLF:

            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
Denise::setOption(Option option, i64 value)
{
    switch (option) {
            
        case OPT_DENISE_REVISION:

            config.revision = (DeniseRevision)value;
            return;

        case OPT_DENISE_VIEWPORT_TRACKING:
            
            config.viewportTracking = (bool)value;
            debugger.resetDIWTracker();
            return;

        case OPT_DENISE_FRAME_SKIPPING:

            config.frameSkipping = (isize)value;
            return;

        case OPT_DENISE_HIDDEN_BITPLANES:
            
            config.hiddenBitplanes = (u8)value;
            return;

        case OPT_DENISE_HIDDEN_SPRITES:
            
            config.hiddenSprites = (u8)value;
            return;
            
        case OPT_DENISE_HIDDEN_LAYERS:
            
            config.hiddenLayers = (u16)value;
            return;
            
        case OPT_DENISE_HIDDEN_LAYER_ALPHA:
            
            config.hiddenLayerAlpha = (u8)value;
            return;

        case OPT_DENISE_CLX_SPR_SPR:
            
            config.clxSprSpr = (bool)value;
            return;
            
        case OPT_DENISE_CLX_SPR_PLF:
            
            config.clxSprPlf = (bool)value;
            return;
            
        case OPT_DENISE_CLX_PLF_PLF:
            
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

    // Convert to a pixel position
    sprhppos[x] = sprhpos[x] + 2 - 4 * HBLANK_MIN;
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
                          (!!(shiftReg[5] & mask) << 5) );
    }
}

void
Denise::extractSlicesOdd(u8 slices[16])
{
    u16 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slices[i] = (u8) ((!!(shiftReg[0] & mask) << 0) |
                          (!!(shiftReg[2] & mask) << 2) |
                          (!!(shiftReg[4] & mask) << 4) );
    }
}

void
Denise::extractSlicesEven(u8 slices[16])
{
    u16 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slices[i] = (u8) ((!!(shiftReg[1] & mask) << 1) |
                          (!!(shiftReg[3] & mask) << 3) |
                          (!!(shiftReg[5] & mask) << 5) );
    }
}

template <Resolution mode> void
Denise::drawOdd(Pixel offset)
{
    static constexpr u16 masks[7] = {
        
        0b000000, // 0 bitplanes
        0b000001, // 1 bitplanes
        0b000001, // 2 bitplanes
        0b000101, // 3 bitplanes
        0b000101, // 4 bitplanes
        0b010101, // 5 bitplanes
        0b010101  // 6 bitplanes
    };
    
    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 2;

    u8 slices[16];
    extractSlicesOdd(slices);
    
    for (isize i = 0; i < 16; i++) {
        
        u8 index = slices[i] & mask;

        switch (mode) {

            case LORES:

                // Synthesize two lores pixels
                assert(pixel + 1 < isizeof(dBuffer));
                dBuffer[pixel] = (dBuffer[pixel] & 0b101010) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b101010) | index;
                pixel++;
                break;

            case HIRES:

                // Synthesize one hires pixel
                assert(pixel < isizeof(dBuffer));
                dBuffer[pixel] = (dBuffer[pixel] & 0b101010) | index;
                pixel++;
                break;

            case SHRES:

                // Synthesize a superHires pixel
                assert(pixel < isizeof(dBuffer));
                if (i % 2 == 0) {
                    dBuffer[pixel] = u8((dBuffer[pixel] & 0b111011) | index << 2);
                } else {
                    dBuffer[pixel] = u8((dBuffer[pixel] & 0b111110) | index);
                    pixel++;
                }
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    shiftReg[0] = shiftReg[2] = shiftReg[4] = 0;
}

template <Resolution mode> void
Denise::drawEven(Pixel offset)
{    
    static constexpr u16 masks[7] = {
        
        0b000000, // 0 bitplanes
        0b000000, // 1 bitplanes
        0b000010, // 2 bitplanes
        0b000010, // 3 bitplanes
        0b001010, // 4 bitplanes
        0b001010, // 5 bitplanes
        0b101010  // 6 bitplanes
    };
    
    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 2;

    u8 slices[16];
    extractSlicesEven(slices);
    
    for (isize i = 0; i < 16; i++) {

        u8 index = slices[i] & mask;

        switch (mode) {

            case LORES:

                // Synthesize s lores pixel
                assert(pixel + 1 < isizeof(dBuffer));
                dBuffer[pixel] = (dBuffer[pixel] & 0b010101) | index;
                pixel++;
                dBuffer[pixel] = (dBuffer[pixel] & 0b010101) | index;
                pixel++;
                break;

            case HIRES:

                // Synthesize a hires pixel
                assert(pixel < isizeof(dBuffer));
                dBuffer[pixel] = (dBuffer[pixel] & 0b010101) | index;
                pixel++;
                break;

            case SHRES:

                // Synthesize a superHires pixel
                assert(pixel < isizeof(dBuffer));
                if (i % 2 == 0) {
                    dBuffer[pixel] = u8((dBuffer[pixel] & 0b110111) | index << 2);
                } else {
                    dBuffer[pixel] = u8((dBuffer[pixel] & 0b111101) | index);
                    pixel++;
                }
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    shiftReg[1] = shiftReg[3] = shiftReg[5] = 0;
}

template <Resolution mode> void
Denise::drawBoth(Pixel offset)
{
    if (BPL_ON_STEROIDS) {

        drawOdd <mode> (offset);
        drawEven <mode> (offset);
        return;
    }

    static constexpr u16 masks[7] = {

        0b000000, // 0 bitplanes
        0b000001, // 1 bitplanes
        0b000011, // 2 bitplanes
        0b000111, // 3 bitplanes
        0b001111, // 4 bitplanes
        0b011111, // 5 bitplanes
        0b111111  // 6 bitplanes
    };

    u16 mask = masks[bpu()];
    Pixel pixel = agnus.pos.pixel() + offset + 2;

    u8 slices[16];
    extractSlices(slices);

    for (isize i = 0; i < 16; i++) {

        u8 index = slices[i] & mask;

        switch (mode) {

            case LORES:

                // Synthesize s lores pixel
                assert(pixel + 1 < isizeof(dBuffer));
                dBuffer[pixel] = index;
                pixel++;
                dBuffer[pixel] = index;
                pixel++;
                break;

            case HIRES:

                // Synthesize a hires pixel
                assert(pixel < isizeof(dBuffer));
                dBuffer[pixel] = index;
                pixel++;
                break;

            case SHRES:

                // Synthesize a superHires pixel
                assert(pixel < isizeof(dBuffer));
                if (i % 2 == 0) {
                    dBuffer[pixel] = u8(index << 2);
                } else {
                    dBuffer[pixel] = u8(dBuffer[pixel] | index);
                    pixel++;
                }
                break;

            default:
                fatalError;
        }
    }

    // Clear the shift registers
    for (isize i = 0; i < 6; i++) shiftReg[i] = 0;
}

void
Denise::drawLoresOdd()
{
    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <LORES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawLoresEven()
{
    if (armedEven) {
        
        updateShiftRegistersEven();
        drawEven <LORES> (pixelOffsetEven);
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
    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <HIRES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawHiresEven()
{
    if (armedEven) {

        updateShiftRegistersEven();
        drawEven <HIRES> (pixelOffsetEven);
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
    if (armedOdd) {

        updateShiftRegistersOdd();
        drawOdd <SHRES> (pixelOffsetOdd);
        armedOdd = false;
    }
}

void
Denise::drawShresEven()
{
    if (armedEven) {

        updateShiftRegistersEven();
        drawEven <SHRES> (pixelOffsetEven);
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

        for (isize i = 0; i < isizeof(dBuffer); i++) {
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

    bool dual = dbplf(initialBplcon0);
    bool hamLine = state.ham;

    // Add a dummy register change to ensure we draw until the line ends
    conChanges.insert(sizeof(dBuffer), RegChange { SET_NONE, 0 });

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
        switch (change.addr) {

            case SET_BPLCON0_DENISE:
                
                dual = dbplf(bplcon0);
                state.ham = ham(change.value);
                hamLine |= state.ham;
                break;

            case SET_BPLCON2:
                
                state.prio = pf2pri(change.value);
                state.zpf1 = zPF1(change.value);
                state.zpf2 = zPF2(change.value);
                break;

            default:
                
                assert(change.addr == SET_NONE);
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
    /* Check for invalid bitplane modes. If the priority of the second bitplane
     * is set to an invalid value (> 4), Denise ignores the data from the first
     * four bitplanes whereever the fifth bitplane is set to 1. Some demos such
     * as "Planet Rocklobster" (Oxyron) show that this kind of bitplane
     * elimination does not happen in HAM mode.
     *
     * Relevant tests in the vAmigaTS test suite:
     * Denise/BPLCON0/invprio0 to Denise/BPLCON0/invprio3
     */
    
    if (!state.zpf2 && !state.ham) {
        
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
        
        assert(PixelEngine::isPaletteIndex(s));
        // The mBuffer alread has the correct value ( mBuffer[i] = s; )
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
    /* If the priority of a playfield is set to an illegal value (zpf1 or
     * zpf2 will be 0 in that case), all pixels are drawn transparent.
     */
    u8 mask1 = state.zpf1 ? 0b1111 : 0b0000;
    u8 mask2 = state.zpf2 ? 0b1111 : 0b0000;

    for (Pixel i = from; i < to; i++) {

        u8 s = dBuffer[i];

        // Determine color indices for both playfields
        u8 index1 = (((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2));
        u8 index2 = (((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3));

        if (index1) {
            
            if (index2) {

                // PF1 is solid, PF2 is solid
                if (prio) {
                    mBuffer[i] = (index2 | 0b1000) & mask2;
                    zBuffer[i] = state.zpf2 | Z_DPF21;
                } else {
                    mBuffer[i] = index1 & mask1;
                    zBuffer[i] = state.zpf1 | Z_DPF12;
                }

            } else {

                // PF1 is solid, PF2 is transparent
                mBuffer[i] = index1 & mask1;
                zBuffer[i] = state.zpf1 | Z_DPF1;
            }

        } else {
            
            if (index2) {

                // PF1 is transparent, PF2 is solid
                mBuffer[i] = (index2 | 0b1000) & mask2;
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
    res == SHRES ? drawSprites<SHRES>() : drawSprites<LORES>();
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
    constexpr Pixel hposMask = R == SHRES ? ~0 : ~1;

    Pixel strt = 0;
    Pixel strt1 = sprhppos[sprite1] & hposMask;
    Pixel strt2 = sprhppos[sprite2] & hposMask;
    
    // Iterate over all recorded register changes
    if (!sprChanges[pair].isEmpty()) {

        for (isize i = 0, end = sprChanges[pair].end(); i < end; i++) {
            
            Pixel trigger = (Pixel)sprChanges[pair].keys[i];
            RegChange &change = sprChanges[pair].elements[i];
            
            // Draw a chunk of pixels
            drawSpritePair <pair,R> (strt, trigger, strt1, strt2);
            strt = trigger;
            
            // Apply the recorded register change
            switch (change.addr) {
                    
                case SET_SPR0DATA + sprite1:
                    
                    sprdata[sprite1] = change.value;
                    SET_BIT(armed, sprite1);
                    break;
                    
                case SET_SPR0DATA + sprite2:
                    
                    sprdata[sprite2] = change.value;
                    SET_BIT(armed, sprite2);
                    break;
                    
                case SET_SPR0DATB + sprite1:
                    
                    sprdatb[sprite1] = change.value;
                    break;
                    
                case SET_SPR0DATB + sprite2:
                    
                    sprdatb[sprite2] = change.value;
                    break;

                case SET_SPR0POS + sprite1:

                    setSPRxPOS(sprite1, change.value);
                    strt1 = sprhppos[sprite1] & hposMask;
                    break;
                    
                case SET_SPR0POS + sprite2:
                    
                    setSPRxPOS(sprite2, change.value);
                    strt2 = sprhppos[sprite2] & hposMask;
                    break;
                    
                case SET_SPR0CTL + sprite1:

                    setSPRxCTL(sprite1, change.value);
                    strt1 = sprhppos[sprite1] & hposMask;
                    CLR_BIT(armed, sprite1);
                    break;
                    
                case SET_SPR0CTL + sprite2:

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
    drawSpritePair <pair,R> (strt, sizeof(mBuffer) - 1, strt1, strt2);
    
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
        switch (change.addr) {
                
            case SET_SPR0DATA + sprite1:
                
                sprdata[sprite1] = change.value;
                break;
                
            case SET_SPR0DATA + sprite2:
                
                sprdata[sprite2] = change.value;
                break;
                
            case SET_SPR0DATB + sprite1:
                
                sprdatb[sprite1] = change.value;
                break;
                
            case SET_SPR0DATB + sprite2:
                
                sprdatb[sprite2] = change.value;
                break;
                
            case SET_SPR0POS + sprite1:

                setSPRxPOS(sprite1, change.value);
                assert(sprpos[sprite1] == change.value);
                break;
                
            case SET_SPR0POS + sprite2:

                setSPRxPOS(sprite2, change.value);
                assert(sprpos[sprite2] == change.value);
                break;
                
            case SET_SPR0CTL + sprite1:

                setSPRxCTL(sprite1, change.value);
                assert(sprctl[sprite1] == change.value);
                break;
                
            case SET_SPR0CTL + sprite2:

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

    assert(hstrt <= isizeof(mBuffer));
    assert(hstop <= isizeof(mBuffer));

    bool armed1 = GET_BIT(armed, sprite1);
    bool armed2 = GET_BIT(armed, sprite2);

    bool attached = GET_BIT(sprctl[sprite2], 7);
    Pixel offset = R == SHRES ? 1 : 2;

    for (Pixel hpos = hstrt; hpos < hstop; hpos += offset) {

        if (hpos == strt1 && armed1) {
            
            ssra[sprite1] = sprdata[sprite1];
            ssrb[sprite1] = sprdatb[sprite1];
        }
        if (hpos == strt2 && armed2) {
            
            ssra[sprite2] = sprdata[sprite2];
            ssrb[sprite2] = sprdatb[sprite2];
        }

        if (ssra[sprite1] | ssrb[sprite1] | ssra[sprite2] | ssrb[sprite2]) {
            
            if (hpos >= spriteClipBegin && hpos < spriteClipEnd) {

                if (attached) {
                    
                    drawAttachedSpritePixelPair <sprite2,R> (hpos);
                    
                } else {
                    
                    drawSpritePixel <sprite1,R> (hpos);
                    drawSpritePixel <sprite2,R> (hpos);
                }
            }
            
            ssra[sprite1] = (u16)(ssra[sprite1] << 1);
            ssrb[sprite1] = (u16)(ssrb[sprite1] << 1);
            ssra[sprite2] = (u16)(ssra[sprite2] << 1);
            ssrb[sprite2] = (u16)(ssrb[sprite2] << 1);
        }
    }

    // Perform collision checks (if enabled)
    if (config.clxSprSpr) {
        
        checkS2SCollisions<2 * pair>(strt1, strt1 + 31);
        checkS2SCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
    if (config.clxSprPlf) {
        
        checkS2PCollisions<2 * pair>(strt1, strt1 + 31);
        checkS2PCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
}

template <isize x, Resolution R> void
Denise::drawSpritePixel(Pixel hpos)
{
    assert(hpos >= spriteClipBegin && hpos < spriteClipEnd);

    u8 a = (ssra[x] >> 15);
    u8 b = (ssrb[x] >> 14) & 2;
    u8 col = a | b;

    if (col) {

        u16 z = Z_SP[x];
        u8 base = 16 + 2 * (x & 6);

        if constexpr (R == SHRES) {

            if (z > zBuffer[hpos]) mBuffer[hpos] = base | col;
            zBuffer[hpos] |= z;

        } else {

            if (z > zBuffer[hpos]) mBuffer[hpos] = base | col;
            if (z > zBuffer[hpos + 1]) mBuffer[hpos + 1] = base | col;
            zBuffer[hpos] |= z;
            zBuffer[hpos + 1] |= z;
        }
    }
}

template <isize x, Resolution R> void
Denise::drawAttachedSpritePixelPair(Pixel hpos)
{
    assert(IS_ODD(x));
    assert(hpos >= spriteClipBegin && hpos < spriteClipEnd);

    u8 col =
    ((ssra[x-1] >> 15) & 0b0001) |
    ((ssrb[x-1] >> 14) & 0b0010) |
    ((ssra[x]   >> 13) & 0b0100) |
    ((ssrb[x]   >> 12) & 0b1000) ;
    
    if (col) {

        u16 z = Z_SP[x];

        if (z > zBuffer[hpos]) {
            
            mBuffer[hpos] = 0b10000 | col;
            zBuffer[hpos] |= z;
        }
        if (z > zBuffer[hpos+1]) {
            
            mBuffer[hpos+1] = 0b10000 | col;
            zBuffer[hpos+1] |= z;
        }
    }
}

void
Denise::updateBorderColor()
{
    if (config.revision != DENISE_OCS && ecsena() && brdrblnk()) {
        borderColor = 64; // Pure black
    } else {
        borderColor = 0;  // Background color
    }
    if (BORDER_DEBUG) {
        borderColor = 65; // Debug color
    }
}

void
Denise::updateBorderBuffer()
{
    // Only proceed if the buffer is dirty
    if (!borderBufferIsDirty) return;
    denise.borderBufferIsDirty--;

    // Get the current value of the horizontal DIW flipflop
    auto hf = hflop;

    // Print some debug info if requested
    if (DIW_DEBUG) {

        trace(true, "updateBorderBuffer (%ld,%ld)\n", hstrt, hstop);
        diwChanges.dump();
    }

    // Determine the initial value of Denise's horizontal counter
    isize counter = HBLANK_MIN * 2;
    
    // OCS Denise does not reset the counter in lines 0 - 8
    if (agnus.pos.v < 9 && isOCS()) counter = (HBLANK_MIN * 2 + agnus.pos.v * 0x1C6) & 0x1FF;

    // Initialize trigger position (position of first register change if any)
    auto trigger = diwChanges.trigger();

    for (isize i = 0; i < isizeof(bBuffer); i++) {

        // Update comparison values if needed
        if (i == trigger) {

            while (i == trigger) {

                RegChange &r = diwChanges.read();
                trigger = diwChanges.trigger();

                switch (r.addr) {

                    case REG_DIWSTRT:

                        hstrt = r.value;
                        trace(DIW_DEBUG, "hstrt -> %ld (%lx)\n", hstrt, hstrt);
                        break;

                    case REG_DIWSTOP:

                        hstop = r.value;
                        trace(DIW_DEBUG, "hstop -> %ld (%lx)\n", hstop, hstop);
                        break;

                    default:
                        break;
                }

                // Inform the debugger about the changed display window
                debugger.updateDiwH(hstrt, hstop);
            }
        }

        // Set or clear the horizontal DIW flipflop
        if (counter == hstrt) {

            trace(DIW_DEBUG, "hflop -> 1 at %ld (%lx)\n", counter, counter);
            hf = true;
        }
        if (counter == hstop) {

            trace(DIW_DEBUG, "hflop -> 0 at %ld (%lx)\n", counter, counter);
            hf = false;
        }

        if (i % 2 == 1) {

            // Advance the horizontal counter
            counter = (counter + 1) & 0x1FF;

            // Wrap over at the end of a line
            if (counter == 0x1C8 && (agnus.pos.v >= 9 || isECS())) counter = 2;
        }

        // Set the border mask (0xFF = no border)
        bBuffer[i] = hf ? 0xFF : borderColor;
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
    // For odd sprites, only proceed if collision detection is enabled
    if constexpr (IS_ODD(x)) if (!GET_BIT(clxcon, 12 + (x/2))) return;

    // Set up the sprite comparison masks
    u16 comp01 = Z_SP0 | (GET_BIT(clxcon, 12) ? Z_SP1 : 0);
    u16 comp23 = Z_SP2 | (GET_BIT(clxcon, 13) ? Z_SP3 : 0);
    u16 comp45 = Z_SP4 | (GET_BIT(clxcon, 14) ? Z_SP5 : 0);
    u16 comp67 = Z_SP6 | (GET_BIT(clxcon, 15) ? Z_SP7 : 0);

    // Iterate over all sprite pixels
    for (Pixel pos = end; pos >= start; pos -= 2) {

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
        
        if (CLX_DEBUG) {
            
            if ((z & comp45) && (z & comp67)) trace(true, "Coll: 45 and 67\n");
            if ((z & comp23) && (z & comp67)) trace(true, "Coll: 23 and 67\n");
            if ((z & comp23) && (z & comp45)) trace(true, "Coll: 23 and 45\n");
            if ((z & comp01) && (z & comp67)) trace(true, "Coll: 01 and 67\n");
            if ((z & comp01) && (z & comp45)) trace(true, "Coll: 01 and 45\n");
            if ((z & comp01) && (z & comp23)) trace(true, "Coll: 01 and 23\n");
        }
    }
}

template <int x> void
Denise::checkS2PCollisions(Pixel start, Pixel end)
{
    // For the odd sprites, only proceed if collision detection is enabled
    if constexpr (IS_ODD(x)) if (!ensp<x>()) return;
    
    u8 enabled1 = enbp1();
    u8 enabled2 = enbp2();
    u8 compare1 = mvbp1() & enabled1;
    u8 compare2 = mvbp2() & enabled2;

    // Check for sprite-playfield collisions
    for (Pixel pos = end; pos >= start; pos -= 2) {

        u16 z = zBuffer[pos];

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // Check for a collision with playfield 2
        if ((dBuffer[pos] & enabled2) == compare2) {
            
            trace(CLX_DEBUG, "S%d collides with PF2\n", x);
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
            
            trace(CLX_DEBUG, "S%d collides with PF1\n", x);
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
    for (isize pos = 0; pos < HPIXELS; pos++) {

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
    if (agnus.pos.hLatched == HPOS_CNT_PAL) pixelEngine.getWorkingBuffer().clear(vpos, HPOS_MAX);

    // Clear the dBuffer
    std::memset(dBuffer, 0, sizeof(dBuffer));

    // Remember whether sprites were armed in this line
    wasArmed = armed;

    // Reset the sprite clipping range
    spriteClipBegin = HPIXELS;
    spriteClipEnd = HPIXELS + 32;

    // Save the current values of various Denise registers
    initialBplcon0 = bplcon0;
    initialBplcon1 = bplcon1;
    initialBplcon2 = bplcon2;

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

    // Run the frame skip logic
    if (frameSkips == 0) {

        pixelEngine.swapBuffers();
        frameSkips = emulator.isWarping() ? config.frameSkipping : 0;

    } else {

        frameSkips--;
    }
}

template void Denise::drawOdd<false>(Pixel offset);
template void Denise::drawOdd<true>(Pixel offset);
template void Denise::drawEven<false>(Pixel offset);
template void Denise::drawEven<true>(Pixel offset);

template void Denise::translateDPF<true>(Pixel from, Pixel to, PFState &state);
template void Denise::translateDPF<false>(Pixel from, Pixel to, PFState &state);

}
