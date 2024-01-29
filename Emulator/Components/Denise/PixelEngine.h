// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "PixelEngineTypes.h"
#include "SubComponent.h"
#include "ChangeRecorder.h"
#include "Constants.h"
#include "FrameBuffer.h"

namespace vamiga {

class PixelEngine : public SubComponent {

    friend class Denise;

    // Current configuration
    PixelEngineConfig config = {};

public:

    //
    // Screen buffers
    //

private:

    /* The emulator utilizes double-buffering for the computed textures.
     * At any time, one of the two buffers is the "working buffer". The other
     * one is the "stable buffer". All drawing functions write to the working
     * buffer and the GPU reads from the stable buffer. Once a frame has
     * been completed, the working buffer and the stable buffer are swapped.
     */
    FrameBuffer emuTexture[2];

    // The currently active buffer
    isize activeBuffer = 0;

    // Mutex for synchronizing access to the stable buffer
    util::Mutex bufferMutex;

    // Buffer with background noise (random black and white pixels)
    Buffer <Texel> noise;

    
    //
    // Color management
    //

    // Lookup table for all 4096 Amiga colors
    Texel colorSpace[4096];

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
    RegChangeRecorder<128> colChanges;


    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    // Initializes both frame buffers with a checkerboard pattern
    void clearAll();


    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "PixelEngine"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;

    
    //
    // Configuring
    //

public:
    
    const PixelEngineConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

    
    //
    // Serializing
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
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;

    
    //
    // Controlling
    //
    
private:

    void _powerOn() override;


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

private:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(u8 &r, u8 &g, u8 &b);


    //
    // Working with frame buffers
    //

public:

    // Returns the working buffer or the stable buffer
    FrameBuffer &getWorkingBuffer();
    const FrameBuffer &getStableBuffer();

    // Return a pointer into the pixel storage
    Texel *workingPtr(isize row = 0, isize col = 0);
    Texel *stablePtr(isize row = 0, isize col = 0);
    
    // Swaps the working buffer and the stable buffer
    void swapBuffers();
    
    // Returns a pointer to randon noise
    Texel *getNoise() const;

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
    
    /* Hides some graphics layers. This function is an optional stage applied
     * after colorize(). It can be used to hide some layers for debugging.
     */
    
public:
    
    void hide(isize line, u16 layer, u8 alpha);
};

}
