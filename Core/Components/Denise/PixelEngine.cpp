// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "PixelEngine.h"
#include "Amiga.h"
#include "Colors.h"
#include "Denise.h"
#include "DmaDebugger.h"
#include "Emulator.h"

#include <fstream>

namespace vamiga {

void
PixelEngine::clearAll()
{
    // Wipe out all textures
    for (isize i = 0; i < NUM_TEXTURES; i++) emuTexture[i].clear();
}

void
PixelEngine::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    if (category == Category::Config) {

        dumpConfig(os);
    }
}

void
PixelEngine::_initialize()
{
    // Setup ECS BRDRBLNK color
    palette[64] = TEXEL(GpuColor(0x00, 0x00, 0x00).rawValue);
    
    // Setup debug colors
    palette[65] = TEXEL(GpuColor(0xD0, 0x00, 0x00).rawValue);
    palette[66] = TEXEL(GpuColor(0xA0, 0x00, 0x00).rawValue);
    palette[67] = TEXEL(GpuColor(0x90, 0x00, 0x00).rawValue);
}

void
PixelEngine::_didReset(bool hard)
{
    if (hard) {
        
        for (isize i = 0; i < NUM_TEXTURES; i++) {
            
            emuTexture[i].nr = 0;
            emuTexture[i].lof = emuTexture[i].prevlof = true;
        }
    }

    activeBuffer = 0;
    updateRGBA();
}

void
PixelEngine::_didLoad()
{
    clearAll();
    updateRGBA();
}

void
PixelEngine::_powerOn()
{
    clearAll();
}

void
PixelEngine::setColor(isize reg, u16 value)
{
    assert(reg < 32);

    AmigaColor newColor(value & 0xFFF);

    color[reg] = newColor;

    // Update standard palette entry
    palette[reg] = colorSpace[value & 0xFFF];

    // Update halfbright palette entry
    palette[reg + 32] = colorSpace[newColor.ehb().rawValue()];
}

void
PixelEngine::updateRGBA()
{
    // Iterate through all 4096 colors
    for (u16 col = 0x000; col <= 0xFFF; col++) {

        u8 r = (col >> 4) & 0xF0;
        u8 g = (col >> 0) & 0xF0;
        u8 b = (col << 4) & 0xF0;

        // Adjust the RBG values according to the current video settings
        adjustRGB(r, g, b);

        // Write the result into the register lookup table
        colorSpace[col] = TEXEL(HI_HI_LO_LO(0xFF, b, g, r));
    }

    // Update all cached RGBA values
    for (isize i = 0; i < 32; i++) setColor(i, color[i].rawValue());
}

void
PixelEngine::adjustRGB(u8 &r, u8 &g, u8 &b)
{
    auto palette = monitor.getConfig().palette;

    // The RGB palette does not alter anything. Return immediately
    if (palette == Palette::RGB) return;
    
    // Normalize adjustment parameters
    double brightness =  (monitor.getConfig().brightness - 50.0);
    double contrast = monitor.getConfig().contrast / 100.0;
    double saturation = monitor.getConfig().saturation / 50.0;

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

        case Palette::BLACK_WHITE:
            u = 0.0;
            v = 0.0;
            break;

        case Palette::PAPER_WHITE:
            u = -128.0 + 120.0;
            v = -128.0 + 133.0;
            break;

        case Palette::GREEN:
            u = -128.0 + 29.0;
            v = -128.0 + 64.0;
            break;

        case Palette::AMBER:
            u = -128.0 + 24.0;
            v = -128.0 + 178.0;
            break;

        case Palette::SEPIA:
            u = -128.0 + 97.0;
            v = -128.0 + 154.0;
            break;

        default:
            assert(palette == Palette::COLOR);
    }

    // Convert YUV to RGB
    double newR = y             + 1.140 * v;
    double newG = y - 0.396 * u - 0.581 * v;
    double newB = y + 2.029 * u;
    newR = std::max(std::min(newR, 255.0), 0.0);
    newG = std::max(std::min(newG, 255.0), 0.0);
    newB = std::max(std::min(newB, 255.0), 0.0);

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

const Texture &
PixelEngine::getStableBuffer(isize offset) const
{
    auto nr = activeBuffer + offset - 1;
    return emuTexture[(nr + NUM_TEXTURES) % NUM_TEXTURES];
}

Texture &
PixelEngine::getWorkingBuffer()
{
    return emuTexture[activeBuffer];
}

Texel *
PixelEngine::workingPtr(isize row, isize col)
{
    assert(row >= 0 && row <= VPOS_MAX);
    assert(col >= 0 && col <= HPOS_MAX);

    return getWorkingBuffer().pixels.ptr + row * HPIXELS + col;
}

Texel *
PixelEngine::stablePtr(isize row, isize col)
{
    assert(row >= 0 && row <= VPOS_MAX);
    assert(col >= 0 && col <= HPOS_MAX);

    return getStableBuffer().pixels.ptr + row * HPIXELS + col;
}

void
PixelEngine::swapBuffers()
{
    emulator.lockTexture();

    videoPort.buffersWillSwap();

    isize oldActiveBuffer = activeBuffer;
    isize newActiveBuffer = (activeBuffer + 1) % NUM_TEXTURES;

    emuTexture[newActiveBuffer].nr = agnus.pos.frame;
    emuTexture[newActiveBuffer].lof = agnus.pos.lof;
    emuTexture[newActiveBuffer].prevlof = emuTexture[oldActiveBuffer].lof;

    activeBuffer = newActiveBuffer;

    emulator.unlockTexture();
}

void
PixelEngine::vsyncHandler()
{
    dmaDebugger.vSyncHandler();
}

void
PixelEngine::eofHandler()
{
    dmaDebugger.eofHandler();
}

void
PixelEngine::replayColRegChanges()
{
    // Apply all color register changes that happened in this line
    for (isize i = 0, end = colChanges.end(); i < end; i++) {
        applyRegisterChange(colChanges.elements[i]);
    }
    colChanges.clear();
}

void
PixelEngine::applyRegisterChange(const RegChange &change)
{
    switch (change.reg) {

        case Reg(0):
            
            break;

        case Reg::BPLCON0:

            hamMode = Denise::ham(change.value);
            shresMode = Denise::shres(change.value);
            break;
            
        default: // It must be a color register then
            
            auto nr = isize(change.reg) - isize(Reg::COLOR00);
            assert(0 <= nr && nr < 32);

            if (color[nr].rawValue() != change.value) {
                setColor(nr, change.value);
            }
            break;
    }
}

void
PixelEngine::colorize(isize line)
{
    // Jump to the first pixel in the specified line in the active frame buffer
    auto *dst = workingPtr(line);
    Pixel pixel = 0;

    // Initialize the HAM mode hold register with the current background color
    AmigaColor hold = color[0];

    // Add a dummy register change to ensure we draw until the line end
    colChanges.insert(HPIXELS, RegChange { .reg = Reg(0), .value = 0 } );

    // Iterate over all recorded register changes
    for (isize i = 0, end = colChanges.end(); i < end; i++) {

        Pixel trigger = (Pixel)colChanges.keys[i];
        RegChange &change = colChanges.elements[i];

        // Colorize a chunk of pixels
        if (shresMode) {
            colorizeSHRES(dst, pixel, trigger);
        } else if (hamMode) {
            colorizeHAM(dst, pixel, trigger, hold);
        } else {
            colorize(dst, pixel, trigger);
        }
        pixel = trigger;

        // Perform the register change
        applyRegisterChange(change);
    }

    // Clear the history cache
    colChanges.clear();

    // Wipe out the HBLANK area
    auto start = agnus.pos.pixel(HBLANK_MIN);
    auto stop  = agnus.pos.pixel(HBLANK_MAX);
    for (pixel = start; pixel <= stop; pixel++) dst[pixel] = Texture::hblank;
}

void
PixelEngine::colorize(Texel *dst, Pixel from, Pixel to)
{
    auto *mbuf = denise.mBuffer;
    auto *bbuf = denise.bBuffer;

    /*
    for (Pixel i = from; i < to; i++) {
        dst[i] = palette[mbuf[i]];
    }
    */
    for (Pixel i = from; i < to; i++) {
        dst[i] = palette[bbuf[i] == 0xFF ? mbuf[i] : bbuf[i]];
    }
}

void
PixelEngine::colorizeSHRES(Texel *dst, Pixel from, Pixel to)
{
    auto *mbuf = denise.mBuffer;
    auto *bbuf = denise.bBuffer;
    auto *zbuf = denise.zBuffer;

    if constexpr (sizeof(Texel) == 4) {

        // Output two super-hires pixels as a single texel
        for (Pixel i = from; i < to; i++) {
            dst[i] = palette[bbuf[i] == 0xFF ? mbuf[i] : bbuf[i]];
        }

    } else {

        // Output each super-hires pixel as a seperate texel
        for (Pixel i = from; i < to; i++) {

            u32 *p = (u32 *)(dst + i);

            if (bbuf[i] != 0xFF) {

                p[0] =
                p[1] = u32(palette[bbuf[i]]);

            } else if (Denise::isSpritePixel(zbuf[i])) {

                p[0] =
                p[1] = u32(palette[mbuf[i]]);

            } else {

                p[0] = u32(palette[mbuf[i] >> 2]);
                p[1] = u32(palette[mbuf[i] & 3]);
            }
        }
    }
}

void
PixelEngine::colorizeHAM(Texel *dst, Pixel from, Pixel to, AmigaColor& ham)
{
    auto *dbuf = denise.dBuffer;
    auto *ibuf = denise.iBuffer;
    auto *mbuf = denise.mBuffer;
    auto *bbuf = denise.bBuffer;

    for (Pixel i = from; i < to; i++) {

        // Check for border pixels
        if (bbuf[i] != 0xFF) {

            dst[i] = palette[bbuf[i]];
            ham = color[bbuf[i]];
            continue;
        }

        u8 index = ibuf[i];
        assert(isPaletteIndex(index));

        switch ((dbuf[i] >> 4) & 0b11) {

            case 0b00: // Get color from register

                ham = color[index];
                break;

            case 0b01: // Modify blue

                ham.b = index & 0xF;
                break;

            case 0b10: // Modify red

                ham.r = index & 0xF;
                break;

            case 0b11: // Modify green

                ham.g = index & 0xF;
                break;

            default:
                fatalError;
        }

        // Synthesize pixel
        if (denise.spritePixelIsVisible(i)) {
            dst[i] = palette[mbuf[i]];
        } else {
            dst[i] = colorSpace[ham.rawValue()];
        }
    }
}

void
PixelEngine::hide(isize line, u16 layers, u8 alpha)
{
    auto *p = workingPtr(line);

    for (Pixel i = 0; i < HPIXELS; i++) {

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
        u8 newr = (u8)(r * (1 - scale) + bg * scale);
        u8 newg = (u8)(g * (1 - scale) + bg * scale);
        u8 newb = (u8)(b * (1 - scale) + bg * scale);
        
        p[i] = 0xFF000000 | newb << 16 | newg << 8 | newr;
    }
}

}
