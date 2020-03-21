// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DENISE_INC
#define _DENISE_INC

#include "AmigaComponent.h"
#include "Colors.h"
#include "PixelEngine.h"

class Denise : public AmigaComponent {

    friend class PixelEngine;
    
    // Current configuration
    DeniseConfig config;

    // Result of the latest inspection
    DeniseInfo info;

    // Sprite information recorded in the previous frame (shown by the GUI)
    SpriteInfo latchedSpriteInfo[8];
    
    // Sprite information recorded in the current frame (constantly changing)
    SpriteInfo spriteInfo[8];

    // Collected statistical information
    DeniseStats stats;

    
    //
    // Sub components
    //
    
public:
    
    // A color synthesizer for computing RGBA values
    PixelEngine pixelEngine = PixelEngine(amiga);


    //
    // Internal state
    //
    
    // Denise has been executed up to this clock cycle.
    Cycle clock = 0;

    // Remembers the pixel position of the first and the last drawn bitplane pixel
    PixelPos lastDrawnPixel;


    //
    // Registers
    //
    
    // The bitplane control registers
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;

    // The bitplane control registers at cycle 0 in the current rasterline
    u16 initialBplcon0;
    u16 initialBplcon1;
    u16 initialBplcon2;

    // The 6 bitplane data registers
    u16 bpldat[6];
    
    // Sprite data registers (SPRxDATA, SPRxDATAB)
    u16 sprdata[8];
    u16 sprdatb[8];

    // Initial values of sprite data registers at cycle 0 in the current rasterline
     u16 initialSprdata[8];
     u16 initialSprdatb[8];

    // Sprite collision registers
    u16 clxdat;
    u16 clxcon;

    /* The 6 bitplane parallel-to-serial shift registers
     * Denise transfers the current values of the BPLDAT registers into the
     * shift registers after BPLDAT1 is written to. This is emulated in
     * function fillShiftRegister().
     *
     * The upper two array elements equal 0. We need these dummy elements in
     * order to pass the array as parameter to function transposeSSE().
     */
    u16 __attribute__ ((aligned (64))) shiftReg[8];

    // Bit slices computed out of the shift registers
    u8 __attribute__ ((aligned (64))) slice[16];
    
    // Flags indicating that the shift registers have been loaded
    bool armedEven;
    bool armedOdd;

    // Extracted from BPLCON1 to emulate horizontal scrolling
    i8 pixelOffsetOdd;
    i8 pixelOffsetEven;

    
    //
    // Register change management
    //

public:

    // Ringbuffer recording control register changes
    RegChangeRecorder<128> conChanges;

    // Ringbuffer recording sprite register changes
    RegChangeRecorder<128> sprChanges;


    //
    // Sprites
    //

    // The position and control registers of all 8 sprites
    u16 sprpos[8];
    u16 sprctl[8];

    // The position and control registers at cycle 0 in the current rasterline
    u16 initialSprpos[8];
    u16 initialSprctl[8];

    // The serial shift registers of all 8 sprites.
    u16 ssra[8];
    u16 ssrb[8];
    
    // Attach control bits of all 8 sprites.
    u8 attach;

    /* Indicates which sprites are curently armed.
     * An armed sprite is a sprite that will be drawn in this line.
     */
    u8 armed;

    /* Indicates which sprites had been armed at least once in the current
     * rasterline.
     * Note that a sprite can be armed and disarmed multiple times in a
     * rasterline by manually modifying SPRxDATA and SPRxCTL, respectively.
     */
    u8 wasArmed;

    // Value of variable 'armed' at cycle 0 in the current rasterline
    u8 initialArmed;

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
    PixelPos spriteClipBegin;
    PixelPos spriteClipEnd;

    //
    // Playfield priorities
    //

private:

    // Priority of playfield 1 (derived from bit PF1P2 - PF1P0 in BPLCON2)
    u16 prio1;

    // Priority of playfield 2 (derived from bit PF2P2 - PF2P0 in BPLCON2)
    u16 prio2;

    // Minimum of prio1 and prio2
    u16 prio12;

    
    //
    // Rasterline data
    //

    /* Four important buffers are involved in the generation of pixel data:
     *
     * bBuffer: The bitplane data buffer
     *
     * While emulating the DMA cycles of a single rasterline, Denise writes
     * the fetched bitplane data into this buffer. It contains the raw
     * bitplane bits coming out the 6 serial shift registers.
     *
     * iBuffer: The color index buffer
     *
     * At the end of each rasterline, Denise translates the fetched bitplane
     * data to color register indices. In single-playfield mode, this is a
     * one-to-one-mapping. In dual-playfield mode, the bitplane data has to
     * be split into two color indices. Only one of the is kept depending on
     * the playfield priority bit.
     *
     * mBuffer: The multiplexed color index buffer
     *
     * This buffer contains the data from the iBuffer, multiplexed with the
     * color index data coming from the sprite synthesizer.
     *
     * zBuffer: The pixel depth buffer
     *
     * During the translating of the bBuffer into the iBuffer, a depth buffer
     * is build. This buffer serves multiple purposes.
     *
     * 1. The depth buffer it is used to implement the display priority. For
     *    example, it is used to decide whether to draw a sprite pixel in front
     *    of or behind a particular playing field pixel. Note: The larger the
     *    value, the closer a pixel is. In traditonal z-buffers, it is the other
     *    way round.
     *
     * 2. The depth buffer is utilized to code meta-information about the pixels
     *    in the current rasterline. This is done by coding the pixel depth with
     *    special bit patterns storing that information. E.g., the pixel depth
     *    can be used to determine, if the pixel has been drawn in dual-
     *    playfield mode or if a sprite-to-sprite collision has taken place.
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
    u8 bBuffer[HPIXELS + (4 * 16) + 6];
    u8 iBuffer[HPIXELS + (4 * 16) + 6];
    u8 mBuffer[HPIXELS + (4 * 16) + 6];
    u16 zBuffer[HPIXELS + (4 * 16) + 6];

    static const u16 Z_0   = 0b10000000'00000000;
    static const u16 Z_SP0 = 0b01000000'00000000;
    static const u16 Z_SP1 = 0b00100000'00000000;
    static const u16 Z_1   = 0b00010000'00000000;
    static const u16 Z_SP2 = 0b00001000'00000000;
    static const u16 Z_SP3 = 0b00000100'00000000;
    static const u16 Z_2   = 0b00000010'00000000;
    static const u16 Z_SP4 = 0b00000001'00000000;
    static const u16 Z_SP5 = 0b00000000'10000000;
    static const u16 Z_3   = 0b00000000'01000000;
    static const u16 Z_SP6 = 0b00000000'00100000;
    static const u16 Z_SP7 = 0b00000000'00010000;
    static const u16 Z_4   = 0b00000000'00001000;
    static const u16 Z_PF1 = 0b00000000'00000100;
    static const u16 Z_PF2 = 0b00000000'00000010;
    static const u16 Z_DPF = 0b00000000'00000001;

    constexpr static const u16 Z_SP[8] = { Z_SP0, Z_SP1, Z_SP2, Z_SP3, Z_SP4, Z_SP5, Z_SP6, Z_SP7 };
    static const u16 Z_SP01234567 = Z_SP0|Z_SP1|Z_SP2|Z_SP3|Z_SP4|Z_SP5|Z_SP6|Z_SP7;
    static const u16 Z_SP0246 = Z_SP0|Z_SP2|Z_SP4|Z_SP6;
    static const u16 Z_SP1357 = Z_SP1|Z_SP3|Z_SP5|Z_SP7;


    //
    // Constructing and serializing
    //
    
public:

    Denise(Amiga& ref);

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.revision
        & config.emulateSprites
        & config.clxSprSpr
        & config.clxSprPlf
        & config.clxPlfPlf;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clock
        & lastDrawnPixel

        & bplcon0
        & bplcon1
        & bplcon2
        & initialBplcon0
        & initialBplcon1
        & initialBplcon2
        & bpldat
        & sprdata
        & sprdatb
        & initialSprdata
        & initialSprdatb
        & clxdat
        & clxcon
        & shiftReg
        & armedEven
        & armedOdd
        & pixelOffsetOdd
        & pixelOffsetEven
        & conChanges
        & sprChanges

        & sprpos
        & sprctl
        & initialSprpos
        & initialSprctl
        & ssra
        & ssrb
        & attach
        & armed
        & wasArmed
        & initialArmed
        & spriteClipBegin
        & spriteClipEnd
        & prio1
        & prio2
        & prio12;
    }

    
    //
    // Configuring
    //

    DeniseConfig getConfig() { return config; }

    DeniseRevision getRevision() { return config.revision; }
    void setRevision(DeniseRevision type);

    bool getEmulateSprites() { return config.emulateSprites; }
    void setEmulateSprites(bool value) { config.emulateSprites = value; }
    
    bool getClxSprSpr() { return config.clxSprSpr; }
    void setClxSprSpr(bool value) { config.clxSprSpr = value; }

    bool getClxSprPlf() { return config.clxSprPlf; }
    void setClxSprPlf(bool value) { config.clxSprPlf = value; }

    bool getClxPlfPlf() { return config.clxPlfPlf; }
    void setClxPlfPlf(bool value) { config.clxPlfPlf = value; }


    //
    // Methods from HardwareComponent
    //
    
private:

    void _powerOn() override;
    void _reset() override;
    void _inspect() override;
    void _dumpConfig() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

public:

    // Returns the result of the most recent call to inspect()
    DeniseInfo getInfo();
    SpriteInfo getSpriteInfo(int nr);
    
    u16 getSpriteHeight(int nr) { return latchedSpriteInfo[nr].height; }
    u16 getSpriteColor(int nr, int reg) { return latchedSpriteInfo[nr].colors[reg]; }
    u64 getSpriteData(int nr, int line) { return latchedSpriteInfo[nr].data[line]; }

    // Returns statistical information about the current activiy
    DeniseStats getStats() { return stats; }

    // Resets the collected statistical information
    void clearStats() { memset(&stats, 0, sizeof(stats)); }


    //
    // Accessing registers
    //
    
public:

    // JOY0DATR:  $00A (r)
    // JOY1DATR:  $00C (r)
    // JOYTEST:   $036 (w)
    //

    u16 peekJOY0DATR();
    u16 peekJOY1DATR();
    void pokeJOYTEST(u16 value);

    // DENISEID: $07C (r) (ECS)
    u16 peekDENISEID();

    // BPLCON0:  $100 (w)

    /*      15 : HIRES         High-resolution enable
     * 14 - 12 : BPU2 - BPU0   Number of bit-planes used
     *      11 : HOMOD         Hold-and-modify enable
     *      10 : DBPLF         Dual-playfield enable
     *       9 : COLOR         Color enable
     *       8 : GAUD          Genlock audio enable
     *   7 - 4 : ---
     *       3 : LPEN          Light pen enable
     *       2 : LACE          Interlace enable
     *       1 : ERSY          External synchronization enable
     *       0 : ---
     */
    void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);
    void setBPLCON0(u16 newValue) { setBPLCON0(bplcon0, newValue); }

    static bool hires(u16 v) { return GET_BIT(v, 15); }
    bool hires() { return hires(bplcon0); }
    static bool lores(u16 v) { return !hires(v); }
    bool lores() { return lores(bplcon0); }
    static bool dbplf(u16 v) { return GET_BIT(v, 10); }
    bool dbplf() { return dbplf(bplcon0); }
    static bool lace(u16 v) { return GET_BIT(v, 2); }
    bool lace() { return lace(bplcon0); }
    static bool ham(u16 v) { return (v & 0x8C00) == 0x0800 && (bpu(v) == 5 || bpu(v) == 6); }
    bool ham() { return ham(bplcon0); }

    /* Returns the Denise view of the BPU bits.
     * The value determines how many shift registers are loaded with the values
     * of their corresponding BPLxDAT registers at the end of a fetch unit.
     * It is computed out of the three BPU bits stored in BPLCON0, but not
     * identical with them. The value differs if the BPU bits reflect an invalid
     * bit pattern.
     * Compare with Agnus::bpu() which returns the Agnus view of the BPU bits.
     */
    static int bpu(u16 v);
    int bpu() { return bpu(bplcon0); }

    // BPLCON1:  $102 (w)
    void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 value);

    // BPLCON2:  $104 (w)
    void pokeBPLCON2(u16 value);
    void setBPLCON2(u16 value);
    static int PF2PRI(u16 v) { return GET_BIT(v, 6); }
    bool PF2PRI() { return PF2PRI(bplcon2); }

    // Computes the z buffer depth for playfield 1 or 2
    static u16 zPF(u16 priorityBits);
    static u16 zPF1(u16 bplcon2) { return zPF(bplcon2 & 7); }
    static u16 zPF2(u16 bplcon2) { return zPF((bplcon2 >> 3) & 7); }

    // CLXDAT:   $00E (r)
    // CLXCON:   $098 (w)
    u16 peekCLXDAT();
    void pokeCLXCON(u16 value);

    // BPLxDAT:  $110 - $11A (w)
    template <int x> void pokeBPLxDAT(u16 value);

    // SPRxPOS:  $140, $148 ... $170, $178 (w)
    // SPRxCTL:  $142, $14A ... $172, $17A (w)
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

    static i16 sprhpos(u16 sprpos, u16 sprctl) {
        return ((sprpos & 0xFF) << 1) | (sprctl & 0x01);
    }
    template <int x> i16 sprhpos() { return sprhpos(sprpos[x], sprctl[x]); }

    // SPRxDATA: $144, $14C ... $174, $17C (w)
    // SPRxDATB: $146, $14E ... $176, $17E (w)
    template <int x> void pokeSPRxDATA(u16 value);
    template <int x> void pokeSPRxDATB(u16 value);

    // COLORxx:  $180, $181 ... $1BC, $1BE (w)
    template <PokeSource s, int xx> void pokeCOLORxx(u16 value);

    
    //
    // Handling sprites
    //

    // Returns true if sprite x is attached to sprite x - 1
    bool attached(int x);

    // Copy data from SPRDATA and SPRDATB into the serial shift registers
    void armSprite(int x);

    // Checks the z buffer and returns true if a sprite pixel is visible
    bool spritePixelIsVisible(int hpos);

    // Extracts the sprite priorities from BPLCON2 (DEPRECATED)
    void updateSpritePriorities(u16 bplcon2);


    //
    // Handling bitplanes
    //

    // Transfers the bitplane registers to the shift registers
    void fillShiftRegisters();

    
    //
    // Synthesizing pixels
    //
    
public:

    // Synthesizes pixels
    template <bool hiresMode> void drawOdd(int offset);
    template <bool hiresMode> void drawEven(int offset);
    template <bool hiresMode> void drawBoth(int offset);
    void drawHiresOdd()  { if (armedOdd)  drawOdd <true>  (pixelOffsetOdd);  }
    void drawHiresEven() { if (armedEven) drawEven<true>  (pixelOffsetEven); }
    void drawHiresBoth();
    void drawLoresOdd()  { if (armedOdd)  drawOdd <false> (pixelOffsetOdd);  }
    void drawLoresEven() { if (armedEven) drawEven<false> (pixelOffsetEven); }
    void drawLoresBoth();

private:

    // Translate bitplane data to color register indices
    void translate();

    // Called by translate() in single-playfield mode
    void translateSPF(int from, int to);

    // Called by translate() in dual-playfield mode
    void translateDPF(bool pf2pri, int from, int to);
    template <bool pf2pri> void translateDPF(int from, int to);


public:

    // Draws all armed sprites. Called at the end of a rasterline
    void drawSprites();

    // Draws an armed sprite pair. Called by drawSprites()
    template <int x> void drawSpritePair();
    template <int x> void drawSpritePair(int hstrt, int hstop,
                                         int strt1, int strt2,
                                         u16 data1, u16 data2,
                                         u16 datb1, u16 datb2,
                                         bool armed1, bool armed2, bool at);

    // Draws a single sprite pixel
    template <int x> void drawSpritePixel(int hpos);
    template <int x> void drawAttachedSpritePixelPair(int hpos);

    /* Draws the left and the right border
     * This method is called at the end of each rasterline.
     */
    void drawBorder(); 

    //
    // Collision checking
    //

public:

    // Checks for sprite-sprite collisions in the current rasterline
    template <int x> void checkS2SCollisions(int start, int end);

    // Checks for sprite-playfield collisions in the current rasterline
    template <int x> void checkS2PCollisions(int start, int end);

    // Checks for playfield-playfield collisions in the current rasterline
    void checkP2PCollisions();

private:

    // Getter for CLXCON bits
    template <int x> u16 getENSP() { return GET_BIT(clxcon, 12 + (x/2)); }
    u16 getENBP1() { return (clxcon >> 6) & 0b010101; }
    u16 getENBP2() { return (clxcon >> 6) & 0b101010; }
    u16 getMVBP1() { return clxcon & 0b010101; }
    u16 getMVBP2() { return clxcon & 0b101010; }


    //
    // Delegation methods
    //
    
public:

    // Called by Agnus at the beginning of each frame
    void beginOfFrame(bool interlace);

    // Called by Agnus at the beginning of each rasterline
    void beginOfLine(int vpos);

    // Called by Agnus at the end of a rasterline
    void endOfLine(int vpos);

    // Called by Agnus if the DMACON register changes
    void pokeDMACON(u16 oldValue, u16 newValue);


    //
    // Debugging the component
    //

    // Gathers the sprite data for the displayed sprite
    void recordSpriteData(unsigned x);

    // Dumps the bBuffer or the iBuffer to the console
    void dumpIBuffer() { dumpBuffer(iBuffer, sizeof(iBuffer)); }
    void dumpBBuffer() { dumpBuffer(bBuffer, sizeof(bBuffer)); }
    void dumpBuffer(u8 *buffer, size_t length);

};

#endif
