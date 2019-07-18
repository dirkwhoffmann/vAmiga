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
#include "Colorizer.h"

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
    Colorizer colorizer;
    

    //
    // Variables
    //
    
    // Denise has been executed up to this clock cycle.
    Cycle clock = 0;
    

    //
    // Registers
    //
    
    // The three bitplane control registers
    uint16_t bplcon0 = 0;
    uint16_t bplcon1 = 0;
    uint16_t bplcon2 = 0;
    
    // The 6 bitplane data registers
    uint16_t bpldat[6];
    
    // Sprite data registers (SPRxDATA, SPRxDATAB)
    uint16_t sprdata[8];
    uint16_t sprdatb[8];

    /* The 6 bitplane parallel-to-serial shift registers
     * Denise transfers the current values of the BPLDAT registers into the
     * shift registers after BPLDAT1 is written to. This is emulated in
     * function fillShiftRegisters().
     */
    uint32_t shiftReg[6];
    
    // Scroll values (set in pokeBPLCON1())
    int8_t scrollLoresOdd;
    int8_t scrollLoresEven;
    int8_t scrollHiresOdd;
    int8_t scrollHiresEven;

    // Indicates if we're running in HAM mode (updated in pokeBPLCON0)
    bool ham;


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
    // Screen buffers
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

    /* Denise keeps four frame buffers, two for storing long frames and
     * another two for storing short frames. The short frame buffers are only
     * used in interlace mode. At each point in time, one of the two buffers
     * is the "working buffer" and the other one the "stable buffer". Denise
     * writes to the working buffers, only. The GPU reads from the stable
     * buffers, only. Once a frame has been completed, the working buffer
     * and the stable buffer is switched.
     */
    ScreenBuffer longFrame1;
    ScreenBuffer longFrame2;
    ScreenBuffer shortFrame1;
    ScreenBuffer shortFrame2;

    // Pointers to the working buffers
    ScreenBuffer *workingLongFrame = &longFrame1;
    ScreenBuffer *workingShortFrame = &shortFrame1;

    // Pointers to the stable buffers
    ScreenBuffer *stableLongFrame = &longFrame2;
    ScreenBuffer *stableShortFrame = &shortFrame2;

    // Pointer to the frame buffer Denise is currently working on
    ScreenBuffer *frameBuffer = &longFrame1;

    
    //
    // Drawing parameters
    //

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
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dump() override;
    
    void didLoadFromBuffer(uint8_t **buffer) override;
    
    
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

    bool hires() { return !!GET_BIT(bplcon0, 15); }
    bool lores() { return !GET_BIT(bplcon0, 15); }
    uint8_t bplconBPU() { return (bplcon0 >> 12) & 0b111; }
    bool bplconHOMOD() { return GET_BIT(bplcon0, 11); }
    bool bplconDBPLF() { return GET_BIT(bplcon0, 10); }
    bool bplconLACE() { return GET_BIT(bplcon0, 2); }

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
    void pokeCOLORx(int x, uint16_t value);

    
    //
    // Handling sprites
    //
    
    // Copy data from SPRDATA and SPRDATB into the serial shift registers
    void armSprite(int x);


    //
    // Managing the bitplane shift registers
    //

    // Transfers the bitplane register contents to the shift registers
    void fillShiftRegisters(); // DEPRECATED

    // Called by a DMA_L1_LAST or DMA_H1_LAST event
    // void flushShiftRegisters();

    
    //
    // Synthesizing pixels
    //
    
public:
    
    // Returns the frame buffer address of a certain pixel in the current line
    int *pixelAddr(int pixel);

    // Synthesizes pixels
    void drawLores(int pixels = 16);
    void drawHires(int pixels = 16);

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
    // Accessing the frame buffers
    //
    
public:
    
    // Returns one of the two stable buffers
    ScreenBuffer getStableLongFrame() {
        // pthread_mutex_lock(&lock);
        ScreenBuffer result = *stableLongFrame;
        // pthread_mutex_unlock(&lock);
        return result;
    }
    ScreenBuffer getStableShortFrame() {
        // pthread_mutex_lock(&lock);
        ScreenBuffer result = *stableShortFrame;
        // pthread_mutex_unlock(&lock);
        return result;
    }

    // Returns one of the working buffers
    // ScreenBuffer getWorkingLongFrame() { return *workingLongFrame; }
    // ScreenBuffer getWorkingShortFrame() { return *workingShortFrame; }


    // Called by Agnus at the beginning of each rasterline
    void beginOfLine(int vpos);

    // Called by Agnus at the end of a rasterline
    void endOfLine(int vpos);


    /* Makes Denise ready for the next frame
     * longFrame indicates whether the next frame is a long frame.
     * interlace indicates whether the next frame is drawn in interlace mode.
     */
    void prepareForNextFrame(bool longFrame, bool interlace);
    
    
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
