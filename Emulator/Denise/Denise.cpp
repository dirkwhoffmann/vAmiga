// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "Amiga.h"
#include "ControlPort.h"
#include "IOUtils.h"
#include "SSEUtils.h"

Denise::Denise(Amiga& ref) : SubComponent(ref)
{    
    subComponents = std::vector<AmigaComponent *> {
        
        &pixelEngine
    };
    
#ifdef SCREEN_RECORDER
    subComponents.push_back(&screenRecorder);
#endif
}

void
Denise::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    std::memset(bBuffer, 0, sizeof(bBuffer));
    std::memset(iBuffer, 0, sizeof(iBuffer));
    std::memset(mBuffer, 0, sizeof(mBuffer));
    std::memset(zBuffer, 0, sizeof(zBuffer));
}

DeniseConfig
Denise::getDefaultConfig()
{
    DeniseConfig defaults;

    defaults.revision = DENISE_OCS;
    defaults.hiddenBitplanes = 0;
    defaults.hiddenSprites = 0;
    defaults.hiddenLayers = 0;
    defaults.hiddenLayerAlpha = 128;
    defaults.clxSprSpr = true;
    defaults.clxSprPlf = true;
    defaults.clxPlfPlf = true;

    return defaults;
}

void
Denise::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    setConfigItem(OPT_DENISE_REVISION, defaults.revision);
    setConfigItem(OPT_HIDDEN_BITPLANES, defaults.hiddenBitplanes);
    setConfigItem(OPT_HIDDEN_SPRITES, defaults.hiddenSprites);
    setConfigItem(OPT_HIDDEN_LAYERS, defaults.hiddenLayers);
    setConfigItem(OPT_HIDDEN_LAYER_ALPHA, defaults.hiddenLayerAlpha);
    setConfigItem(OPT_CLX_SPR_SPR, defaults.clxSprSpr);
    setConfigItem(OPT_CLX_SPR_PLF, defaults.clxSprPlf);
    setConfigItem(OPT_CLX_PLF_PLF, defaults.clxPlfPlf);
}

i64
Denise::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DENISE_REVISION:     return config.revision;
        case OPT_HIDDEN_BITPLANES:    return config.hiddenBitplanes;
        case OPT_HIDDEN_SPRITES:      return config.hiddenSprites;
        case OPT_HIDDEN_LAYERS:       return config.hiddenLayers;
        case OPT_HIDDEN_LAYER_ALPHA:  return config.hiddenLayerAlpha;
        case OPT_CLX_SPR_SPR:         return config.clxSprSpr;
        case OPT_CLX_SPR_PLF:         return config.clxSprPlf;
        case OPT_CLX_PLF_PLF:         return config.clxPlfPlf;
            
        default:
            fatalError;
    }
}

void
Denise::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_DENISE_REVISION:
            
            if (!DeniseRevisionEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, DeniseRevisionEnum::keyList());
            }
            
            config.revision = (DeniseRevision)value;
            return;
                        
        case OPT_HIDDEN_BITPLANES:
            
            config.hiddenBitplanes = (u8)value;
            return;

        case OPT_HIDDEN_SPRITES:
            
            config.hiddenSprites = (u8)value;
            return;
            
        case OPT_HIDDEN_LAYERS:
            
            config.hiddenLayers = (u16)value;
            return;
            
        case OPT_HIDDEN_LAYER_ALPHA:
            
            config.hiddenLayerAlpha = (u8)value;
            return;

        case OPT_CLX_SPR_SPR:
            
            config.clxSprSpr = (bool)value;
            return;
            
        case OPT_CLX_SPR_PLF:
            
            config.clxSprPlf = (bool)value;
            return;
            
        case OPT_CLX_PLF_PLF:
            
            config.clxPlfPlf = (bool)value;
            return;

        default:
            fatalError;
    }
}

void
Denise::_inspect() const
{
    synchronized {
        
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
        
        info.joydat[0] = controlPort1.joydat();
        info.joydat[1] = controlPort2.joydat();
        info.clxdat = 0;
        
        for (isize i = 0; i < 6; i++) {
            info.bpldat[i] = bpldat[i];
        }
        for (isize i = 0; i < 32; i++) {
            info.colorReg[i] = pixelEngine.getColor(i);
            info.color[i] = pixelEngine.getRGBA(i);
        }
    }
}

void
Denise::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Chip revision");
        os << DeniseRevisionEnum::key(config.revision) << std::endl;
        os << tab("Hidden bitplanes");
        os << hex(config.hiddenBitplanes) << std::endl;
        os << tab("Hidden sprites");
        os << hex(config.hiddenSprites) << std::endl;
        os << tab("Hidden layers");
        os << hex(config.hiddenLayers) << std::endl;
        os << tab("Hidden layer alpha");
        os << dec(config.hiddenLayerAlpha) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
    }
    
    if (category & dump::Registers) {
        
        os << tab("BPLCON0");
        os << hex(bplcon0) << std::endl;
        os << tab("BPLCON1");
        os << hex(bplcon1) << std::endl;
        os << tab("BPLCON2");
        os << hex(bplcon2) << std::endl;
        os << tab("BPLCON3");
        os << hex(bplcon3) << std::endl;
    
        os << tab("SPRxDATA");
        for (isize i = 0; i < 8; i++) os << hex(sprdata[i]) << ' ';
        os << std::endl;
        os << tab("SPRxDATB");
        for (isize i = 0; i < 8; i++) os << hex(sprdatb[i]) << ' ';
        os << std::endl;
        os << tab("SPRxPOS");
        for (isize i = 0; i < 8; i++) os << hex(sprpos[i]) << ' ';
        os << std::endl;
        os << tab("SPRxCTL");
        for (isize i = 0; i < 8; i++) os << hex(sprctl[i]) << ' ';
        os << std::endl;
    }
}

bool
Denise::spritePixelIsVisible(Pixel hpos) const
{
    u16 z = zBuffer[hpos];
    return (z & Z_SP01234567) > (z & ~Z_SP01234567);
}

void
Denise::updateShiftRegisters()
{
    // Only proceed if the load cycle has been reached
    if (agnus.pos.h < fillPos) return;
    
    fillPos = INT16_MAX;
    armedOdd = true;
    armedEven = true;
    
    switch (bpu()) {
            
        case 6: shiftReg[5] = bpldatPipe[5];
        case 5: shiftReg[4] = bpldatPipe[4];
        case 4: shiftReg[3] = bpldatPipe[3];
        case 3: shiftReg[2] = bpldatPipe[2];
        case 2: shiftReg[1] = bpldatPipe[1];
        case 1: shiftReg[0] = bpldatPipe[0];
    }
    
    // On Intel machines, call the optimized SSE code
    #if (defined(__i386__) || defined(__x86_64__)) && defined(__MACH__)
    
    if (!NO_SSE) {
        util::transposeSSE(shiftReg, slice);
        return;
    }
    
    #endif
    
    // On all other machines, fallback to the slower standard implementation
    u32 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slice[i] = (u8) ((!!(shiftReg[0] & mask) << 0) |
                         (!!(shiftReg[1] & mask) << 1) |
                         (!!(shiftReg[2] & mask) << 2) |
                         (!!(shiftReg[3] & mask) << 3) |
                         (!!(shiftReg[4] & mask) << 4) |
                         (!!(shiftReg[5] & mask) << 5) );
    }
}

template <bool hiresMode> void
Denise::drawOdd(Pixel offset)
{
    assert(!hiresMode || (agnus.pos.h & 0x3) == agnus.scrollHiresOdd);
    assert( hiresMode || (agnus.pos.h & 0x7) == agnus.scrollLoresOdd);

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
    Pixel currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {
        
        u8 index = slice[i] & mask;
        
        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
            
        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
        }
    }
 
    // Disarm and clear the shift registers
    armedOdd = false;
    shiftReg[0] = shiftReg[2] = shiftReg[4] = 0;
}

template <bool hiresMode> void
Denise::drawEven(Pixel offset)
{
    assert(!hiresMode || (agnus.pos.h & 0x3) == agnus.scrollHiresEven);
    assert( hiresMode || (agnus.pos.h & 0x7) == agnus.scrollLoresEven);
    
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
    Pixel currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {

        u8 index = slice[i] & mask;

        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;

        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;
        }
    }
 
    // Disarm and clear the shift registers
    armedEven = false;
    shiftReg[1] = shiftReg[3] = shiftReg[5] = 0;
}

template <bool hiresMode> void
Denise::drawBoth(Pixel offset)
{
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
    Pixel currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {
        
        u8 index = slice[i] & mask;
        
        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel++] = index;
            
        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel++] = index;
            bBuffer[currentPixel++] = index;
        }
    }
    
    // Disarm and clear the shift registers
    armedEven = armedOdd = false;
    for (isize i = 0; i < 6; i++) shiftReg[i] = 0;
}

void
Denise::drawHiresOdd()
{
    updateShiftRegisters();
    if (armedOdd) drawOdd <true> (pixelOffsetOdd);
}

void
Denise::drawHiresEven()
{
    updateShiftRegisters();
    if (armedEven) drawEven <true> (pixelOffsetEven);
}

void
Denise::drawHiresBoth()
{
    updateShiftRegisters();

    if (armedOdd && armedEven && pixelOffsetOdd == pixelOffsetEven) {

        assert((agnus.pos.h & 0x3) == agnus.scrollHiresOdd);
        assert((agnus.pos.h & 0x3) == agnus.scrollHiresEven);
        
        drawBoth <true> (pixelOffsetOdd);

    } else {
    
        if (armedOdd) drawOdd <true> (pixelOffsetOdd);
        if (armedEven) drawEven <true> (pixelOffsetEven);
    }
}

void
Denise::drawLoresOdd()
{
    updateShiftRegisters();
    if (armedOdd) drawOdd <false> (pixelOffsetOdd);
}

void
Denise::drawLoresEven()
{
    updateShiftRegisters();
    if (armedEven) drawEven <false> (pixelOffsetEven);
}

void
Denise::drawLoresBoth()
{
    updateShiftRegisters();

    if (armedOdd && armedEven && pixelOffsetOdd == pixelOffsetEven) {

        assert((agnus.pos.h & 0x7) == agnus.scrollLoresOdd);
        assert((agnus.pos.h & 0x7) == agnus.scrollLoresEven);
        
        drawBoth <false> (pixelOffsetOdd);

    } else {
    
        if (armedOdd) drawOdd <false> (pixelOffsetOdd);
        if (armedEven) drawEven <false> (pixelOffsetEven);
    }
}

void
Denise::translate()
{
    Pixel pixel = 0;

    // Wipe out some bitplane data if requested
    if (config.hiddenBitplanes) {
    
        for (isize i = 0; i < isizeof(bBuffer); i++) {
            bBuffer[i] &= ~config.hiddenBitplanes;
        }
    }
    
    // Start with the playfield state as it was at the beginning of the line
    PFState state;
    state.zpf1 = zPF1(initialBplcon2);
    state.zpf2 = zPF2(initialBplcon2);
    state.prio = pf2pri(initialBplcon2);
    state.ham = ham(initialBplcon0);
    bool dual = dbplf(initialBplcon0);

    // Add a dummy register change to ensure we draw until the line ends
    conChanges.insert(sizeof(bBuffer), RegChange { SET_NONE, 0 });

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

             u8 s = bBuffer[i];

             assert(PixelEngine::isRgbaIndex(s));
             iBuffer[i] = mBuffer[i] = (s & 0x10) ? (s & 0x30) : s;
             zBuffer[i] = 0;
         }
        return;
    }
    
    // Translate the usual way
    for (Pixel i = from; i < to; i++) {
        
        u8 s = bBuffer[i];
        
        assert(PixelEngine::isRgbaIndex(s));
        iBuffer[i] = mBuffer[i] = s;
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

        u8 s = bBuffer[i];

        // Determine color indices for both playfields
        u8 index1 = (((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2));
        u8 index2 = (((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3));

        if (index1) {
            
            if (index2) {

                // PF1 is solid, PF2 is solid
                if (prio) {
                    iBuffer[i] = mBuffer[i] = (index2 | 0b1000) & mask2;
                    zBuffer[i] = state.zpf2 | Z_DPF21;
                } else {
                    iBuffer[i] = mBuffer[i] = index1 & mask1;
                    zBuffer[i] = state.zpf1 | Z_DPF12;
                }

            } else {

                // PF1 is solid, PF2 is transparent
                iBuffer[i] = mBuffer[i] = index1 & mask1;
                zBuffer[i] = state.zpf1 | Z_DPF1;
            }

        } else {
            
            if (index2) {

                // PF1 is transparent, PF2 is solid
                iBuffer[i] = mBuffer[i] = (index2 | 0b1000) & mask2;
                zBuffer[i] = state.zpf2 | Z_DPF2;

            } else {

                // PF1 is transparent, PF2 is transparent
                iBuffer[i] = mBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }
        }
    }
}

void
Denise::drawSprites()
{
    if (wasArmed) {
        
        if (wasArmed & 0b11000000) drawSpritePair <3> ();
        if (wasArmed & 0b00110000) drawSpritePair <2> ();
        if (wasArmed & 0b00001100) drawSpritePair <1> ();
        if (wasArmed & 0b00000011) drawSpritePair <0> ();
        
        // Record sprite data in debug mode
        if (amiga.inDebugMode()) debugger.recordSprites(wasArmed);
    }
    
    /* If a sprite was armed, the code above has been executed which means
     * that all recorded register changes have been applied and the relevant
     * sprite registers are all up to date at this time. For unarmed sprites,
     * however, the register change buffers may contain unprocessed entried.
     * We replay those to get the sprite registers up to date.
     */
    if (!sprChanges[3].isEmpty()) replaySpriteRegChanges <3> ();
    if (!sprChanges[2].isEmpty()) replaySpriteRegChanges <2> ();
    if (!sprChanges[1].isEmpty()) replaySpriteRegChanges <1> ();
    if (!sprChanges[0].isEmpty()) replaySpriteRegChanges <0> ();
}

template <isize pair> void
Denise::drawSpritePair()
{
    constexpr isize sprite1 = 2 * pair;
    constexpr isize sprite2 = 2 * pair + 1;

    Pixel strt = 0;
    Pixel strt1 = sprhppos<sprite1>();
    Pixel strt2 = sprhppos<sprite2>();
    
    // Iterate over all recorded register changes
    if (!sprChanges[pair].isEmpty()) {
                
        for (isize i = 0, end = sprChanges[pair].end(); i < end; i++) {
            
            Pixel trigger = (Pixel)sprChanges[pair].keys[i];
            RegChange &change = sprChanges[pair].elements[i];
            
            // Draw a chunk of pixels
            drawSpritePair <pair> (strt, trigger, strt1, strt2);
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
                    
                    sprpos[sprite1] = change.value;
                    strt1 = sprhppos<sprite1>();
                    break;
                    
                case SET_SPR0POS + sprite2:
                    
                    sprpos[sprite2] = change.value;
                    strt2 = sprhppos<sprite2>();
                    break;
                    
                case SET_SPR0CTL + sprite1:
                    
                    sprctl[sprite1] = change.value;
                    strt1 = sprhppos<sprite1>();
                    CLR_BIT(armed, sprite1);
                    break;
                    
                case SET_SPR0CTL + sprite2:
                    
                    sprctl[sprite2] = change.value;
                    strt2 = sprhppos<sprite2>();
                    CLR_BIT(armed, sprite2);
                    break;

                default:
                    fatalError;
            }
        }
    }
    
    // Draw until the end of the line
    drawSpritePair <pair> (strt, sizeof(mBuffer) - 1, strt1, strt2);
    
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
                
                sprpos[sprite1] = change.value;
                break;
                
            case SET_SPR0POS + sprite2:
                
                sprpos[sprite2] = change.value;
                break;
                
            case SET_SPR0CTL + sprite1:
                
                sprctl[sprite1] = change.value;
                break;
                
            case SET_SPR0CTL + sprite2:
                
                sprctl[sprite2] = change.value;
                break;
                
            default:
                fatalError;
        }
    }
    
    sprChanges[pair].clear();
}

template <isize pair> void
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

    for (Pixel hpos = hstrt; hpos < hstop; hpos += 2) {

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
                    
                    drawAttachedSpritePixelPair<sprite2>(hpos);
                    
                } else {
                    
                    drawSpritePixel <sprite1> (hpos);
                    drawSpritePixel <sprite2> (hpos);
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

template <isize x> void
Denise::drawSpritePixel(Pixel hpos)
{
    assert(hpos >= spriteClipBegin && hpos < spriteClipEnd);

    u8 a = (ssra[x] >> 15);
    u8 b = (ssrb[x] >> 14) & 2;
    u8 col = a | b;

    if (col) {

        u16 z = Z_SP[x];
        u8 base = 16 + 2 * (x & 6);

        if (z > zBuffer[hpos]) mBuffer[hpos] = base | col;
        if (z > zBuffer[hpos + 1]) mBuffer[hpos + 1] = base | col;
        zBuffer[hpos] |= z;
        zBuffer[hpos + 1] |= z;
    }
}

template <isize x> void
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
    if constexpr (BORDER_DEBUG) {
        borderColor = 65; // Debug color
    }
}

void
Denise::drawBorder()
{
    // Check if the horizontal flipflop was set somewhere in this rasterline
    bool hFlopWasSet = agnus.diwHFlop || agnus.diwHFlopOn != -1;

    // Check if the whole line is blank (drawn in background color)
    bool lineIsBlank = !agnus.diwVFlop || !hFlopWasSet;

    if (lineIsBlank) {

        // Draw blank line
        for (Pixel i = 0; i < HPIXELS; i++) {
            bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
        }

    } else {

        // Draw left border
        if (!agnus.diwHFlop && agnus.diwHFlopOn != -1) {
            for (isize i = 0; i < 2 * agnus.diwHFlopOn; i++) {
                bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
            }
        }

        // Draw right border
        if (agnus.diwHFlopOff != -1) {
            for (isize i = 2 * agnus.diwHFlopOff; i < HPIXELS; i++) {
                bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
            }
        }
    }

#ifdef LINE_DEBUG
    if (LINE_DEBUG) {
        for (Pixel i = 0; i < HPIXELS / 2; i++) {
            iBuffer[i] = mBuffer[i] = 64;
        }
    }
#endif
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
        
        if constexpr (CLX_DEBUG) {
            
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
        if ((bBuffer[pos] & enabled2) == compare2) {
            
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
        if ((bBuffer[pos] & enabled1) == compare1) {
            
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

        u16 b = bBuffer[pos];

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
    pixelEngine.vsyncHandler();
    debugger.vsyncHandler();
}

void
Denise::beginOfLine(isize vpos)
{    
    // Save the current values of various Denise registers
    initialBplcon0 = bplcon0;
    initialBplcon1 = bplcon1;
    initialBplcon2 = bplcon2;
    wasArmed = armed;

    // Prepare the bitplane shift registers
    for (isize i = 0; i < 6; i++) shiftReg[i] = 0;

    // Clear the bBuffer
    std::memset(bBuffer, 0, sizeof(bBuffer));

    // Reset the sprite clipping range
    spriteClipBegin = HPIXELS;
    spriteClipEnd = HPIXELS;
}

void
Denise::endOfLine(isize vpos)
{
    // Check if we are below the VBLANK area
    if (vpos >= 26) {

        // Translate bitplane data to color register indices
        translate();

        // Draw sprites
        drawSprites();

        // Perform playfield-playfield collision check (if enabled)
        if (config.clxPlfPlf) checkP2PCollisions();

        // Draw border pixels
        drawBorder();

        // Synthesize RGBA values and write the result into the frame buffer
        pixelEngine.colorize(vpos);

        // Remove certain graphics layers if requested
        if (config.hiddenLayers) {
            pixelEngine.hide(vpos, config.hiddenLayers, config.hiddenLayerAlpha);
        }
        
    } else {
        
        drawSprites();
        pixelEngine.endOfVBlankLine();
        conChanges.clear();
    }

    assert(conChanges.isEmpty());
    assert(pixelEngine.colChanges.isEmpty());
    assert(sprChanges[0].isEmpty());
    assert(sprChanges[1].isEmpty());
    assert(sprChanges[2].isEmpty());
    assert(sprChanges[3].isEmpty());

    // Invoke the DMA debugger
    dmaDebugger.computeOverlay();
    
    // Encode a HIRES / LORES marker in the first HBLANK pixel
    *denise.pixelEngine.pixelAddr(HBLANK_MIN * 4) = hires() ? 0 : -1;
}

template void Denise::drawOdd<false>(Pixel offset);
template void Denise::drawOdd<true>(Pixel offset);
template void Denise::drawEven<false>(Pixel offset);
template void Denise::drawEven<true>(Pixel offset);

template void Denise::translateDPF<true>(Pixel from, Pixel to, PFState &state);
template void Denise::translateDPF<false>(Pixel from, Pixel to, PFState &state);
