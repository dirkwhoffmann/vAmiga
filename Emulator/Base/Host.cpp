// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Host.h"
#include "Paula.h"
#include "IOUtils.h"

namespace vamiga {

void
Host::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::State) {

        os << tab("Audio sample rate");
        os << flt(sampleRate) << " Hz" << std::endl;
        os << tab("Monitor refresh rate");
        os << flt(refreshRate) << " Hz" << std::endl;
        os << tab("Frame buffer size");
        os << dec(frameBufferWidth) << " x ";
        os << dec(frameBufferHeight) << " Texels" << std::endl;
    }
}

void
Host::setSampleRate(double hz)
{
    sampleRate = hz;
    paula.muxer.setSampleRate(hz);
}

void
Host::setHostRefreshRate(double fps)
{
    switch (i16(fps)) {

        case 50: case 60: case 100: case 120: case 200: case 240:

            refreshRate = fps;
            break;

        default:

            // We keep the old value because the new value is likely the result
            // of a wrong measurement.
            break;
    }
}

std::pair<isize, isize>
Host::getFrameBufferSize() const
{
    return std::pair<isize, isize>(frameBufferWidth, frameBufferHeight);
}

void
Host::setFrameBufferSize(std::pair<isize, isize> size)
{
    frameBufferWidth = size.first;
    frameBufferHeight = size.second;
}

}
