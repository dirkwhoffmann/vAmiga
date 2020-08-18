// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PixelEngine::PixelEngine(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("PixelEngine");

    // Allocate frame buffers
    for (int i = 0; i < 2; i++) {

        longFrame[i].data = new u32[PIXELS];
        longFrame[i].longFrame = true;

        shortFrame[i].data = new u32[PIXELS];
        shortFrame[i].longFrame = false;
    }

    // Create random background noise pattern
    const size_t noiseSize = 2 * 512 * 512;
    noise = new u32[noiseSize];
    for (int i = 0; i < noiseSize; i++) {
        noise[i] = rand() % 2 ? 0xFF000000 : 0xFFFFFFFF;
    }

    // Setup some debug colors
    indexedRgba[64] = GpuColor(0xFF, 0x00, 0x00).rawValue;
    indexedRgba[65] = GpuColor(0xD0, 0x00, 0x00).rawValue;
    indexedRgba[66] = GpuColor(0xA0, 0x00, 0x00).rawValue;
    indexedRgba[67] = GpuColor(0x90, 0x00, 0x00).rawValue;
    indexedRgba[68] = GpuColor(0x00, 0xFF, 0xFF).rawValue;
    indexedRgba[69] = GpuColor(0x00, 0xD0, 0xD0).rawValue;
    indexedRgba[70] = GpuColor(0x00, 0xA0, 0xA0).rawValue;
    indexedRgba[71] = GpuColor(0x00, 0x90, 0x90).rawValue;
}

PixelEngine::~PixelEngine()
{
    for (int i = 0; i < 2; i++) {

        delete[] longFrame[i].data;
        delete[] shortFrame[i].data;
    }

    delete[] noise;
}

void
PixelEngine::_powerOn()
{
    // Initialize frame buffers with a checkerboard debug pattern
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {

            int pos = line * HPIXELS + i;
            int col = (line / 4) % 2 == (i / 8) % 2 ? 0xFF222222 : 0xFF444444;
            longFrame[0].data[pos] = longFrame[1].data[pos] = col;
            shortFrame[0].data[pos] = shortFrame[1].data[pos] = col;
        }
    }
}

void
PixelEngine::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS

    // Initialize frame buffers
    workingLongFrame = &longFrame[0];
    workingShortFrame = &shortFrame[0];
    stableLongFrame = &longFrame[1];
    stableShortFrame = &shortFrame[1];
    frameBuffer = &longFrame[0];

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
PixelEngine::setColor(int reg, u16 value)
{
    assert(reg < 32);

    colreg[reg] = value & 0xFFF;

    u8 r = (value & 0xF00) >> 8;
    u8 g = (value & 0x0F0) >> 4;
    u8 b = (value & 0x00F);

    indexedRgba[reg] = rgba[value & 0xFFF];
    indexedRgba[reg + 32] = rgba[((r / 2) << 8) | ((g / 2) << 4) | (b / 2)];
}

void
PixelEngine::updateRGBA()
{
    // Iterate through all 4096 colors
    for (u16 col = 0x000; col <= 0xFFF; col++) {

        // Convert the Amiga color into an RGBA value
        u8 r = (col >> 4) & 0xF0;
        u8 g = (col >> 0) & 0xF0;
        u8 b = (col << 4) & 0xF0;

        // Convert the Amiga value to an RGBA value
        adjustRGB(r, g, b);

        // Write the result into the register lookup table
        rgba[col] = HI_HI_LO_LO(0xFF, b, g, r);
    }

    // Update all RGBA values that are cached in indexedRgba[]
    for (int i = 0; i < 32; i++) setColor(i, colreg[i]);
}

void
PixelEngine::adjustRGB(u8 &r, u8 &g, u8 &b)
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

    r = u8(newR);
    g = u8(newG);
    b = u8(newB);
}

bool
PixelEngine::isLongFrame(ScreenBuffer *buf)
{
    bool result = (buf == &longFrame[0] || buf == &longFrame[1]);
    assert(result == buf->longFrame);
    return result;
}

bool
PixelEngine::isShortFrame(ScreenBuffer *buf)
{
    bool result = (buf == &shortFrame[0] || buf == &shortFrame[1]);
    assert(result == !buf->longFrame);
    return result;
}

ScreenBuffer
PixelEngine::getStableLongFrame()
{
    ScreenBuffer result;
    synchronized { result = *stableLongFrame; }
    return result;
}

ScreenBuffer
PixelEngine::getStableShortFrame()
{
    ScreenBuffer result;
    synchronized { result = *stableShortFrame; }
    return result;
}

u32 *
PixelEngine::getNoise()
{
    int offset = rand() % (512 * 512);
    return noise + offset;
}

u32 *
PixelEngine::pixelAddr(int pixel)
{
    u32 offset = pixel + agnus.pos.v * HPIXELS;

    assert(pixel < HPIXELS);
    assert(offset < PIXELS);

    return frameBuffer->data + offset;
}

void
PixelEngine::beginOfFrame(bool interlace)
{     
    synchronized {
        
        if (isLongFrame(frameBuffer)) {
            
            // Declare the finished buffer stable
            swap(workingLongFrame, stableLongFrame);
            
            // Select the next buffer to work on
            frameBuffer = agnus.frame.lof ? workingLongFrame : workingShortFrame;
            
        } else {
            
            assert(isShortFrame(frameBuffer));
            
            // Declare the finished buffer stable
            swap(workingShortFrame, stableShortFrame);
            
            // Select the next buffer to work on
            frameBuffer = workingLongFrame;
        }
        
        frameBuffer->interlace = interlace;
    }
    
    dmaDebugger.vSyncHandler();
}

void
PixelEngine::endOfVBlankLine()
{
    // Apply all color register changes that happened in this line
    for (int i = colChanges.begin(); i != colChanges.end(); i = colChanges.next(i)) {
        applyRegisterChange(colChanges.elements[i]);
    }
}

void
PixelEngine::applyRegisterChange(const RegChange &change)
{
    switch (change.addr) {

        case 0:
            break;

        case BPLCON0:
            hamMode = Denise::ham(change.value);
            break;
            
        default: // It must be a color register then
            assert(change.addr >= 0x180 && change.addr <= 0x1BE);
            setColor((change.addr - 0x180) >> 1, change.value);
            break;
    }
}

void
PixelEngine::colorize(int line)
{
    // Jump to the first pixel in the specified line in the active frame buffer
    u32 *dst = frameBuffer->data + line * HPIXELS;
    int pixel = 0;

    // Initialize the HAM mode hold register with the current background color
    u16 hold = colreg[0];

    // Add a dummy register change to ensure we draw until the line end
    colChanges.insert(HPIXELS, RegChange { SET_NONE, 0 } );

    // Iterate over all recorded register changes
    for (int i = colChanges.begin(); i != colChanges.end(); i = colChanges.next(i)) {

        Cycle trigger = colChanges.keys[i];
        RegChange &change = colChanges.elements[i];

        // Colorize a chunk of pixels
        if (hamMode) {
            colorizeHAM(dst, pixel, trigger, hold);
        } else {
            colorize(dst, pixel, trigger);
        }
        pixel = trigger;

        // Perform the register change
        applyRegisterChange(change);
    }

    // Wipe out the HBLANK area
    for (int pixel = 4 * HBLANK_MIN; pixel <= 4 * HBLANK_MAX; pixel++) {
        dst[pixel] = rgbaHBlank;
    }

    // Clear the history cache
    colChanges.clear();
}

void
PixelEngine::colorize(u32 *dst, int from, int to)
{
    u8 *mbuf = denise.mBuffer;

    for (int i = from; i < to; i++) {
        dst[i] = indexedRgba[mbuf[i]];
    }
}

void
PixelEngine::colorizeHAM(u32 *dst, int from, int to, u16& ham)
{
    u8 *bbuf = denise.bBuffer;
    u8 *ibuf = denise.iBuffer;
    u8 *mbuf = denise.mBuffer;

    for (int i = from; i < to; i++) {

        u8 index = ibuf[i];
        assert(isRgbaIndex(index));

        // switch ((index >> 4) & 0b11) {
        switch ((bbuf[i] >> 4) & 0b11) {

            case 0b00: // Get color from register

                ham = colreg[index];
                break;

            case 0b01: // Modify blue

                ham &= 0xFF0;
                ham |= (index & 0b1111);
                break;

            case 0b10: // Modify red

                ham &= 0x0FF;
                ham |= (index & 0b1111) << 8;
                break;

            case 0b11: // Modify green

                ham &= 0xF0F;
                ham |= (index & 0b1111) << 4;
                break;

            default:
                assert(false);
        }

        // Synthesize pixel
        if (denise.spritePixelIsVisible(i)) {
            dst[i] = rgba[colreg[mbuf[i]]];
        } else {
            dst[i] = rgba[ham];
        }
    }
}

void
PixelEngine::hide(int line, u16 layers, u8 alpha)
{
    u32 *p = frameBuffer->data + line * HPIXELS;

    for (int i = 0; i < HPIXELS; i++) {

        u16 z = denise.zBuffer[i];

        // Check for case 1: A sprite is visible
        if (Denise::isSpritePixel(z)) {

            if (Denise::isSpritePixel<0>(z) && !(layers & 0x01)) continue;
            if (Denise::isSpritePixel<1>(z) && !(layers & 0x02)) continue;
            if (Denise::isSpritePixel<2>(z) && !(layers & 0x04)) continue;
            if (Denise::isSpritePixel<3>(z) && !(layers & 0x08)) continue;
            if (Denise::isSpritePixel<4>(z) && !(layers & 0x10)) continue;
            if (Denise::isSpritePixel<5>(z) && !(layers & 0x20)) continue;
            if (Denise::isSpritePixel<6>(z) && !(layers & 0x40)) continue;
            if (Denise::isSpritePixel<7>(z) && !(layers & 0x80)) continue;
        
        } else {

            // Check for case 2: Playfield 1 is visible
            if ((Denise::upperPlayfield(z) == 1) && !(layers & 0x100)) continue;
        
            // Check for case 3: layfield 2 is visible
            if ((Denise::upperPlayfield(z) == 2) && !(layers & 0x200)) continue;
        }
        
        u8 r = p[i] & 0xFF;
        u8 g = (p[i] >> 8) & 0xFF;
        u8 b = (p[i] >> 16) & 0xFF;

        double scale = alpha / 255.0;
        u8 bg = (line / 4) % 2 == (i / 8) % 2 ? 0x22 : 0x44;
        u8 newr = r * (1 - scale) + bg * scale;
        u8 newg = g * (1 - scale) + bg * scale;
        u8 newb = b * (1 - scale) + bg * scale;
        
        p[i] = 0xFF000000 | newb << 16 | newg << 8 | newr;
    }
}
