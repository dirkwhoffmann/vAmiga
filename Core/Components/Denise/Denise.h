// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DeniseTypes.h"
#include "SubComponent.h"
#include "Colors.h"
#include "DeniseDebugger.h"
#include "Memory.h"
#include "PixelEngine.h"
#include "Recorder.h"

namespace vamiga {

class Denise final : public SubComponent, public Inspectable<DeniseInfo> {

    friend class DeniseDebugger;

    Descriptions descriptions = {{

        .type           = Class::Denise,
        .name           = "Denise",
        .description    = "Graphics",
        .shell          = "denise"
    }};

    Options options = {

        Opt::DENISE_REVISION,
        Opt::DENISE_VIEWPORT_TRACKING,
        Opt::DENISE_FRAME_SKIPPING,
        Opt::DENISE_HIDDEN_BITPLANES,
        Opt::DENISE_HIDDEN_SPRITES,
        Opt::DENISE_HIDDEN_LAYERS,
        Opt::DENISE_HIDDEN_LAYER_ALPHA,
        Opt::DENISE_CLX_SPR_SPR,
        Opt::DENISE_CLX_SPR_PLF,
        Opt::DENISE_CLX_PLF_PLF
    };

    // Current configuration
    DeniseConfig config = {};
    
    
    //
    // Subcomponents
    //
    
public:
    
    // Color synthesizer for computing RGBA values
    PixelEngine pixelEngine = PixelEngine(amiga);

    // Sprite tracker
    DeniseDebugger debugger = DeniseDebugger(amiga);
    
    // A screen recorder for creating video streams
    // Recorder screenRecorder = Recorder(amiga);

    
    //
    // Counters
    //
    
    // Denise has been executed up to this clock cycle
    Cycle clock = 0;

    // Frame skip counter (activated in warp mode)
    isize frameSkips = 0;

    //
    // Registers
    //
    
    // Register values as written by poke[DIWSTRT/STOP/HIGH]
    u16 diwstrt;
    u16 diwstop;
    u16 diwhigh;

    // Display window coordinates (extracted from DIWSTRT, DIWSTOP, and DIWHIGH)
    isize hstrt;
    isize hstop;
    
    /* Denise contains a flipflop controlling the horizontal display window.
     * It is cleared inside the border area and set inside the display area:
     *
     *   - When hpos matches the position in DIWSTRT, the flipflop is set.
     *   - When hpos matches the position in DIWSTOP, the flipflop is reset.
     *
     * Because Denise counts ... -> $1C6 -> $1C7 -> $002 -> $003 -> ...
     *
     *   - The smallest recognised value for DIWSTRT is $002.
     *   - The largest recognised value for DIWSTOP is $1C7.
     */
    bool hflop;

    // Indicates whether the border mask needs an update
    isize borderBufferIsDirty;

    // Bitplane control registers
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    u16 bplcon3;

    // Bitplane control registers at cycle 0 in the current rasterline
    u16 initialBplcon0;
    u16 initialBplcon1;
    u16 initialBplcon2;

    // Bitplane resolution (derived from bplcon0)
    Resolution res;

    // Extracted from BPLCON1 to emulate horizontal scrolling
    Pixel pixelOffsetOdd;
    Pixel pixelOffsetEven;

    // Color register index for the border color (0 = background color)
    u8 borderColor;
    
    // Bitplane data registers
    u16 bpldat[6];
    
    // Pipeline registers
    u16 bpldatPipe[6];

    // Sprite collision registers
    u16 clxdat;
    u16 clxcon;

    //
    // Shift registers
    //
    
    /* Parallel-to-serial shift registers. Denise transfers the current values
     * of the BPLDAT registers into these shift registers after BPLDAT1 is
     * written to. This is emulated in function fillShiftRegister().
     */
    u16 shiftReg[6];

    // Flags indicating that the shift registers have been loaded
    bool armedOdd;
    bool armedEven;

    
    //
    // Register change management
    //

public:

    // Ringbuffer recording control register changes
    RegChangeRecorder<128> conChanges;

    // Ringbuffers recording sprite register changes (one for each sprite pair)
    RegChangeRecorder<128> sprChanges[4];

    // Ringbuffer recording DIW register changes
    RegChangeRecorder<128> diwChanges;


    //
    // Sprites
    //

    // Sprite data registers (SPRxDATA, SPRxDATAB)
    u16 sprdata[8];
    u16 sprdatb[8];

    // The position and control registers of all 8 sprites
    u16 sprpos[8];
    u16 sprctl[8];

    // Horizontal (pixel) coordinates of all 8 sprites
    i16 sprhpos[8];
    i16 sprhppos[8];

    // The serial shift registers of all 8 sprites
    u16 ssra[8];
    u16 ssrb[8];
    
    /* Indicates which sprites are currently armed. An armed sprite is a sprite
     * that will be drawn in this line.
     */
    u8 armed;

    /* Remembers the sprites that were armed in the current rasterline. Note
     * that a sprite can be armed and disarmed multiple times in a rasterline
     * by manually modifying SPRxDATA and SPRxCTL, respectively.
     */
    u8 wasArmed;

    /* Sprite clipping window
     *
     * The clipping window determines where sprite pixels can be drawn.

     *  spriteClipBegin : The first possible sprite pixel in this rasterline
     *    spriteClipEnd : The last possible sprite pixel in this rasterline + 1
     *
     * The variables are set in the hsyncHandler to their expected values.
     * In general, sprites can be drawn if we are in a bitplane DMA line as
     * testes by function inBplDmaLine(). If BPLCON0 changes in the middle
     * of rasterline, the sprite clipping window is adjusted, too. The
     * following conditions are likely to apply on a real Amiga:
     *
     * 1. Enabling sprites is always possible, even at high DMA cycle numbers.
     * 2. Disbabling sprites only has an effect until the DDFSTRT position
     *    has been reached. If sprite drawing was enabled at that position,
     *    it can't be disabled in the same rasterline any more.
     */
    Pixel spriteClipBegin;
    Pixel spriteClipEnd;


    //
    // Rasterline data
    //

    /* Multiple buffers are involved in the generation of pixel data:
     *
     * dBuffer: Data buffer
     *
     * While emulating the DMA cycles of a single rasterline, Denise writes
     * the fetched bitplane data into this buffer. It contains the raw
     * bitplane bits coming out the 6 serial shift registers.
     *
     * bBuffer: Border pixel buffer
     *
     * This buffer is used to determine whether a border pixel has to be drawn.
     * If the buffer contains a value of 0xFF, border drawing is off for this
     * pixel. Otherwise, the buffer contains the number of the color register
     * storing the border color.
     *
     * iBuffer: Color index buffer
     *
     * At the end of each rasterline, Denise translates the fetched bitplane
     * data to color register indices. In single-playfield mode, this is a
     * one-to-one-mapping. In dual-playfield mode, the bitplane data has to
     * be split into two color indices. Only one of them is kept depending on
     * the playfield priority bit.
     *
     * mBuffer: Multiplexed color index buffer
     *
     * This buffer contains the data from the iBuffer, multiplexed with the
     * color index data coming from the sprite synthesizer.
     *
     * zBuffer: Pixel depth buffer
     *
     * When the dBuffer is translated into the iBuffer, a depth buffer is build.
     * This buffer serves multiple purposes.
     *
     * 1. The depth buffer is utilized to manage display priority. For example,
     *    it is used to decide whether to draw a sprite pixel in front of or
     *    behind a particular playfield pixel. Note: The larger the value, the
     *    closer a pixel is. In traditonal z-buffers, it is the other way round.
     *
     * 2. The depth buffer is utilized to code meta-information about the pixels
     *    in the current rasterline. This is done by coding the pixel depth with
     *    special bit patterns storing that information. E.g., the pixel depth
     *    can be used to determine if the pixel has been drawn in dual-
     *    playfield mode or if a sprite-to-sprite collision has occurred.
     *
     * The following bit format is utilized:
     *
     * _0_ SP0 SP1 _1_ SP2 SP3 _2_ SP4 SP5 _3_ SP6 SP7 _4_ DPF PF1 PF2
     *
     *  DPF : Set if the pixel is drawn in dual-playfield mode.
     *  PF1 : Set if the pixel is solid in playfield 1.
     *  PF1 : Set if the pixel is solid in playfield 2.
     *  SPx : Set if the pixel is solid in sprite x.
     *  _x_ : Playfield priority derived from the current value in BPLCON2.
     */
    u8 dBuffer[HPIXELS + (4 * 16) + 8];
    u8 bBuffer[HPIXELS + (4 * 16) + 8];
    u8 iBuffer[HPIXELS + (4 * 16) + 8];
    u8 mBuffer[HPIXELS + (4 * 16) + 8];
    u16 zBuffer[HPIXELS + (4 * 16) + 8];

    static constexpr u16 Z_0   = 0b10000000'00000000;
    static constexpr u16 Z_SP0 = 0b01000000'00000000;
    static constexpr u16 Z_SP1 = 0b00100000'00000000;
    static constexpr u16 Z_1   = 0b00010000'00000000;
    static constexpr u16 Z_SP2 = 0b00001000'00000000;
    static constexpr u16 Z_SP3 = 0b00000100'00000000;
    static constexpr u16 Z_2   = 0b00000010'00000000;
    static constexpr u16 Z_SP4 = 0b00000001'00000000;
    static constexpr u16 Z_SP5 = 0b00000000'10000000;
    static constexpr u16 Z_3   = 0b00000000'01000000;
    static constexpr u16 Z_SP6 = 0b00000000'00100000;
    static constexpr u16 Z_SP7 = 0b00000000'00010000;
    static constexpr u16 Z_4   = 0b00000000'00001000;

    // Dual-playfield bits (meta-information, not used for depth)
    static constexpr u16 Z_DPF   = 0x1;  // Both playfields transparent
    static constexpr u16 Z_DPF1  = 0x2;  // PF1 opaque, PF2 transparent
    static constexpr u16 Z_DPF2  = 0x3;  // PF1 transparent, PF2 opaque
    static constexpr u16 Z_DPF12 = 0x4;  // Both playfields opaque, PF1 visible
    static constexpr u16 Z_DPF21 = 0x5;  // Both playfields opaque, PF2 visible
    static constexpr u16 Z_DUAL  = 0x7;  // Mask covering all DPF bits

    static constexpr u16 Z_SP[8] = {
        Z_SP0, Z_SP1, Z_SP2, Z_SP3, Z_SP4, Z_SP5, Z_SP6, Z_SP7 };
    static constexpr u16 Z_SP01234567 = Z_SP0|Z_SP1|Z_SP2|Z_SP3|Z_SP4|Z_SP5|Z_SP6|Z_SP7;
    static constexpr u16 Z_SP0246 = Z_SP0|Z_SP2|Z_SP4|Z_SP6;
    static constexpr u16 Z_SP1357 = Z_SP1|Z_SP3|Z_SP5|Z_SP7;
    
    static bool isSpritePixel(u16 z) {
        return (z & Z_SP01234567) > (z & ~Z_SP01234567);
    }
    template <int nr> static bool isSpritePixel(u16 z) {
        return (z & Z_SP[nr]) > (z & ~Z_SP[nr]);
    }
    static int upperPlayfield(u16 z) {
        return ((z & Z_DUAL) == Z_DPF2 || (z & Z_DUAL) == Z_DPF21) ? 2 : 1;
    }
    
    
    //
    // Initializing
    //
    
public:

    Denise(Amiga& ref);

    Denise& operator= (const Denise& other) {

        CLONE(config)

        CLONE(pixelEngine)
        CLONE(debugger)

        CLONE(clock)

        CLONE(diwstrt)
        CLONE(diwstop)
        CLONE(diwhigh)
        CLONE(hstrt)
        CLONE(hstop)
        CLONE(hflop)
        CLONE(borderBufferIsDirty)
        CLONE(bplcon0)
        CLONE(bplcon1)
        CLONE(bplcon2)
        CLONE(bplcon3)
        CLONE(initialBplcon0)
        CLONE(initialBplcon1)
        CLONE(initialBplcon2)
        CLONE(res)
        CLONE(pixelOffsetOdd)
        CLONE(pixelOffsetEven)
        CLONE(borderColor)
        CLONE_ARRAY(bpldat)
        CLONE_ARRAY(bpldatPipe)
        CLONE(clxdat)
        CLONE(clxcon)
        CLONE_ARRAY(shiftReg)
        CLONE(armedOdd)
        CLONE(armedEven)
        CLONE(conChanges)
        // for (isize i = 0; i < 4; i++) CLONE(sprChanges[i])
        CLONE_ARRAY(sprChanges)
        CLONE(diwChanges)

        CLONE_ARRAY(sprdata)
        CLONE_ARRAY(sprdatb)
        CLONE_ARRAY(sprpos)
        CLONE_ARRAY(sprctl)
        CLONE_ARRAY(sprhpos)
        CLONE_ARRAY(sprhppos)
        CLONE_ARRAY(ssra)
        CLONE_ARRAY(ssrb)
        CLONE(armed)
        CLONE(wasArmed)
        CLONE(spriteClipBegin)
        CLONE(spriteClipEnd)

        CLONE_ARRAY(dBuffer)
        CLONE_ARRAY(bBuffer)
        CLONE_ARRAY(iBuffer)
        CLONE_ARRAY(mBuffer)
        CLONE_ARRAY(zBuffer)

        return *this;
    }


    //
    // Methods from Serializable
    //
    
private:
        
    template <class T>
    void serialize(T& worker)
    {
        worker

        << diwstrt
        << diwstop
        << diwhigh
        << hstrt
        << hstop
        << hflop
        << borderBufferIsDirty
        << bplcon0
        << bplcon1
        << bplcon2
        << bplcon3
        << initialBplcon0
        << initialBplcon1
        << initialBplcon2
        << res
        << pixelOffsetOdd
        << pixelOffsetEven
        << borderColor
        << bpldat
        << bpldatPipe
        << clxdat
        << clxcon
        << shiftReg
        << armedOdd
        << armedEven
        << conChanges
        << sprChanges
        << diwChanges

        << sprdata
        << sprdatb
        << sprpos
        << sprctl
        << sprhpos
        << sprhppos
        << ssra
        << ssrb
        << armed
        << wasArmed
        << spriteClipBegin
        << spriteClipEnd;

        if (isSoftResetter(worker)) return;

        worker

        << clock;

        if (isResetter(worker)) return;

        worker

        << config.revision
        << config.clxSprSpr
        << config.clxSprPlf
        << config.clxPlfPlf;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override;
    void _didReset(bool hard) override;
    

    //
    // Methods from Configurable
    //

public:
    
    const DeniseConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;
    

    //
    // Querying chip properties
    //

public:

    bool isOCS() const { return config.revision == DeniseRev::OCS; }
    bool isECS() const { return config.revision == DeniseRev::ECS; }


    //
    // Analyzing
    //

public:
    
    void cacheInfo(DeniseInfo &result) const override;


    //
    // Working with the bitplane shift registers
    //
    
public:
    
    // Transfers the bitplane pipeline registers to the shift registers
    void updateShiftRegistersOdd();
    void updateShiftRegistersEven();

    // Extracts a bit slice from the shift registers
    void extractSlices(u8 slices[16]);
    void extractSlicesOdd(u8 slices[16]);
    void extractSlicesEven(u8 slices[16]);

    
    //
    // Drawing bitplanes
    //

public:

    // Wrappers around the core drawing routines
    void drawLoresOdd();
    void drawLoresEven();
    void drawLoresBoth();
    void drawHiresOdd();
    void drawHiresEven();
    void drawHiresBoth();
    void drawShresOdd();
    void drawShresEven();
    void drawShresBoth();

private:
    
    // Core drawing routines
    template <Resolution mode> void drawOdd(Pixel offset);
    template <Resolution mode> void drawEven(Pixel offset);
    template <Resolution mode> void drawBoth(Pixel offset);

    // Data type used by the translation functions
    typedef struct { u16 zpf1; u16 zpf2; bool prio; bool ham; } PFState;

    // Translates the bitplane data to color register indices
    void translate();

    // Called by translate() in single-playfield mode
    void translateSPF(Pixel from, Pixel to, PFState &state);

    // Called by translate() in dual-playfield mode
    void translateDPF(Pixel from, Pixel to, PFState &state);
    
    // Called by translateDPF(...)
    template <bool prio> void translateDPF(Pixel from, Pixel to, PFState &state);

    
    //
    // Drawing the border
    //
    
private:
    
    // Determines the color register index for drawing the border
    void updateBorderColor();

    // Updates the border pixel mask (called by the hsync handler)
    void updateBorderBuffer();

    // Marks the border buffer dirty for a specific number of lines
    void markBorderBufferAsDirty(isize lines = 2);


    //
    // Drawing sprites
    //

public:

    // Setter for SPRxPOS and SPRxCTL
    void setSPRxPOS(isize x, u16 value) { sprpos[x] = value; updateSprHCoords(x); }
    void setSPRxCTL(isize x, u16 value) { sprctl[x] = value; updateSprHCoords(x); }

    // Updates the cached values for horizontal coordinates
    void updateSprHCoords(isize x);

    // Checks the z buffer and returns true if a sprite pixel is visible
    bool spritePixelIsVisible(Pixel hpos) const;

private:
    
    // Draws all sprites
    void drawSprites();
    template <Resolution R> void drawSprites();

    // Draws an sprite pair. Called by drawSprites()
    template <isize pair, Resolution R> void drawSpritePair();
    template <isize pair, Resolution R> void drawSpritePair(Pixel hstrt, Pixel hstop,
                                                            Pixel strt1, Pixel strt2);
    
    // Replays all recorded sprite register changes
    template <isize pair> void replaySpriteRegChanges();

    // Draws a single sprite pixel
    template <isize x, Resolution R> void drawSpritePixel(Pixel hpos);
    template <isize x, Resolution R> void drawAttachedSpritePixelPair(Pixel hpos);

    
    //
    // Checking collisions
    //

private:

    // Checks for sprite-sprite collisions in the current rasterline
    template <int x> void checkS2SCollisions(Pixel start, Pixel end);

    // Checks for sprite-playfield collisions in the current rasterline
    template <int x> void checkS2PCollisions(Pixel start, Pixel end);

    // Checks for playfield-playfield collisions in the current rasterline
    void checkP2PCollisions();


    //
    // Delegation methods
    //
    
public:

    // Called by Agnus at the beginning of each frame (DEPRECATED)
    void vsyncHandler();

    // Called by Agnus at the beginning of the HSYNC area
    void hsyncHandler(isize vpos);

    // Called by Agnus at the end of each line
    void eolHandler();

    // Called by Agnus at the end of each frame
    void eofHandler();

    
    //
    // Accessing registers (DeniseRegs.cpp)
    //
    
public:

    void setDIWSTRT(u16 value);
    void setDIWSTOP(u16 value);
    void setDIWHIGH(u16 value);

private:

    // Called by setDIWSTRT, setDIWSTOP, setDIWHIGH
    void setHSTRT(isize value);
    void setHSTOP(isize value);

public:

    u16 peekJOY0DATR() const;
    u16 peekJOY1DATR() const;
    void pokeJOYTEST(u16 value);

    u16 peekDENISEID();
    u16 spypeekDENISEID() const;

    template <Accessor s> void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);

    template <Accessor s> void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 oldValue, u16 newValue);

    template <Accessor s> void pokeBPLCON2(u16 value);
    void setBPLCON2(u16 value);
    
    template <Accessor s> void pokeBPLCON3(u16 value);
    void setBPLCON3(u16 value);

    u16 peekCLXDAT();
    u16 spypeekCLXDAT() const;
    void pokeCLXCON(u16 value);
    
    template <isize x, Accessor s> void pokeBPLxDAT(u16 value);
    template <isize x> void setBPLxDAT(u16 value);

    template <isize x> void pokeSPRxPOS(u16 value);
    template <isize x> void pokeSPRxCTL(u16 value);
    template <isize x> void pokeSPRxDATA(u16 value);
    template <isize x> void pokeSPRxDATB(u16 value);
    
    template <isize xx, Accessor s> void pokeCOLORxx(u16 value);
    
    
    //
    // Accessing single bits
    //
    
public:
    
    // BPLCON0
    static bool shres(u16 v) { return GET_BIT(v, 6); }
    bool shres() const { return ham(bplcon0); }
    static bool hires(u16 v) { return GET_BIT(v, 15); }
    bool hires() const { return hires(bplcon0); }
    static bool lores(u16 v) { return !hires(v); }
    bool lores() const { return lores(bplcon0); }
    static bool dbplf(u16 v) { return GET_BIT(v, 10); }
    bool dbplf() const { return dbplf(bplcon0); }
    static bool lace(u16 v) { return GET_BIT(v, 2); }
    bool lace() const { return lace(bplcon0); }
    static bool ham(u16 v) { return (v & 0x8800) == 0x0800; }
    bool ham() const { return ham(bplcon0); }
    static bool ecsena(u16 v) { return GET_BIT(v, 0); }
    bool ecsena() const { return ecsena(bplcon0); }

    // BPLCON2
    static bool pf2pri(u16 value) { return GET_BIT(value, 6); }
    bool pf2pri() const { return pf2pri(bplcon2); }
    static u16 pf1px(u16 bplcon2) { return (bplcon2 & 7); }
    u16 pf1px() const { return pf1px(bplcon2); }
    static u16 pf2px(u16 bplcon2) { return (bplcon2 >> 3) & 7; }
    u16 pf2px() const { return pf2px(bplcon2); }

    // BPLCON3
    static bool brdrblnk(u16 v) { return !!GET_BIT(v, 5); }
    bool brdrblnk() const { return brdrblnk(bplcon3); }

    // CLXCON
    template <int x> bool ensp() { return !!GET_BIT(clxcon, 12 + (x/2)); }
    u8 enbp1() const { return (u8)((clxcon >> 6) & 0b010101); }
    u8 enbp2() const { return (u8)((clxcon >> 6) & 0b101010); }
    u8 mvbp1() const { return (u8)(clxcon & 0b010101); }
    u8 mvbp2() const { return (u8)(clxcon & 0b101010); }
    
    
    //
    // Computing derived values
    //

private:

    // Computes the bitmap resolution from a given BPLCON0 value
    Resolution resolution(u16 v);

    // Computes the z buffer depth for playfield 1 or 2
    static u16 zPF(u16 prioBits);
    static u16 zPF1(u16 bplcon2) { return zPF(pf1px(bplcon2)); }
    static u16 zPF2(u16 bplcon2) { return zPF(pf2px(bplcon2)); }

    // Checks whether the BPU bits in BPLCON0 are an invalid combination
    static bool invBPU(u16 v) { return ((v >> 12) & 0b111) > (hires(v) ? 4 : 6); }
    bool invBPU() const { return invBPU(bplcon0); }

    /* Returns the Denise view of the BPU bits. The value determines how many
     * shift registers are loaded with the values of their corresponding
     * BPLxDAT registers at the end of a fetch unit. It is computed out of the
     * three BPU bits stored in BPLCON0, but not identical with them. The value
     * differs if the BPU bits reflect an invalid bit pattern.
     */
    static u8 bpu(u16 v);
    u8 bpu() const { return bpu(bplcon0); }
};

}
