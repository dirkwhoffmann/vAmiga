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
     
        r = (colorReg[reg] >> 8) & 0xF;
        g = (colorReg[reg] >> 4) & 0xF;
        b = colorReg[reg] & 0xF;
        
        // At this point we can apply arbitrary color adjustments
        // For now, we do a simple scaling
        r <<= 4;
        g <<= 4;
        b <<= 4;
    }
    
    // Store color values
    colorRGBA[reg] = HI_HI_LO_LO(0xFF, b, g, r);
    colorRGBA[reg + 32] = HI_HI_LO_LO(0xFF, b / 2, g / 2, r / 2);
    colorCache[col] = colorRGBA[reg];
}

void
Colorizer::updateRGBAs()
{
    for (unsigned reg = 0; reg < 32; reg++) {
        computeRGBA(reg);
    }
}
