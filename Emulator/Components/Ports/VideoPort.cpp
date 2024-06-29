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
            throw Error(ERROR_OPT_UNSUPPORTED);
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

const FrameBuffer &
VideoPort::getTexture() const
{
    if (isPoweredOn()) {

        return denise.pixelEngine.getStableBuffer();
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

/*
u32 *
VideoPort::getNoiseTexture() const
{
    static u32 *noise = nullptr;
    constexpr isize noiseSize = 16 * 512 * 512;

    if (!noise) {

        noise = new u32[noiseSize];

        for (isize i = 0; i < noiseSize; i++) {
            noise[i] = rand() % 2 ? 0xFF000000 : 0xFFFFFFFF;
        }
    }

    int offset = rand() % (512 * 512);
    return noise + offset;
}

u32 *
VideoPort::getBlankTexture() const
{
    static u32 *blank = nullptr;

    if (!blank) {

        blank = new u32[Texture::height * Texture::width];

        for (isize i = 0; i < Texture::height * Texture::width; i++) {
            blank[i] = 0xFF000000;
        }
    }

    return blank;
}
*/

}
