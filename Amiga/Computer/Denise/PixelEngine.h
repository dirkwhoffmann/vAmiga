// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _COLORIZER_INC
#define _COLORIZER_INC

#include "HardwareComponent.h"

class PixelEngine : public HardwareComponent {

public:

    // Quick-access references
    class Denise *denise;

private:

    /* Color lookup table DEPRECATED
     *
     *  0 .. 31: Values of the 32 Amiga color registers.
     * 32 .. 63: Additional colors used in halfbright mode.
     * 64 .. 71: Some predefined debug colors
     */
    // uint16_t colors[colorTableCnt];

    // The 32 color registers
    uint16_t colreg[32];

    // RGBA values for all possible 4096 Amiga colors
    uint32_t rgba[4096];

    /* The color register values translated to RGBA
     * Note that the number of elements exceeds the number of color registers:
     *  0 .. 31: RGBA values of the 32 color registers.
     * 32 .. 63: RGBA values of the 32 color registers in halfbright mode.
     * 64 .. 71: Additional colors used for debugging
     */
    static const int rgbaIndexCnt = 32 + 32 + 8;
    uint32_t indexedRgba[rgbaIndexCnt];

    // The most recently computed HAM pixel in Amiga RGB format
    uint16_t hamRGB;

    // The current drawing mode
    DrawingMode mode;


    //
    // Color adjustment parameters
    //
    
    Palette palette = COLOR_PALETTE;
    double brightness = 50.0;
    double contrast = 100.0;
    double saturation = 1.25;


    //
    // Register change history
    //

    /* Recorded register changes
     * The change history is recorded by Denise and reset at the end of each
     * scanline.
     */
    RegisterChange changeHistory[128];

    // Number of recorded register changes
    int changeCount = 0;
    

    //
    // Constructing and destructing
    //
    
public:
    
    PixelEngine();


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

        & colreg
        & mode
        & changeHistory
        & changeCount;
    }


    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _reset() override { RESET_SNAPSHOT_ITEMS }
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring the color palette
    //
    
public:
    
    Palette getPalette() { return palette; }
    void setPalette(Palette p);
    
    double getBrightness() { return brightness; }
    void setBrightness(double value);
    
    double getSaturation() { return saturation; }
    void setSaturation(double value);
    
    double getContrast() { return contrast; }
    void setContrast(double value);


    //
    // Accessing color registers
    //

public:

    // Performs a consistency check for debugging.
    bool isRgbaIndex(int nr) { return nr < rgbaIndexCnt; }
    
    // Changes one of the 32 Amiga color registers.
    void setColor(int reg, uint16_t value);

    // Returns a color value in Amiga format or RGBA format
    uint16_t getColor(int nr) { assert(nr < 32); return colreg[nr]; }
    uint32_t getRGBA(int nr) { assert(nr < 32); return indexedRgba[nr]; }

    // Returns sprite color in Amiga format or RGBA format
    uint16_t getSpriteColor(int s, int nr) { assert(s < 8); return getColor(16 + nr + 2 * (s & 6)); }
    uint32_t getSpriteRGBA(int s, int nr) { return rgba[getSpriteColor(s,nr)]; }


    //
    // Using the color lookup table
    //

private:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b);


    //
    // Working with recorded register changes
    //

public:

    // Records a color register change to be processed in translateToRGBA()
    void recordRegisterChange(uint32_t addr, uint16_t value, int16_t pixel);

    // Applies a register change
    void applyRegisterChange(const RegisterChange &change);


    //
    // Synthesizing pixels
    //

public:

    // Translates bitplane data to RGBA values
    void translateToRGBA(uint8_t *src, int *dest);
    // void translateToRGBA_HAM(uint8_t *src, int *dest);

    /* Draws a chunk of pixels.
     * There are three variants of this function:
     *
     *     drawSP:  Draws in single-playfield mode
     *     drawDP:  Draws in dual-playfield mode
     *     drawHAM: Draws in HAM mode
     *
     * Parameters:
     *
     *     src:     Pointer to a buffer storing color indices
     *     dst:     Pointer to a buffer storing RGBA values
     *     from:    First pixel to draw
     *     to:      Last pixel to draw + 1
     *
     * Side effects:
     *
     *     The source buffer is cleared (0 is written)
     */
    void drawSPF(uint8_t *src, int *dst, int from, int to);
    void drawDPF(uint8_t *src, int *dst, int from, int to);
    void drawHAM(uint8_t *src, int *dst, int from, int to);

};

#endif
