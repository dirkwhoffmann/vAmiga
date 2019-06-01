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

class Colorizer : public HardwareComponent {
    
private:
    
    // The 32 Amiga color registers
    // TimeRecorded<uint16_t> colorReg[32];
    uint16_t colorReg[32];

    /* The 64 Amiga colors in RGBA format
     * The first 32 entries represent the colors that are stored in the color
     * registers in RGBA format. The other 32 colors contain the RGBA values
     * used in halfbright mode. They are computed out of the first 32 colors by
     * dividing each color component by 2.
     */
    uint32_t colorRGBA[64];
    
    /* Color cache for all possible 4096 colors
     * When computeRGBA() is invoked, the function first checks if the color
     * is stored in the color cache. If not, it is computed on-the-fly and
     * stored in the cache. Whenever a color adjustment parameter changes,
     * the color cache is cleared.
     */
    uint32_t colorCache[4096];

    // The most recently computed HAM pixel in Amiga RGB format
    uint16_t hamRGB;


    //
    // Color adjustment parameters
    //
    
    Palette palette = COLOR_PALETTE;
    double brightness = 50.0;
    double contrast = 100.0;
    double saturation = 1.25;


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

    // Peeks a value from one of the 32 color registers.
    uint16_t peekColorReg(int reg);

    /* Pokes a value into one of the 32 color registers.
     * cycle = Master cycle in which the write happens.
     */
    void pokeColorReg(int reg, uint16_t value, Cycle cycle);
    void pokeColorRegCpu(int reg, uint16_t value);
    void pokeColorRegCopper(int reg, uint16_t value);


    //
    // Managing the color cache
    //

private:

    // Clears the color cache
    void clearColorCache();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b);

public:

    /* Computes the RGBA value for a color given in 12-bit Amiga RGB format.
     * If the requested color is already stored in the color cache, the cached
     * value is returned. Otherwise, the color is computed and written into
     * the cache before it is returned to the caller.
     */
    uint32_t computeRGBA(uint16_t rgb);
    uint32_t computeRGBA(uint8_t r, uint8_t g, uint8_t b);


    //
    // Managing the color lookup table
    //

public:

    // Returns the RGBA value for a certain color register.
    uint32_t getRGBA(int nr) { return colorRGBA[nr]; }

    // Returns the RGBA value for a certain sprite color.
    uint32_t getSpriteRGBA(int s, int nr) { return colorRGBA[16 + nr + 2 * (s & 6)]; }

    // Updates the complete color lookup table.
    void updateColorTable();

private:

    // Updates the lookup table for a certain color register.
    void updateColorTable(int nr);


    //
    // Working in HAM mode
    //

public:
    
    /* Resets the stored RGB value to the background color
     * This function needs to be called at the beginning of each rasterline.
     */
    void prepareForHAM() { hamRGB = colorReg[0]; }

    /* Computes the RGBA value for a color given in the Amiga's HAM format.
     * If the requested color is already stored in the color cache, the cached
     * value is returned. Otherwise, the color is computed and written into
     * the cache before it is returned to the caller.
     */
    uint32_t computeHAM(uint8_t index);

};

#endif

