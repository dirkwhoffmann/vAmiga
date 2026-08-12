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

#include <algorithm>
#include <cmath>
#include <fstream>

namespace vamiga {

namespace {

/* Amiga video output targeted CRT displays with a gamma of roughly 2.8,
 * while host displays are calibrated for the sRGB-ish standard gamma of
 * 2.2. GAMMA_IN undoes the former to obtain linear light, GAMMA_OUT
 * re-applies the latter to encode the result for the host display.
 */
constexpr double GAMMA_IN = 2.8;
constexpr double GAMMA_OUT = 2.2;

}

void
PixelEngine::clearAll()
{
    // Wipe out all textures
    for (isize i = 0; i < NUM_TEXTURES; i++) emuTexture[i].clear();
}

void
PixelEngine::_dump(Category category, std::ostream &os) const
{
    if (category == Category::Config) {

        dumpConfig(os);
    }
}

void
PixelEngine::_initialize()
{
    // Setup the ECS BRDRBLNK color (BORDER_BG is mirrored in updateRGBA)
    borderPalette[BORDER_BLNK] = TEXEL(GpuColor(0x00, 0x00, 0x00).rawValue);

    // Setup the border debug color
    borderPalette[BORDER_DEBUG] = TEXEL(GpuColor(0xD0, 0x00, 0x00).rawValue);
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
PixelEngine::setColor(isize reg, u16 value, bool loct)
{
    assert(reg < 256);

    AmigaColor newColor = color[reg];
    
    if (denise.isAGA()) {
        
        // In AGA, a write with LOCT cleared stores the given nibble in both
        // halves of each component. A write with LOCT set replaces the lower
        // nibbles and preserves the upper ones.
        
        if (!loct) newColor.setHiNibbles(value);
        newColor.setLoNibbles(value);
        
    } else {
        
        newColor.setHiNibbles(value);
        newColor.setLoNibbles(0);
    }
    
    setColor(reg, newColor);
}

u16
PixelEngine::getSpriteColor(isize s, isize nr) const
{
    return getColor(denise.sprBase(s) + nr + 2 * (s & 6));
}

void
PixelEngine::setColor(isize reg, AmigaColor value)
{
    if (color[reg] != value) {
        
        // Update color register
        color[reg] = value;
        
       //  printf("*** color[%ld] = %02X %02X %02X\n", reg, color[reg].r, color[reg].g, color[reg].b);

        // Update palette entry
        updateRGBA(reg);
        
        // Update halfbright palette entry if needed
        if (reg < 32) updateRGBA(reg + 32);
    }
}

void
PixelEngine::updateRGBA(isize nr)
{
    assert(nr < 256);
 
    if (nr >= 32 && nr < 64 && ehbMode()) {

        palette[nr] = toTexel(color[nr - 32].ehb());
        return;
    }
    
    palette[nr] = toTexel(color[nr]);

    // Keep the background border color in sync with color register 0
    if (nr == 0) borderPalette[BORDER_BG] = palette[0];
}

void
PixelEngine::updateRGBA()
{
    // Recompute the adjustment coefficients
    updateAdjLut();
    
    // Update all cached RGBA values
    for (isize i = 0; i < 256; i++) updateRGBA(i); // setColor(i, color[i].getHiNibbles());
}

void
PixelEngine::updateEHB()
{
    // TODO: This function is called frequently. Inline to speed it up
    for (isize i = 32; i < 64; ++i) updateRGBA(i);
}

bool
PixelEngine::hamMode() const
{
    return Denise::ham(bplcon0);
}

bool
PixelEngine::hamMode8() const
{
    return Denise::ham8(bplcon0);
}

bool
PixelEngine::shresMode() const
{
    return Denise::shres(bplcon0);
}

bool
PixelEngine::ehbMode() const
{
    switch (denise.getConfig().revision) {

        case DeniseRev::OCS:

            return true;

        case DeniseRev::ECS:

            return !Denise::killehb(bplcon2);

        default: // AGA

            return !Denise::killehb(bplcon2) && (bplcon0 & 0x7010) == 0x6000;
    }
}

Texel
PixelEngine::toTexel(const AmigaColor c) const
{
    // Clamps a 16.16 fixed-point linear-light value to an 8-bit range, then
    // re-encodes it into the (non-linear) color space of the host display
    auto clamp = [this](i32 v) {
        u8 lin = u8(v < 0 ? 0 : v > (255 << 16) ? 255 : v >> 16);
        return gammaLut[lin];
    };

    isize r8 = isize(c.r);
    isize g8 = isize(c.g);
    isize b8 = isize(c.b);

    u8 r = clamp(adjLut[0 * 256 + r8] + adjLut[1 * 256 + g8] + adjLut[2 * 256 + b8]);
    u8 g = clamp(adjLut[3 * 256 + r8] + adjLut[4 * 256 + g8] + adjLut[5 * 256 + b8]);
    u8 b = clamp(adjLut[6 * 256 + r8] + adjLut[7 * 256 + g8] + adjLut[8 * 256 + b8]);

    return TEXEL(HI_HI_LO_LO(0xFF, b, g, r));
}

void
PixelEngine::updateAdjLut()
{
    auto palette = monitor.getConfig().palette;

    if (palette == Palette::RGB) {

        std::memset(adjLut, 0, sizeof(adjLut));

        for (isize i = 0; i < 256; i++) {

            auto value = i32(std::round(double(i) * 65536.0));
            adjLut[0 * 256 + i] = value; // (0,0)
            adjLut[4 * 256 + i] = value; // (1,1)
            adjLut[8 * 256 + i] = value; // (2,2)
            gammaLut[i] = u8(i);
        }

        return;
    }

    // Normalize adjustment parameters
    
    double brightness = double(monitor.getConfig().brightness) - 50.0;
    double contrast = double(monitor.getConfig().contrast) / 100.0;
    double saturation = double(monitor.getConfig().saturation) / 50.0;

    // Coefficients of the RGB to YUV conversion. The luminance always depends
    // on the input color, whereas the chrominance is replaced by a constant
    // for the monochrome palettes.
    
    double y[3] = { 0.299 * contrast, 0.587 * contrast, 0.114 * contrast };
    double u[3] = { 0.0, 0.0, 0.0 };
    double v[3] = { 0.0, 0.0, 0.0 };
    double u0 = 0.0, v0 = 0.0;

    switch (palette) {
            
        case Palette::BLACK_WHITE:
            
            break;
            
        case Palette::PAPER_WHITE:
            
            u0 = -128.0 + 120.0;
            v0 = -128.0 + 133.0;
            break;
            
        case Palette::GREEN:
            
            u0 = -128.0 + 29.0;
            v0 = -128.0 + 64.0;
            break;
            
        case Palette::AMBER:
            
            u0 = -128.0 + 24.0;
            v0 = -128.0 + 178.0;
            break;
            
        case Palette::SEPIA:
            
            u0 = -128.0 + 97.0;
            v0 = -128.0 + 154.0;
            break;
            
        default:
            
            assert(palette == Palette::COLOR);
            double s = saturation * contrast;
            u[0] = -0.147 * s; u[1] = -0.289 * s; u[2] =  0.436 * s;
            v[0] =  0.615 * s; v[1] = -0.515 * s; v[2] = -0.100 * s;
            break;
    }

    // Convert YUV back to RGB, which yields the affine transformation
    
    double m[3][3], off[3];

    for (isize i = 0; i < 3; i++) {

        m[0][i] = y[i]                + 1.140 * v[i];
        m[1][i] = y[i] - 0.396 * u[i] - 0.581 * v[i];
        m[2][i] = y[i] + 2.029 * u[i];
    }
    off[0] = brightness                + 1.140 * v0;
    off[1] = brightness - 0.396 * u0   - 0.581 * v0;
    off[2] = brightness + 2.029 * u0;

    // Tabulate the transformation in 16.16 fixed point format
    // The constant part is folded into the table of the red input

    for (isize out = 0; out < 3; out++) {
        for (isize in = 0; in < 3; in++) {
            for (isize val = 0; val < 256; val++) {

                // Linearize the input value (undo the Amiga's assumed CRT
                // gamma) before feeding it into the affine transformation,
                // so brightness, contrast and saturation are applied to
                // linear light rather than to gamma-encoded values
                
                double linear = std::pow(double(val) / 255.0, GAMMA_IN) * 255.0;
                auto adjIdx = (out * 3 + in) * 256;
                double t = m[out][in] * linear + (in == 0 ? off[out] : 0.0);
                adjLut[adjIdx + val] = i32(std::round(t * 65536.0));
            }
        }
    }

    // Tabulate the re-encoding curve that converts the clamped linear-light
    // result back into the color space expected by the host display
    
    for (isize i = 0; i < 256; i++) {

        double linear = std::clamp(double(i) / 255.0, 0.0, 1.0);
        double display = std::pow(linear, 1.0 / GAMMA_OUT) * 255.0;
        gammaLut[i] = u8(std::round(std::clamp(display, 0.0, 255.0)));
    }
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

            bplcon0 = change.value;
            updateEHB();
            break;

        case Reg::BPLCON2:

            bplcon2 = change.value;
            updateEHB();
            break;

        default:
            
            // It must be a color register then
            setColor(BYTE1(isize(change.reg)), change.value, !!BYTE2(isize(change.reg)));
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
        if (shresMode()) {
            colorizeSHRES(dst, pixel, trigger);
        } else if (hamMode8()) {
            colorizeHAM8(dst, pixel, trigger, hold);
        } else if (hamMode()) {
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
    
    // Let sprite shine through the border if enabled (AGA only)
    if (denise.borderSprites()) removeBorderOverSprites(from, to);
            
    // Colorize pixels
    for (Pixel i = from; i < to; i++)
        dst[i] = bbuf[i] == BORDER_NONE ? palette[mbuf[i]] : borderPalette[bbuf[i]];
}

void
PixelEngine::colorizeSHRES(Texel *dst, Pixel from, Pixel to)
{
    auto *mbuf = denise.mBuffer;
    auto *bbuf = denise.bBuffer;
    auto *zbuf = denise.zBuffer;

    // Let sprite shine through the border if enabled (AGA only)
    if (denise.borderSprites()) removeBorderOverSprites(from, to);

    if constexpr (sizeof(Texel) == 4) {

        // Output two super-hires pixels as a single texel
        for (Pixel i = from; i < to; i++) {
            dst[i] = bbuf[i] == BORDER_NONE ? palette[mbuf[i]] : borderPalette[bbuf[i]];
        }

    } else {

        // Output each super-hires pixel as a seperate texel
        for (Pixel i = from; i < to; i++) {

            u32 *p = (u32 *)(dst + i);

            if (bbuf[i] != BORDER_NONE) {

                p[0] =
                p[1] = u32(borderPalette[bbuf[i]]);

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

    // Let sprite shine through the border if enabled (AGA only)
    if (denise.borderSprites()) removeBorderOverSprites(from, to);

    for (Pixel i = from; i < to; i++) {

        // Check for border pixels
        if (bbuf[i] != BORDER_NONE) {

            dst[i] = borderPalette[bbuf[i]];

            // Only the background code corresponds to a real color register
            if (bbuf[i] == BORDER_BG) ham = color[0];
            continue;
        }

        u8 index = ibuf[i];
        assert(isPaletteIndex(index));

        switch ((dbuf[i] >> 4) & 0b11) {

            case 0b00: // Get color from register

                ham = color[index];
                break;

            case 0b01: // Modify blue

                ham.b = u8((index & 0xF) << 4);
                break;

            case 0b10: // Modify red

                ham.r = u8((index & 0xF) << 4);
                break;

            case 0b11: // Modify green

                ham.g = u8((index & 0xF) << 4);
                break;
        }

        // Synthesize pixel
        if (denise.spritePixelIsVisible(i)) {
            dst[i] = palette[mbuf[i]];
        } else {
            dst[i] = toTexel(ham);
        }
    }
}

void
PixelEngine::colorizeHAM8(Texel *dst, Pixel from, Pixel to, AmigaColor& ham)
{
    auto *dbuf = denise.dBuffer;
    auto *ibuf = denise.iBuffer;
    auto *mbuf = denise.mBuffer;
    auto *bbuf = denise.bBuffer;

    // Let sprite shine through the border if enabled (AGA only)
    if (denise.borderSprites()) removeBorderOverSprites(from, to);

    for (Pixel i = from; i < to; i++) {

        // Check for border pixels
        if (bbuf[i] != BORDER_NONE) {

            dst[i] = borderPalette[bbuf[i]];

            // Only the background code corresponds to a real color register
            if (bbuf[i] == BORDER_BG) ham = color[0];
            continue;
        }

        u8 index = ibuf[i];
        assert(isPaletteIndex(index));

        // HAM8 uses the lowest two bitplanes as control bits and the
        // remaining six as data. Because a component holds eight bits,
        // the two least significant ones are left untouched.
        
        switch (dbuf[i] & 0b11) {

            case 0b00: // Get color from register

                ham = color[index >> 2];
                break;

            case 0b01: // Modify blue

                ham.b = (ibuf[i] & 0xFC) | (ham.b & 0x03);
                break;

            case 0b10: // Modify red

                ham.r = (ibuf[i] & 0xFC) | (ham.r & 0x03);
                break;

            case 0b11: // Modify green

                ham.g = (ibuf[i] & 0xFC) | (ham.g & 0x03);
                break;
        }

        // Synthesize pixel
        if (denise.spritePixelIsVisible(i)) {
            dst[i] = palette[mbuf[i]];
        } else {
            dst[i] = toTexel(ham);
        }
    }
}

void
PixelEngine::removeBorderOverSprites(Pixel from, Pixel to)
{
    auto *bbuf = denise.bBuffer;
    auto *zbuf = denise.zBuffer;
    
    for (Pixel i = from; i < to; i++) {
        if (Denise::isSpritePixel(zbuf[i])) bbuf[i] = BORDER_NONE;
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
