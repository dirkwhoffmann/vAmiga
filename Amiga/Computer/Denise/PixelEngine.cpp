// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PixelEngine::PixelEngine()
{
    setDescription("PixelEngine");
    
    // Setup some debug colors (in Amiga color format)
    colors[64] = 0x0F00;
    colors[65] = 0x0D00;
    colors[66] = 0x0A00;
    colors[67] = 0x0900;
    colors[68] = 0x00FF;
    colors[69] = 0x00DD;
    colors[70] = 0x00AA;
    colors[71] = 0x0099;
}

void
PixelEngine::_initialize()
{
    denise = &amiga->denise;
}

void
PixelEngine::_powerOn()
{
    updateRGBA();
}

void
PixelEngine::setPalette(Palette p)
{
    palette = p;
    updateRGBA();
}

void
PixelEngine::setBrightness(double value)
{
    brightness = value;
    updateRGBA();
}

void
PixelEngine::setSaturation(double value)
{
    saturation = value;
    updateRGBA();
}

void
PixelEngine::setContrast(double value)
{
    contrast = value;
    updateRGBA();

}

void
PixelEngine::setColor(int reg, uint16_t value)
{
    assert(reg < 32);

    debug(COL_DEBUG, "setColor%02d(%X)\n", reg, value);

    uint8_t r = (value & 0xF00) >> 8;
    uint8_t g = (value & 0x0F0) >> 4;
    uint8_t b = (value & 0x00F);

    colors[reg] = value & 0xFFF;
    colors[reg + 32] = ((r / 2) << 8) | ((g / 2) << 4) | (b / 2);

    /*
    colors[8] = 0xF00;
    colors[9] = 0xFF0;
    colors[10] = 0x0F0;
    colors[11] = 0x00F;
    */
}

void
PixelEngine::updateRGBA()
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
PixelEngine::adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b)
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

uint16_t
PixelEngine::computeHAM(uint8_t index)
{
    switch ((index >> 4) & 0b11) {

        case 0b00: // Get color from register

            hamRGB = colors[index];
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

    return hamRGB;
}

void
PixelEngine::recordRegisterChange(uint32_t addr, uint16_t value, int16_t pixel) {

    // debug("recordRegisterChange(%X, %X, %d)\n", addr, value, pixel);

    int newPos = changeCount++;

    // Add new entry
    assert(newPos < 256);
    changeHistory[newPos].addr = addr;
    changeHistory[newPos].value = value;
    changeHistory[newPos].pixel = pixel;

    // Keep the list sorted
    while (newPos > 0 && changeHistory[newPos].pixel < changeHistory[newPos - 1].pixel) {

        swap(changeHistory[newPos], changeHistory[newPos - 1]);
        newPos--;
    }
}

void
PixelEngine::applyRegisterChange(const RegisterChange &change)
{
    switch (change.addr) {

        case BPLCON0:

            denise->bplcon0 = change.value;
            ham = denise->ham();
            break;

        default:

            // It must be a color register then
            assert(change.addr >= 0x180 && change.addr <= 0x1BE);

            // debug("Changing color reg %d to %X\n", (change.addr - 0x180) >> 1, change.value);
            setColor((change.addr - 0x180) >> 1, change.value);
            break;
    }
}

void
PixelEngine::translateToRGBA(uint8_t *src, int *dest)
{
    int pixel = 0;

    // Iterate over all recorded register changes
    for (int i = 0; i < changeCount; i++) {

        RegisterChange &change = changeHistory[i];

        // Draw pixels until the next change happens
        if (denise->dbplf()) {
            drawDP(src, dest, pixel, change.pixel); // Dual playfield mode
        } else {
            drawSP(src, dest, pixel, change.pixel); // Single playfield mode
        }

        pixel = change.pixel;

        // Perform the register change
        applyRegisterChange(change);
    }

    // Draw the rest of the line
    if (denise->dbplf()) {
        drawDP(src, dest, pixel, HPIXELS); // Dual playfield mode
    } else {
        drawSP(src, dest, pixel, HPIXELS); // Single playfield mode
    }

    /*
    for (; pixel <= LAST_PIXEL; pixel++) {

        assert(isColorTableIndex(src[pixel]));
        dest[pixel] = rgba[colors[src[pixel]]];
        src[pixel] = 0;
    }
    */

    // Wipe out the HBLANK area
    for (pixel = 4 * 0x0F; pixel <= 4 * 0x35; pixel++) {
        dest[pixel] = 0x00444444;
    }

    // Delete all recorded register changes
    changeCount = 0;
}

void
PixelEngine::drawSP(uint8_t *src, int *dest, int from, int to)
{
    for (int i = from; i < to; i++) {

        assert(isColorTableIndex(src[i]));
        dest[i] = rgba[colors[src[i]]];
        src[i] = 0;
    }
}

void
PixelEngine::drawDP(uint8_t *src, int *dest, int from, int to)
{
    // Determine playfield priority
    bool pf2pri = denise->PF2PRI();

    for (int i = from; i < to; i++) {

        uint8_t s = src[i];

        /* BPU | Planes in playfield 1 | Planes in playfield 2
         * ---------------------------------------------------
         *  1  | Plane 1               | none
         *  2  | Plane 1               | Plane 2
         *  3  | Plane 1, 3            | Plane 2
         *  4  | Plane 1, 3            | Plane 2, 4
         *  5  | Plane 1, 3, 5         | Plane 2, 4
         *  6  | Plane 1, 3, 5         | Plane 2, 4, 6
         */

        // Determine color indices for both playfields
        uint8_t index1 = ((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2);
        uint8_t index2 = ((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3);

        // debug("indices %d %d\n", index1, index2);
        assert(index1 < 8);
        assert(index2 < 8);

        // If not transparent, PF2 uses color registers 9 and above
        if (index2) index2 |= 0b1000;

        // Determine final index according to playfield priority
        uint8_t index;
        if (pf2pri) {
            index = index2 ? index2 : index1;
        } else {
            index = index1 ? index1 : index2;
        }

        // Draw the pixel
        assert(isColorTableIndex(index));
        dest[i] = rgba[colors[index]];
        src[i] = 0;
    }
}

void
PixelEngine::translateToRGBA_HAM(uint8_t *src, int *dest)
{
    int pixel = 0;

    // Initialize the HAM color storage with the background color
    hamRGB = colors[0];

    // Process recorded color changes
    for (int change = 0; change < changeCount; change++) {

        // Draw a chunk of pixels
        for (; pixel < changeHistory[change].pixel; pixel++) {
            dest[pixel] = rgba[computeHAM(src[pixel])];
            src[pixel] = 0;
        }

        // Perform the color change
        setColor(changeHistory[change].addr, changeHistory[change].value);
    }

    // Draw the rest of the line
    for (; pixel <= LAST_PIXEL; pixel++) {
        dest[pixel] = rgba[computeHAM(src[pixel])];
        src[pixel] = 0;
    }

    changeCount = 0;
}
