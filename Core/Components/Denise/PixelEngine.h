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
#include "Texture.h"
#include "utl/concurrency.h"

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
    AmigaColor color[32];

    /* Active color palette
     *
     *  0 .. 31 : ABGR values of the 32 color registers
     * 32 .. 63 : ABGR values of the 32 color registers in halfbright mode
     *       64 : Pure black (used if the ECS BRDRBLNK bit is set)
     * 65 .. 67 : Additional debug colors
     */
    static const int paletteCnt = 32 + 32 + 1 + 3;
    Texel palette[paletteCnt];
    
    // Indicates whether HAM mode or SHRES mode is enabled
    bool hamMode;
    bool shresMode;

    
    //
    // Register change history buffer
    //

public:

    // Color register history
    RegChangeRecorder<256> colChanges;


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
        CLONE(hamMode)
        CLONE(shresMode)
        CLONE_ARRAY(palette)

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
        << hamMode
        << shresMode;

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
    
    // Changes one of the 32 Amiga color registers
    void setColor(isize reg, u16 value);
    void setColor(isize reg, AmigaColor value);

    // Returns a color value in Amiga format
    u16 getColor(isize nr) const { return color[nr].rawValue(); }

    // Returns sprite color in Amiga format
    u16 getSpriteColor(isize s, isize nr) const { return getColor(16 + nr + 2 * (s & 6)); }


    //
    // Using the color lookup table
    //

public:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Converts an Amiga color into a texel, applying the monitor settings
    Texel toTexel(const AmigaColor c) const;
    
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
    
    void colorize(Texel *dst, Pixel from, Pixel to);
    void colorizeSHRES(Texel *dst, Pixel from, Pixel to);
    void colorizeHAM(Texel *dst, Pixel from, Pixel to, AmigaColor& ham);
    
    
    //
    // Hiding graphics layers
    //
    
public:
    
    void hide(isize line, u16 layer, u8 alpha);
};

}
