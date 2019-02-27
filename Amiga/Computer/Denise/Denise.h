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

//
// THIS CLASS IS A STUB TO GET THE VISUAL PROTOTYPE WORKING
//

class Denise : public HardwareComponent {
    
public:
    
    // Color synthesizer
    Colorizer colorizer;
    
    // Denise has been executed up to this clock cycle.
    Cycle clock = 0;
    
    // Frame counter (records the number of frames drawn since power on)
    uint64_t frame = 0;
    
    
    //
    // Registers
    //
    
    // The three bitplane control registers
    uint16_t bplcon0 = 0;
    uint16_t bplcon1 = 0;
    uint16_t bplcon2 = 0;

    // The 6 bitplane data registers
    uint16_t bpldat[6]; 
    
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
    
    // Number of vertical pixels
    // THIS VALUE IS WRONG. IT'S HERE TO FAKE THE VISUAL PROTOTYPE WORK WHICH
    // IS BASED ON THE TEXTURES USED IN VIRTUALC64.
    static const long VPIXELS = 284;
    
    // Number of horizontal pixels
    // THIS VALUE IS WRONG. IT'S HERE TO FAKE THE VISUAL PROTOTYPE WHICH
    // IS BASED ON THE TEXTURES USED IN VIRTUALC64.
    static const long HPIXELS = 428;

    static const long BUFSIZE = VPIXELS * HPIXELS * 4;
    

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

    // FAKE PICTURES FOR THE VISUAL PROTOTYPE
    int *fakeImage1 = new int[HPIXELS * VPIXELS];
    int *fakeImage2 = new int[HPIXELS * VPIXELS];

    /* Currently active frame buffer
     * This variable points either to longFrame or shortFrame
     */
    int *frameBuffer = longFrame;

    /* Pointer to the beginning of the current rasterline
     * This pointer is used by all rendering methods to write pixels. It always
     * points to the beginning of a rasterline, either in longFrame or
     * shortFrame. It is reset at the beginning of each frame and incremented
     * at the beginning of each rasterline.
     */
    int *pixelBuffer = longFrame;

    /* Offset into pixelBuffer
     */
    short bufferoffset;

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
    void _dump() override;

    void didLoadFromBuffer(uint8_t **buffer) override;
  
    //
    // Collecting information
    //
    
public:
    
    // Collects the data shown in the GUI's debug panel.
    DeniseInfo getInfo();
    
    //
    // Accessing registers
    //
    
public:
   
    void pokeBPLCON0(uint16_t value);
    void pokeBPLCON1(uint16_t value);
    void pokeBPLCON2(uint16_t value);
    void pokeBPLxDAT(int x, uint16_t value);
    // uint16_t peekCOLORxx(int xx) { return colorizer.peekColorReg(xx); }
    // void pokeCOLORxx(int xx, uint16_t value) { colorizer.pokeColorReg(xx, value); }
    
    // Returns the number of active bitplanes
    // inline int activeBitplanes() { return (bplcon0 >> 12) & 0b111; }

    
    //
    // Handling the shift register
    //
    
    void fillShiftRegisters();
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    /* Returns the currently stabel screen buffer.
     * If Denise is working on the long frame, a pointer to the short frame is
     * returned and vice versa.
     */
    void *screenBuffer() { return (frameBuffer == longFrame) ? shortFrame : longFrame; }
   
    // Fills the fake pictures with some data
    void initFakePictures(const void *fake1, const void *fake2) {
        
        assert(fake1 != NULL);
        assert(fake2 != NULL);
        
        memcpy((void *)fakeImage1, fake1, BUFSIZE);
        memcpy((void *)fakeImage2, fake2, BUFSIZE);
    }
    
    // Fake some video output
    void endOfFrame();
};

#endif
