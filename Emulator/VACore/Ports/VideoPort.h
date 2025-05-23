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
#include "FrameBuffer.h"

namespace vamiga {

class VideoPort final : public SubComponent, public Inspectable<VideoPortInfo, VideoPortStats> {

    Descriptions descriptions = {{

        .type           = Class::VideoPort,
        .name           = "Video",
        .description    = "Video Port",
        .shell          = "video"
    }};

    ConfigOptions options = {

        Opt::VID_WHITE_NOISE
    };

    // Current configuration
    VideoPortConfig config = { };

    // Predefined frame buffers
    mutable FrameBuffer whiteNoise;
    FrameBuffer blank;

    //  White noise data
    Buffer <Texel> noise;


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

    } SERIALIZERS(serialize, override);


    //
    // Methods from CoreComponent
    //

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Configurable
    //

public:

    const VideoPortConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt opt) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt opt, i64 value) override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(VideoPortInfo &result) const override;
    void cacheStats(VideoPortStats &result) const override;


    //
    // Getting textures
    //

public:

    // Returns a pointer to the stable emulator texture
    const class FrameBuffer &getTexture(isize offset = 0) const;

    // Informs the video port about a buffer swap
    void buffersWillSwap();
    
    // Determines the active texture area by auto-detecting border pixels
    void findInnerArea(isize &x1, isize &x2, isize &y1, isize &y2) const;
    void findInnerAreaNormalized(double &x1, double &x2, double &y1, double &y2) const;
};

}
