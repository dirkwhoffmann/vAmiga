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
#include "Colorizer.h"

class Denise : public HardwareComponent {
    
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
    // Counters
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
    
    // Sprite control word 1 (SPRxPOS)
    uint16_t sprpos[8];
    
    // Sprite control word 2 (SPRxCTL)
    uint16_t sprctl[8];
    
    // Counter for digital (mouse) input (port 1 and 2)
    uint16_t joydat[2];
    
    /* The 6 bitplane parallel-to-serial shift registers
     * Denise transfers the current values of the bpldat registers into
     * the shift registers after BPLDAT1 is written to. This is emulated
     * in function fillShiftRegisters().
     */
    uint32_t shiftReg[6];
    
    // Scroll values. The variables are set in pokeBPLCON1()
    int8_t scrollLowEven;
    int8_t scrollLowOdd;
    int8_t scrollHiEven;
    int8_t scrollHiOdd;
    
    
    //
    // Screen buffers
    //
    
    /* Screen buffer for long and short frames
     *
     *   - Long frames consist of the odd rasterlines 1, 3, 5, ..., 625.
     *   - Short frames consist of the even rasterlines 2, 4, 6, ..., 624.
     *
     * Paula writes the graphics output into this buffer. At a later stage,
     * both buffers are converted into textures. After that, the GPU merges
     * them into the final image and draws it on the screen.
     */
    int *longFrame = new int[HPIXELS * VPIXELS];
    int *shortFrame = new int[HPIXELS * VPIXELS];
    
    /* Currently active frame buffer
     * This variable points either to longFrame or shortFrame
     */
    int *frameBuffer = longFrame;
    
    /* Pointer to the beginning of the current rasterline
     * This pointer is used by all rendering methods to write pixels. It always
     * points to the beginning of a rasterline, either in longFrame or
     * shortFrame. It is reset at the beginning of each frame and incremented
     * at the beginning of each rasterline.
     * DEPRECATED
     */
    // int *rasterline = longFrame;
    
    // The current rasterline has been drawn up to this horizontal position
    short pixel;
    
    
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
    
    
    //
    // Accessing registers
    //
    
    public:
    
    // BPLCON0
    void pokeBPLCON0(uint16_t value);
    
    // Returns true if we're running in HIRES mode
    inline bool hires() { return (bplcon0 & 0x8000); }
    
    // Returns true if we're running in LORES mode
    inline bool lores() { return !(bplcon0 & 0x8000); }
    
    // BPLCON0, BPLCON1
    void pokeBPLCON1(uint16_t value);
    void pokeBPLCON2(uint16_t value);
    
    // BPL1DAT ... BPL6DAT
    void pokeBPLxDAT(int x, uint16_t value);
    
    // SPR0POS ... SPR7POS
    void pokeSPRxPOS(int x, uint16_t value);
    void pokeSPRxCTL(int x, uint16_t value);
    
    // JOY0DATR, JOY1DATR
    uint16_t peekJOYxDATR(int x);
    
    
    
    //
    // Processing events
    //
    
    // Processes an overdue event
    // void serviceEvent(EventID id, int64_t data);
    
    
    
    //
    // Handling DMA
    //
    
    void fillShiftRegisters();
    
    
    //
    // Synthesizing pixels
    //
    
    private:
    
    // Returns the frame buffer address of a certain pixel in the current line
    int *pixelAddr(int pixel);
    
    public:
    
    // Synthesizes 16 pixels from the current shift register data
    void draw16();
    
    
    //
    // Accessing the frame buffers
    //
    
    public:
    
    /* Returns true if the long frame / short frame is ready for display
     * The long frame is ready for display, if Denise is currently working on
     * on the short frame and vice verse.
     */
    inline bool longFrameIsReady() { return (frameBuffer == shortFrame); }
    inline bool shortFrameIsReady() { return (frameBuffer == longFrame); }
    
    // Returns the long frame screen buffer
    inline void *getLongFrame() { return longFrame; }

    // Returns the short frame screen buffer
    inline void *getShortFrame() { return shortFrame; }

    /* Returns the currently stabel screen buffer.
     * If Denise is working on the long frame, a pointer to the short frame is
     * returned and vice versa.
     */
    inline void *screenBuffer() { return (frameBuffer == longFrame) ? shortFrame : longFrame; }
    
    // HSYNC handler
    void endOfLine();

    // VSYNC handler
    void endOfFrame();
    
    
    //
    // Debugging the component
    //
    
    // Called by the GUI to manually change the number of active bitplanes
    void debugSetActivePlanes(int count);
    
    // Called by the GUI to manually change the contents of BPLCON0
    void debugSetBPLCON0Bit(unsigned bit, bool value);
};

#endif
