// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "ChangeRecorder.h"
#include "Constants.h"
#include "HostTypes.h"
#include "Texture.h"
#include "utl/concurrency.h"
#include "utl/support/Bits.h"

namespace vamiga {

class PixelEngine final : public SubComponent {

    Descriptions descriptions = {{

        .type           = Class::PixelEngine,
        .name           = "PixelEngine",
        .description    = "Amiga Monitor",
        .shell          = "monitor"
    }};

    Options options = {

    };

    friend class Denise;


    //
    // Screen buffers
    //

private:

    static constexpr isize NUM_TEXTURES = 8;
    
    /* The emulator manages textures in a ring buffer to allow access to older
     * frames ("run-behind" feature). At any time, one texture serves as the
     * working buffer, where all drawing functions write, while the other
     * textures are considered stable. Once a frame is completed, the next
     * texture in the ring becomes the new working buffer.
     */
    Texture emuTexture[NUM_TEXTURES];

    /* Parallel ring buffer holding the DMA debugger's raw, unblended
     * per-channel visualization (see DmaDebugger::computeOverlay). Kept
     * separate from emuTexture so the Layers inspector's preview can show
     * DMA usage on its own, independent of whether it is also blended into
     * the real picture (DMA_DEBUG_OVERLAY) -- mirrors VICII's own
     * emuTexture/dmaTexture split in the C64 core. Indexed by the same
     * activeBuffer as emuTexture, so both stay in lockstep.
     */
    Texture dmaTexture[NUM_TEXTURES];

    // The currently active buffer
    isize activeBuffer = 0;

    // Mutex for synchronizing access to the stable buffer
    Mutex bufferMutex;

    
    //
    // Color management
    //

private:
    
    /* The monitor adjustment is an affine transformation of the RGB components:
     *
     *     output = M * input + offset
     *
     * where input and output are RGB vectors. Since the transformation is linear
     * in each input component, it can be tabulated efficiently. Instead of storing
     * a LUT for all possible colors (which would be impractical because AGA colors
     * span the full 24-bit range), the contribution of each input component to each
     * output component is stored separately:
     *
     *     R' = lut(R->R) + lut(G->R) + lut(B->R)
     *     G' = lut(R->G) + lut(G->G) + lut(B->G)
     *     B' = lut(R->B) + lut(G->B) + lut(B->B)
     *
     * The table is indexed by adjIdx(out, in) + value, where out selects the
     * output component, in selects the input component, and value is the 8-bit
     * input component value. Entries are stored in 16.16 fixed-point format.
     *
     * The constant offset of the affine transformation is folded into the tables
     * of the red input component to avoid an additional addition during rendering.
     */
    i32 adjLut[9 * 256];

    /* Gamma re-encoding table. adjLut is tabulated in a linearized color
     * space (see updateAdjLut()), so the affine transformation above yields
     * a linear-light result. This table converts that clamped 8-bit linear
     * value back into the (non-linear) color space expected by the host
     * display, completing the linearize -> adjust -> re-encode pipeline.
     */
    u8 gammaLut[256];


    // Color register colors
    AmigaColor color[256];

    // ABGR values of the 256 color registers
    static const int paletteCnt = 256;
    Texel palette[paletteCnt];

    /* ABGR values for the special border codes stored in Denise's bBuffer
     * (see the BORDER_xxx constants below). Indexed directly by the code, so
     * BORDER_NONE's slot is unused. Kept separate from palette[] because
     * these colors don't correspond to a color register: BORDER_BLNK and
     * BORDER_DEBUG have no register at all, and BORDER_BG is just a mirror
     * of palette[0], refreshed whenever register 0 changes (see updateRGBA).
     */
    Texel borderPalette[4];

    /* ABGR values for the two sub-pixels of an ECS super hires pixel (see
     * AmigaColor::shresHi and shresLo). Only the first 16 registers are
     * reachable in that mode, so the tables stop there. Kept precomputed
     * because colorizeShres would otherwise redo the bit twiddling and the
     * monitor adjustment for every sub-pixel of every line.
     */
    Texel shresPalette[2][16];

    /* ABGR values for a blended super hires pixel pair, indexed by the color
     * index of the first sub-pixel and that of the second (see the shresBlend
     * option). The two are averaged in Amiga color space rather than in the
     * host's, because what smears the sub-pixels together on real hardware is
     * the monitor's video amplifier, which low-passes the signal before the
     * phosphor's non-linearity, not after it. Averaging the finished texels
     * instead would make a $5,$5 pair noticeably darker than a $0,$A one, and
     * the A500+ shows them as equal (Denise/Modes/shres/shramp1 to shramp7).
     */
    Texel shresPaletteBlend[16][16];

    // Indicates that shresPaletteBlend is out of date (see updateShresBlend)
    bool shresBlendDirty = true;


    /* Snapshots of BPLCON0 and BPLCON2 as of the most recently replayed
     * register change (see applyRegisterChange). All video-mode decisions
     * that can change mid-rasterline (HAM, HAM8, SHRES, EHB) are computed
     * from these two snapshots instead of Denise's live registers, since the
     * live registers already hold the end-of-line value by the time a line
     * is colorized and would give wrong answers for earlier register changes
     * replayed within the same line.
     */
    u16 bplcon0;
    u16 bplcon2;


    //
    // Register change history buffer
    //

public:

    // Color register history
    RegChangeRecorder<1024> colChanges;


    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    // Initializes both frame buffers with a checkerboard pattern
    void clearAll();

    PixelEngine& operator= (const PixelEngine& other) {

        CLONE_ARRAY(adjLut)
        CLONE_ARRAY(gammaLut)
        CLONE(colChanges)
        CLONE_ARRAY(color)
        CLONE(bplcon0)
        CLONE(bplcon2)
        CLONE_ARRAY(palette)
        CLONE_ARRAY(borderPalette)

        return *this;
    }


    //
    // Methods from Serializable
    //

private:

    template <class T>
    void serialize(T& worker)
    {
        worker

        << colChanges
        << color
        << bplcon0
        << bplcon2;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override;
    void _initialize() override;
    void _powerOn() override;
    void _didLoad() override;
    void _didReset(bool hard) override;

    
    //
    // Methods from Configurable
    //

public:
    
    const Options &getOptions() const override { return options; }


    //
    // Accessing color registers
    //

public:

    // Performs a consistency check for debugging
    static bool isPaletteIndex(isize nr) { return nr < paletteCnt; }

    /* Codes stored in Denise's border pixel buffer (bBuffer), also used to
     * index borderPalette[] directly.
     */
    static constexpr u8 BORDER_NONE  = 0; // Not a border pixel
    static constexpr u8 BORDER_BG    = 1; // Border drawn in the background color (register 0)
    static constexpr u8 BORDER_BLNK  = 2; // Border drawn in pure black (ECS BRDRBLNK)
    static constexpr u8 BORDER_DEBUG = 3; // Border drawn in the debug color

    // Changes one of the 32 Amiga color registers
    void setColor(isize reg, u16 value, bool loct);
    
    // Returns a color value in Amiga format
    u16 getColor(isize nr) const { return color[nr].getHiNibbles(); }

    // Returns sprite color in Amiga format
    u16 getSpriteColor(isize s, isize nr) const;

private:

    void setColor(isize reg, AmigaColor value);


    //
    // Derived video modes
    //

private:

    // Computed from the tracked bplcon0 / bplcon2 snapshots
    bool hamMode6() const;
    bool hamMode8() const;
    bool shresMode() const;
    bool shresModeEcs() const;
    bool ehbMode() const;


    //
    // Using the color lookup table
    //

public:

    // Updates a single entry in the RGBA lookup table
    void updateRGBA(isize nr);
    
    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Updates the EHB range in the RGBA lookup table
    void updateEHB();

private:

    // Rebuilds shresPaletteBlend if a register below 16 has changed
    void updateShresBlend();

    /* Color index of an ECS super hires pixel. Super hires pixels are paired
     * up within aligned groups of four (one lores pixel), pixel 0 with pixel 2
     * and pixel 1 with pixel 3, so both members of a pair are always fetched
     * together and a pair never reaches beyond the bitplane data. See the
     * comment in colorizeShres.
     */
    static isize shresIndex(const u8 *mbuf, Pixel k) {

        Pixel base = (k & ~3) | (k & 1);
        return ((mbuf[base + 2] & 3) * 4) + (mbuf[base] & 3);
    }

public:
    
    // Converts an Amiga color into a texel, applying the monitor settings
    Texel toTexel(const AmigaColor c) const;

    /* Converts to and from a GpuColor<F>. Since GpuColor<F>'s rawValue is
     * already packed exactly as a texel of format F (see Colors.h), this is
     * now just a bitcast plus the TEXEL sub-pixel duplication -- there is no
     * format-dependent shuffling left to do here at all, which is the point:
     * the old fixed-layout GpuColor needed a matching pair of conversions at
     * every read/write of an actual pixel buffer, and a mismatch there (or a
     * caller that skipped them) silently mixed the wrong channel whenever
     * the host wasn't running in ABGR. With F carried in the type, a
     * GpuColor<F> and a Texel produced under HOST_TEX_FORMAT == F agree by
     * construction.
     *
     * F is still a template parameter (rather than each Texel simply
     * knowing its own format at runtime) because these run in per-pixel hot
     * paths (DmaDebugger::computeOverlay runs this across every visible
     * pixel of every scanline): callers should switch on
     * host.getConfig().texFormat once, not per pixel, and call the matching
     * instantiation from there on.
     */
    template <TexelFormat F> static constexpr GpuColor<F>
    fromTexel(Texel t)
    {
        return GpuColor<F>(u32(t));
    }

    template <TexelFormat F> static constexpr Texel
    toTexel(GpuColor<F> c)
    {
        return TEXEL(c.rawValue);
    }

    // One-off, runtime-dispatched construction of a GpuColor for whatever
    // the host's current HOST_TEX_FORMAT happens to be, converted straight
    // to a Texel -- for setup code that isn't a per-pixel hot path (e.g. the
    // border debug color below). Prefer the templated overloads above on a
    // hot path.
    Texel toTexel(u8 r, u8 g, u8 b, u8 a = 0xFF) const;

private:
    
    // Recomputes the color adjustment tables from the monitor settings
    void updateAdjLut();
    

    //
    // Working with frame buffers
    //

public:

    // Returns the working buffer or the stable buffer
    Texture &getWorkingBuffer();
    const Texture &getStableBuffer(isize offset = 0) const;

    // Return a pointer into the pixel storage
    Texel *workingPtr(isize row = 0, isize col = 0);
    Texel *stablePtr(isize row = 0, isize col = 0);

    // Same as above, but for the DMA debugger's own texture (see dmaTexture)
    Texture &getWorkingDmaBuffer();
    const Texture &getStableDmaBuffer(isize offset = 0) const;
    Texel *dmaWorkingPtr(isize row = 0, isize col = 0);

    // Swaps the working buffer and the stable buffer
    void swapBuffers();
    
    // Called after each frame to switch the frame buffers
    void vsyncHandler();

    // Called at the end of each frame
    void eofHandler();

    //
    // Working with recorded register changes
    //

public:

    // Applies all recorded color register changes
    void replayColRegChanges();

    // Applies a single register change
    void applyRegisterChange(const RegChange &change);


    //
    // Synthesizing pixels
    //

public:
    
    /* Colorizes a rasterline. This function implements the last stage in the
     * graphics pipelile. It translates a line of color register indices into a
     * line of RGBA values in GPU format.
     */
    void colorize(isize line);
    
private:
    
    void colorize(u32 *dst, Pixel from, Pixel to);
    void colorizeShres(u32 *dst, Pixel from, Pixel to);
    void colorizeHAM(u32 *dst, Pixel from, Pixel to, AmigaColor& ham);
    void colorizeHAM8(u32 *dst, Pixel from, Pixel to, AmigaColor& ham);

    // Removes the border pixels on top of sprites
    void removeBorderOverSprites(Pixel from, Pixel to);
    
    //
    // Hiding graphics layers
    //
    
public:
    
    void hide(isize line, u16 layer, u8 alpha);
};

}
