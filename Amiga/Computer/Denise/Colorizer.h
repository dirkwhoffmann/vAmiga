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

typedef struct {

    uint32_t addr;
    uint16_t value;
    int16_t pixel;
}
RegisterChange;

class Colorizer : public HardwareComponent {
    
private:
    
    /* Color lookup table
     *
     *  0 .. 31: Values of the 32 Amiga color registers.
     * 32 .. 63: Additional colors used in halfbright mode.
     * 64 .. 71: Some predefined debug colors
     */
    static const int colorTableCnt = 32 + 32 + 8;
    uint16_t colors[colorTableCnt];

    // The RGBA values of all 4096 Amiga colors
    uint32_t rgba[4096];

    // The most recently computed HAM pixel in Amiga RGB format
    uint16_t hamRGB;


    //
    // Color adjustment parameters
    //
    
    Palette palette = COLOR_PALETTE;
    double brightness = 50.0;
    double contrast = 100.0;
    double saturation = 1.25;


    /* Recorded color register changes
     * The recorded changes are processed in translateToRGBA()
     */
    RegisterChange colorChanges[128];

    // Number of recorded color changes
    int colorChangeCount = 0;
    

    //
    // Constructing and destructing
    //
    
public:
    
    Colorizer();


    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;


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
    bool isColorTableIndex(int nr) { return nr < colorTableCnt; }
    
    // Changes one of the 32 Amiga color registers.
    void setColor(int reg, uint16_t value);

    // Returns a color value in Amiga format or RGBA format
    uint16_t getColor(int nr) { assert(isColorTableIndex(nr)); return colors[nr]; }
    uint32_t getRGBA(int nr) { return rgba[getColor(nr)]; }

    // Returns sprite color in Amiga format or RGBA format
    uint16_t getSpriteColor(int s, int nr) { assert(s < 8); return getColor(16 + nr + 2 * (s & 6)); }
    uint32_t getSpriteRGBA(int s, int nr) { return rgba[getSpriteColor(s,nr)]; }


    //
    // Managing the color lookup table
    //

private:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b);


    //
    // Working in HAM mode
    //

public:
    
    /* Resets the stored RGB value to the background color
     * This function needs to be called at the beginning of each rasterline.
     */
    void prepareForHAM() { hamRGB = colors[0]; }

    // Computes the Amiga color value for a color given in HAM format.
    uint16_t computeHAM(uint8_t index);


    //
    // Synthesizing pixels
    //

public:

    // Records a color register change to be processed in translateToRGBA()
    void recordColorRegisterChange(uint32_t addr, uint16_t value, int16_t pixel);

    // Translates bitplane data to RGBA values.
    void translateToRGBA(const uint8_t *src, int *dest);
    void translateToRGBA_HAM(const uint8_t *src, int *dest);
};

#endif

