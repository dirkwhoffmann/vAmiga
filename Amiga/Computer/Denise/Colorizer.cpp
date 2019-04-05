// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Colorizer.h"

Colorizer::Colorizer()
{
    setDescription("Colorizer");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &colorReg,  sizeof(colorReg),  WORD_ARRAY },
    });
}

void
Colorizer::_powerOn()
{
    clearColorCache();
}

uint16_t
Colorizer::peekColorReg(int reg)
{
    assert(reg < 32);
    
    return colorReg[reg] & 0xFFF;
}

void
Colorizer::pokeColorReg(int reg, uint16_t value)
{
    assert(reg < 32);
    
    colorReg[reg] = value;
    computeRGBA(reg);
}

void
Colorizer::setPalette(Palette p)
{
    palette = p;
    
    clearColorCache();
    updateRGBAs();
}

void
Colorizer::setBrightness(double value)
{
    brightness = value;
    
    clearColorCache();
    updateRGBAs();
}

void
Colorizer::setSaturation(double value)
{
    saturation = value;
    
    clearColorCache();
    updateRGBAs(); 
}

void
Colorizer::setContrast(double value)
{
    contrast = value;

    clearColorCache();
    updateRGBAs();    
}

void
Colorizer::clearColorCache()
{
    memset(colorCache, 0, sizeof(colorCache));
}

void
Colorizer::computeRGBA(int reg)
{
    assert(reg < 32);

    uint8_t r,g,b;

    // xx xx xx xx R3 R2 R1 R0 G3 G2 G1 G0 B3 B2 B1 B0
    uint32_t col = colorReg[reg] & 0xFFF;
    
    // Check if RGBA value is stored in the color cache
    if (colorCache[col] != 0) {
        
        r = colorCache[col] & 0xFF;
        g = (colorCache[col] >> 8) & 0xFF;
        b = (colorCache[col] >> 16) & 0xFF;
        
    } else {
     
        r = ((colorReg[reg] >> 8) & 0xF) << 4;
        g = ((colorReg[reg] >> 4) & 0xF) << 4;
        b = ((colorReg[reg] >> 0) & 0xF) << 4;
        
        adjustRGB(r, g, b);
    }
    
    // Store color values
    colorRGBA[reg] = HI_HI_LO_LO(0xFF, b, g, r);
    colorRGBA[reg + 32] = HI_HI_LO_LO(0xfF, b / 2, g / 2, r / 2);
    colorCache[col] = colorRGBA[reg];
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

void
Colorizer::updateRGBAs()
{
    for (unsigned reg = 0; reg < 32; reg++) {
        computeRGBA(reg);
    }
}
