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


//
// THIS CLASS IS A STUB TO GET THE VISUAL PROTOTYPE WORKING
//

class Denise : public HardwareComponent {
    
public:
    
    // Denise has been executed up to this clock cycle.
    uint64_t clock = 0;
    
    // Number of PAL rasterlines
    // static const long RASTERLINES = 625;

    // Number of vertical pixels
    // THIS VALUE IS WRONG. IT'S HERE TO FAKE THE VISUAL PROTOTYPE WORK WHICH
    // IS BASED ON THE TEXTURES USED IN VIRTUALC64.
    static const long VPIXELS = 284;
    
    // Number of horizontal pixels
    // THIS VALUE IS WRONG. IT'S HERE TO FAKE THE VISUAL PROTOTYPE WHICH
    // IS BASED ON THE TEXTURES USED IN VIRTUALC64.
    static const long HPIXELS = 428;

    static const long BUFSIZE = VPIXELS * HPIXELS * 4;
    
    // Frame counter (records the number of frames drawn since power on)
    uint64_t frame = 0;
    
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
    void executeUntil(uint64_t targetClock); 
    void endOfFrame();
};

#endif
