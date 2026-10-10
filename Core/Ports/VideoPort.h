// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VideoPortTypes.h"
#include "SubComponent.h"
#include "Texture.h"
#include "utl/wrappers.h"

namespace vamiga {

class VideoPort final : public SubComponent {

    Descriptions descriptions = {{

        .type           = Class::VideoPort,
        .name           = "Video",
        .description    = "Video Port",
        .shell          = "video"
    }};

    Options options = {

        Opt::VID_WHITE_NOISE
    };

    // Current configuration
    VideoPortConfig config = { };

public:

    // Result of the latest inspection
    utl::Backed<VideoPortInfo> info;
    utl::Backed<VideoPortStats> metrics;

private:

    // Predefined frame buffers
    mutable Texture whiteNoise;
    Texture blank;

    //  White noise data
    Buffer <Texel> noise;

    // Remembers the number of the most recently grabbed frame
    mutable i64 latestGrabbedFrame = 0;

    // Counts the number of dropped frames
    mutable isize droppedFrames = 0;


    //
    // Methods
    //

public:

    VideoPort(Amiga &ref);
    ~VideoPort();

    VideoPort& operator= (const VideoPort& other) {

        CLONE(config)

        return *this;
    }

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Serializable
    //

public:

    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) return;

        worker

        << config.whiteNoise;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

private:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from Configurable
    //

public:

    const VideoPortConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt opt) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt opt, i64 value) override;


    //
    // Analyzing
    //

public:

    VideoPortInfo cacheInfo() const;
    VideoPortStats cacheStats() const;


    //
    // Getting textures
    //

public:

    // Returns a pointer to the stable emulator texture
    const class Texture &getTexture(isize offset = 0) const;

    // Returns a pointer to the stable DMA debugger texture
    const class Texture &getXrayTexture(isize offset = 0) const;

    /* Saves the stable emulator texture or DMA debugger texture as a PNG file.
     *
     * The cutout is given in texels of the texture: x counts super-hires
     * pixels (0 ... 2 * HPIXELS), y counts lines (0 ... VPIXELS). The upper
     * bounds x2 and y2 are exclusive. Without a cutout, the entire texture is
     * saved. Lines are repeated four times to compensate for the elongated
     * shape of super-hires pixels, so the image has the proportions the GUI
     * shows. Throws if the cutout is empty or exceeds the texture.
     */
    void saveTexture(const fs::path &path, isize offset = 0) const;
    void saveTexture(const fs::path &path, isize x1, isize x2, isize y1, isize y2,
                     isize offset = 0) const;
    void saveXrayTexture(const fs::path &path, isize offset = 0) const;
    void saveXrayTexture(const fs::path &path, isize x1, isize x2, isize y1, isize y2,
                         isize offset = 0) const;

private:

    void savePNG(const Texture &texture, const fs::path &path,
                 isize x1, isize x2, isize y1, isize y2) const;

public:

    // Informs the video port about a buffer swap
    void buffersWillSwap();
    
    // Determines the active texture area by auto-detecting border pixels
    void findInnerArea(isize &x1, isize &x2, isize &y1, isize &y2) const;
    void findInnerAreaNormalized(double &x1, double &x2, double &y1, double &y2) const;
};

}
