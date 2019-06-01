// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Colorizer::Colorizer()
{
    setDescription("Colorizer");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &colorReg,  sizeof(colorReg),  WORD_ARRAY },
    });

    clearColorCache();
}

void
Colorizer::_powerOn()
{
    clearColorCache();
    updateRGBA();
}


void
Colorizer::setPalette(Palette p)
{
    palette = p;
    
    clearColorCache();
    updateColorTable();
    updateRGBA();
}

void
Colorizer::setBrightness(double value)
{
    brightness = value;
    
    clearColorCache();
    updateColorTable();
    updateRGBA();
}

void
Colorizer::setSaturation(double value)
{
    saturation = value;
    
    clearColorCache();
    updateColorTable();
    updateRGBA();
}

void
Colorizer::setContrast(double value)
{
    contrast = value;

    clearColorCache();
    updateColorTable();
    updateRGBA();

}

uint16_t
Colorizer::peekColorReg(int reg)
{
    assert(reg < 32);

    return colorReg[reg];
}

void
Colorizer::pokeColorReg(int reg, uint16_t value, Cycle cycle)
{
    assert(reg < 32);

    debug(COL_DEBUG, "pokeCOLOR%02d(%X)\n", reg, value);

    colorReg[reg] = value & 0xFFF;
    updateColorTable(reg);
}

void
Colorizer::pokeColorRegCpu(int reg, uint16_t value)
{
    pokeColorReg(reg, value, amiga->masterClock);
}

void
Colorizer::pokeColorRegCopper(int reg, uint16_t value)
{
    pokeColorReg(reg, value, amiga->agnus.clock);
}

void
Colorizer::clearColorCache()
{
    memset(colorCache, 0, sizeof(colorCache));
    memset(colorRGBA, 0, sizeof(colorRGBA));
}

void
Colorizer::adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b)
{
    // Normalize adjustment parameters
    double brightness = this->brightness - 50.0;
    double contrast = this->contrast / 100.0;
    double saturation = this->saturation / 50.0;

    // Convert RGB to YUV
    double y =  0.299 * r + 0.587 * g + 0.114 * b;
    double u = -0.147 * r - 0.289 * g + 0.436 * b;
    double v =  0.615 * r - 0.515 * g - 0.100 * b;

    // Adjust saturation
    u *= saturation;
    v *= saturation;

    // Apply contrast
    y *= contrast;
    u *= contrast;
    v *= contrast;

    // Apply brightness
    y += brightness;

    // Translate to monochrome if applicable
    switch(palette) {

        case BLACK_WHITE_PALETTE:
            u = 0.0;
            v = 0.0;
            break;

        case PAPER_WHITE_PALETTE:
            u = -128.0 + 120.0;
            v = -128.0 + 133.0;
            break;

        case GREEN_PALETTE:
            u = -128.0 + 29.0;
            v = -128.0 + 64.0;
            break;

        case AMBER_PALETTE:
            u = -128.0 + 24.0;
            v = -128.0 + 178.0;
            break;

        case SEPIA_PALETTE:
            u = -128.0 + 97.0;
            v = -128.0 + 154.0;
            break;

        default:
            assert(palette == COLOR_PALETTE);
    }

    // Convert YUV to RGB
    double newR = y             + 1.140 * v;
    double newG = y - 0.396 * u - 0.581 * v;
    double newB = y + 2.029 * u;
    newR = MAX(MIN(newR, 255), 0);
    newG = MAX(MIN(newG, 255), 0);
    newB = MAX(MIN(newB, 255), 0);

    // Apply Gamma correction for PAL models
    /*
     r = gammaCorrect(r, 2.8, 2.2);
     g = gammaCorrect(g, 2.8, 2.2);
     b = gammaCorrect(b, 2.8, 2.2);
     */

    r = uint8_t(newR);
    g = uint8_t(newG);
    b = uint8_t(newB);
}

uint32_t
Colorizer::computeRGBA(uint16_t rgb)
{
    assert(rgb < 4096);

    /*
    // Compute color if it is not cached yet
    if (colorCache[rgb] == 0) {

        uint8_t r = (rgb >> 4) & 0xF0;
        uint8_t g = (rgb >> 0) & 0xF0;
        uint8_t b = (rgb << 4) & 0xF0;

        // Convert the Amiga value to an RGBA value
        adjustRGB(r, g, b);

        // Write value into cache
        colorCache[rgb] = HI_HI_LO_LO(0xFF, b, g, r);
    }

    return colorCache[rgb];
    */
    return rgba[rgb];
}

uint32_t
Colorizer::computeRGBA(uint8_t r, uint8_t g, uint8_t b)
{
    return computeRGBA((r << 8) | (g << 4) | b);
}

void
Colorizer::updateRGBA()
{
    debug("updateRGBA\n");

    // Iterate through all 4096 colors
    for (uint16_t col = 0x000; col <= 0xFFF; col++) {

        // Convert the Amiga color into an RGBA value
        uint8_t r = (col >> 4) & 0xF0;
        uint8_t g = (col >> 0) & 0xF0;
        uint8_t b = (col << 4) & 0xF0;

        // Convert the Amiga value to an RGBA value
        adjustRGB(r, g, b);

        // Write the result into the register lookup table
        rgba[col] = HI_HI_LO_LO(0xFF, b, g, r);
    }
}

void
Colorizer::updateColorTable()
{
    for (unsigned reg = 0; reg < 32; reg++) {
        updateColorTable(reg);
    }
}

void
Colorizer::updateColorTable(int nr)
{
    assert(nr < 32);

    // xx xx xx xx R3 R2 R1 R0 G3 G2 G1 G0 B3 B2 B1 B0
    uint32_t col = colorReg[nr] & 0xFFF;

    // Convert the Amiga color into an RGBA value
    uint32_t rgba = computeRGBA(col >> 8, (col >> 4) & 0xF, col & 0xF);

    // Write the result into the register lookup table
    colorRGBA[nr] = rgba;

    // Compute the half-bright version for this color
    uint8_t b = (rgba >> 12) & 0xFF;
    uint8_t g = (rgba >> 8) & 0xFF;
    uint8_t r = (rgba >> 0) & 0xFF;
    colorRGBA[nr + 32] = HI_HI_LO_LO(0xFF, b / 2, g / 2, r / 2);
}

uint32_t
Colorizer::computeHAM(uint8_t index)
{
    assert(index < 64);

    switch (index >> 4) {

        case 0b00: // Get color from register

            hamRGB = colorReg[index];
            break;

        case 0b01: // Modify blue

            hamRGB &= 0xFF0;
            hamRGB |= (index & 0b1111);
            break;

        case 0b10: // Modify red

            hamRGB &= 0x0FF;
            hamRGB |= (index & 0b1111) << 8;
            break;

        case 0b11: // Modify green

            hamRGB &= 0xF0F;
            hamRGB |= (index & 0b1111) << 4;
            break;

        default:
            assert(false);
    }

    return computeRGBA(hamRGB);
    // hamRGB &= 0xFFF;
    // return ((hamRGB & 0xF00) >> 4) | ((hamRGB & 0xF0) << 8) | ((hamRGB & 0xF) << 20);
}
