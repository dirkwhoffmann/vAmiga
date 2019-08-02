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
    // MOVE TO Denise
    uint8_t prio1;

    // Priority of playfield 2 (derived from bit PF2P2 - PF2P0 in BPLCON2)
    // MOVE TO Denise
    uint8_t prio2;

    // Minimum of pf1pri and pf2pri
    // MOVE TO Denise
    uint8_t prioMin;

    
    //
    // Rasterline data
    //

    /* Bitplane data of the currently drawn rasterline.
     * While emulating the DMA cycles in a single rasterline, Denise writes
     * the fetched bitplane data into this array. Each array element stores
     * the color register index of a single pixel that will later appear on
     * the screen. After the rasterline is finished, Denise reads the values
     * from this array, translates them into RGBA, and writes the RGBA values
     * into one of the four frame buffers (see below).
     */
    uint8_t rasterline[HPIXELS + (4 * 16) + 6];

    // zBuffer to implement the sprite / playfield display hierarchy
    int8_t zBuffer[HPIXELS + (4 * 16) + 6];

    
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
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
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
        & prioMin
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
    
    // Copy data from SPRDATA and SPRDATB into the serial shift registers
    void armSprite(int x);

    // Extracts the sprite priorities from BPLCON2
    void updateSpritePriorities(uint16_t bplcon2);


    //
    // Managing the bitplane shift registers
    //

    // Transfers the bitplane register contents to the shift registers
    template <int x> void fillShiftRegister(uint16_t value)
    {
        shiftReg[x] = REPLACE_LO_WORD(shiftReg[x], value);
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

    // Called by translate()
    // void applyRegisterChange(const RegisterChange &change);

public:

    /* Draws the sprite pixels.
     * This method is called at the end of each rasterline.
     */
    void drawSprites();

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
