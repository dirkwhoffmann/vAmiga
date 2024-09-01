// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "VideoPort.h"
#include "Denise.h"

namespace vamiga {

VideoPort::VideoPort(Amiga &ref) : SubComponent(ref)
{
    // Create random noise data
    noise.alloc(2 * PIXELS);
    for (isize i = 0; i < noise.size; i++) {
        noise[i] = rand() % 2 ? FrameBuffer::black : FrameBuffer::white;
    }

    // Setup the white-noise framebuffer (redirect the data source)
    whiteNoise.pixels.dealloc();
    whiteNoise.pixels.ptr = noise.ptr;
    whiteNoise.pixels.size = PIXELS;

    // Setup the blank framebuffer
    for (isize i = 0; i < blank.pixels.size; i++) {
        blank.pixels.ptr[i] = 0xFF000000;
    }
};

VideoPort::~VideoPort()
{
    // Don't free the white-noise framebuffer as it points to managed data
    whiteNoise.pixels.ptr = nullptr;
    whiteNoise.pixels.size = 0;
}

void
VideoPort::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        dumpConfig(os);
    }

    if (category == Category::State) {

    }
}

i64
VideoPort::getOption(Option option) const
{
    switch (option) {

        case OPT_VID_WHITE_NOISE:   return config.whiteNoise;

        default:
            fatalError;
    }
}

void
VideoPort::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_VID_WHITE_NOISE:

            return;

        default:
            throw Error(VAERROR_OPT_UNSUPPORTED);
    }
}

void
VideoPort::setOption(Option opt, i64 value)
{
    checkOption(opt, value);

    switch (opt) {

        case OPT_VID_WHITE_NOISE:

            config.whiteNoise = (bool)value;
            return;

        default:
            fatalError;
    }
}

void 
VideoPort::cacheInfo(VideoPortInfo &result) const
{

}

void 
VideoPort::cacheStats(VideoPortStats &result) const
{

}

const FrameBuffer &
VideoPort::getTexture() const
{
    if (isPoweredOn()) {

        auto &result = denise.pixelEngine.getStableBuffer();
        info.latestGrabbedFrame = result.nr;
        return result;
    }
    if (config.whiteNoise) {

        whiteNoise.pixels.ptr = noise.ptr + (rand() % PIXELS);
        whiteNoise.nr++;
        whiteNoise.prevlof = whiteNoise.lof;
        whiteNoise.lof = !whiteNoise.lof;

        return whiteNoise;
    }

    return blank;
}

void 
VideoPort::buffersWillSwap()
{
    // Check if the texture has been grabbed
    auto grabbed = info.latestGrabbedFrame;
    auto current = denise.pixelEngine.getStableBuffer().nr;

    if (grabbed < current) {

        stats.droppedFrames++;
        debug(VID_DEBUG, "Frame %lld dropped (total: %ld latest: %lld)\n", 
            current, stats.droppedFrames, grabbed);
    }
}

}
