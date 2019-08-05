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

#include "HardwareComponent.h"
#include "Colors.h"
#include "PixelEngine.h"

class Denise : public HardwareComponent {

public:

    // Quick-access references
    class Agnus *agnus;
    
private:
    
    // Information shown in the GUI inspector panel
    DeniseInfo info;
    
    
    //
    // Sub components
    //
    
public:
    
    // A color synthesizer for computing RGBA values
    PixelEngine pixelEngine;
    

    //
    // Configuration
    //

    bool collisionCheck;


    //
    // Variables
    //
    
    // Denise has been executed up to this clock cycle.
    Cycle clock = 0;
    

    //
    // Registers
    //
    
    // The bitplane control registers
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;

    // The bitplane control registers at cycle 0 in the current rasterline
    uint16_t initialBplcon0;
    uint16_t initialBplcon1;
    uint16_t initialBplcon2;

    // The 6 bitplane data registers
    uint16_t bpldat[6];
    
    // Sprite data registers (SPRxDATA, SPRxDATAB)
    uint16_t sprdata[8];
    uint16_t sprdatb[8];

    /* The 6 bitplane parallel-to-serial shift registers
     * Denise transfers the current values of the BPLDAT registers into the
     * shift registers after BPLDAT1 is written to. This is emulated in
     * function fillShiftRegister().
     */
    uint32_t shiftReg[6];
    
    // Scroll values (set in pokeBPLCON1())
    int8_t scrollLoresOdd;
    int8_t scrollLoresEven;
    int8_t scrollHiresOdd;
    int8_t scrollHiresEven;


    //
    // Register change history buffer
    //

public:

    // Control register histroy
    ChangeHistory conRegHistory;


    //
    // Sprites
    //
    
    /* Horizontal trigger positions of all 8 sprites.
     * Note: The vertical trigger positions are stored inside Agnus. Denise
     * knows nothing about them.
     */
    int16_t sprhstrt[8];
    
    // The serial shift registers of all 8 sprites.
    uint32_t sprShiftReg[8];
    
    // The current DMA states aof all 8 sprites.
    // DEPRECATED. IS HELD BY AGNUS
    SprDMAState sprDmaState[8];
    
    // Attach control bits of all 8 sprites.
    uint8_t attach;

    /* Indicates which sprites are armed in the current rasterline.
     * An armed sprite is a sprite that will be drawn in this line.
     */
    uint8_t armed;


    //
    // Playfield priorities
    //

private:

    // Priority of playfield 1 (derived from bit PF1P2 - PF1P0 in BPLCON2)
    uint16_t prio1;

    // Priority of playfield 2 (derived from bit PF2P2 - PF2P0 in BPLCON2)
    uint16_t prio2;

    // Minimum of pf1pri and pf2pri
    uint16_t prio12;

    
    //
    // Rasterline data
    //

    /* Three important buffers are involved in the generation of pixel data:
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
     * be split into two color indices and the right one has to be choosen
     * according to the playfield priority bit.
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
    uint8_t bBuffer[HPIXELS + (4 * 16) + 6];
    uint8_t iBuffer[HPIXELS + (4 * 16) + 6];
    uint16_t zBuffer[HPIXELS + (4 * 16) + 6];

    const uint16_t Z_0   = 0b10000000'00000000;
    const uint16_t Z_SP0 = 0b01000000'00000000;
    const uint16_t Z_SP1 = 0b00100000'00000000;
    const uint16_t Z_1   = 0b00010000'00000000;
    const uint16_t Z_SP2 = 0b00001000'00000000;
    const uint16_t Z_SP3 = 0b00000100'00000000;
    const uint16_t Z_2   = 0b00000010'00000000;
    const uint16_t Z_SP4 = 0b00000001'00000000;
    const uint16_t Z_SP5 = 0b00000000'10000000;
    const uint16_t Z_3   = 0b00000000'01000000;
    const uint16_t Z_SP6 = 0b00000000'00100000;
    const uint16_t Z_SP7 = 0b00000000'00010000;
    const uint16_t Z_4   = 0b00000000'00001000;
    const uint16_t Z_PF1 = 0b00000000'00000100;
    const uint16_t Z_PF2 = 0b00000000'00000010;
    const uint16_t Z_DPF = 0b00000000'00000001;

    
    //
    // Drawing parameters
    //

public:

    // Position of the first and the last pixel covered by bitplane DMA
    int16_t firstCanvasPixel;
    int16_t lastCanvasPixel;

    // The current rasterline has been drawn up to this horizontal position
    // DEPRECATED
    short currentPixel;


    //
    // Constructing and destructing
    //
    
public:
    
    Denise();
    ~Denise();


    //
    // Configuring device properties
    //

    bool getCollisionCheck() { return collisionCheck; }
    void setCollisionCheck(bool value) { collisionCheck = value; debug("collisionCheck = %d\n", collisionCheck); }


    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & collisionCheck;
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clock
        & sprhstrt
        & sprShiftReg
        & sprDmaState
        & attach
        & armed
        & prio1
        & prio2
        & prio12
        & bplcon0
        & bplcon1
        & bplcon2
        & initialBplcon0
        & initialBplcon1
        & initialBplcon2
        & bpldat
        & sprdata
        & sprdatb
        & shiftReg
        & scrollLoresOdd
        & scrollLoresEven
        & scrollHiresOdd
        & scrollHiresEven
        & conRegHistory
        & firstCanvasPixel
        & lastCanvasPixel
        & currentPixel;
    }

    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _reset() override { RESET_SNAPSHOT_ITEMS }
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Reading the internal state
    //
    
public:
    
    // Returns the latest internal state recorded by inspect()
    DeniseInfo getInfo();
    SpriteInfo getSprInfo(int nr);


    //
    // Translating coordinates
    //

    // Translates a DMA cycle (hpos) to it's corresponding pixel position (ppos)
    int16_t ppos(int16_t hpos) { return (hpos * 4) + 6; }

    
    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x00A and 0x00C (r)
    uint16_t peekJOY0DATR();
    uint16_t peekJOY1DATR();

    // OCS register 0x036 (w)
    void pokeJOYTEST(uint16_t value);
    
    // OCS register 0x100 (w)
    void pokeBPLCON0(uint16_t value);
    void pokeBPLCON0(uint16_t oldValue, uint16_t newValue);

    bool hires(uint16_t v) { return !!GET_BIT(v, 15); }
    bool hires() { return hires(bplcon0); }
    bool lores(uint16_t v) { return !GET_BIT(v, 15); }
    bool lores() { return lores(bplcon0); }
    bool dbplf(uint16_t v) { return GET_BIT(v, 10); }
    bool dbplf() { return dbplf(bplcon0); }
    bool lace(uint16_t v) { return GET_BIT(v, 2); }
    bool lace() { return lace(bplcon0); }
    int bpu(uint16_t v) { return (v >> 12) & 0b111; }
    int bpu() { return bpu(bplcon0); }
    bool ham(uint16_t v) { return (v & 0x8C00) == 0x0800 && (bpu(v) == 5 || bpu(v) == 6); }
    bool ham() { return ham(bplcon0); }

    // OCS register 0x102 (w)
    void pokeBPLCON1(uint16_t value);

    // OCS register 0x104 (w)
    void pokeBPLCON2(uint16_t value);
    bool PF2PRI() { return GET_BIT(bplcon2, 6); }

    // OCS registers 0x110, ..., 0x11A (w)
    template <int x> void pokeBPLxDAT(uint16_t value);
    
    // OCS registers 0x140, 0x148, ..., 0x170, 0x178 (w)
    template <int x> void pokeSPRxPOS(uint16_t value);

    // OCS registers 0x142, 0x14A, ..., 0x172, 0x17A (w)
    template <int x> void pokeSPRxCTL(uint16_t value);

    // OCS registers 0x144, 0x14C, ..., 0x174, 0x17C (w)
    template <int x> void pokeSPRxDATA(uint16_t value);
    
    // OCS registers 0x146, 0x14E, ..., 0x176, 0x17E (w)
    template <int x> void pokeSPRxDATB(uint16_t value);

    // OCS registers 0x180, 0x181, ..., 0x1BC, 0x1BE (w)
    void pokeColorReg(uint32_t addr, uint16_t value);

    
    //
    // Handling sprites
    //

    // Returns true if sprite x is attached to sprite x - 1
    bool attached(int x);

    // Copy data from SPRDATA and SPRDATB into the serial shift registers
    void armSprite(int x);

    // Extracts the sprite priorities from BPLCON2
    void updateSpritePriorities(uint16_t bplcon2);


    //
    // Managing the bitplane shift registers
    //

    // Transfers the bitplane register contents to the shift registers
    void fillShiftRegisters()
    {
        for (int i = 0; i < 6; i++) {
            shiftReg[i] = REPLACE_LO_WORD(shiftReg[i], bpldat[i]);
        }
    }


    
    //
    // Synthesizing pixels
    //
    
public:

    // Synthesizes pixels
    template <int HIRES> void draw(int pixels);
    void drawLores(int pixels = 16) { draw<0>(pixels); }
    void drawHires(int pixels = 16) { draw<1>(pixels); }

private:

    // Translate bitplane data to color register indices
    void translate();

    // Called by translate() in single-playfield mode
    void translateSPF(int from, int to);

    // Called by translate() in dual-playfield mode
    void translateDPF(int from, int to);
    template <bool pf2pri> void translateDPF(int from, int to);


public:

    /* Draws the sprite pixels.
     * This method is called at the end of each rasterline.
     */
    void drawSprites();
    template <int x> void drawSprite();
    template <int x> void drawSpritePair();

    // Checks for sprite-to-sprite collisions
    void checkSpriteCollisions(int start); 


    /* Draws the left and the right border.
     * This method is called at the end of each rasterline.
     */
    void drawBorder(); 


    //
    // Delegation methods
    //
    
public:

    // Called by Agnus at the beginning of each rasterline
    void beginOfLine(int vpos);

    // Called by Agnus at the end of a rasterline
    void endOfLine(int vpos);

    
    //
    // Debugging the component
    //
    
    // Called by the GUI to manually change the number of active bitplanes
    void debugSetBPU(int count);
    
    // Called by the GUI to manually change the contents of BPLCONx
    void debugSetBPLCONx(unsigned x, uint16_t value);
    void debugSetBPLCONxBit(unsigned x, unsigned bit, bool value);
    void debugSetBPLCONxNibble(unsigned x, unsigned nibble, uint8_t value);

};

#endif
