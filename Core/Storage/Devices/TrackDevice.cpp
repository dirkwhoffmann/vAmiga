// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "TrackDevice.h"
#include <algorithm>

namespace vamiga {

using CHS = TrackDevice::CHS;
using TS  = TrackDevice::TS;

void
TrackDevice::buildTrackMap() const
{
    track2block.clear();
    track2block.reserve(numTracks());

    for (isize t = 0, offset = 0; t < numTracks(); ++t) {

        track2block.push_back(offset);
        offset += numSectors(t);
    }
}

isize
TrackDevice::block2track(isize b) const
{
    assert(0 <= b && b < capacity());

    if (track2block.empty()) buildTrackMap();

    // Find the track via binary search
    auto it = std::upper_bound(track2block.begin(), track2block.end(), b);
    return isize(std::distance(track2block.begin(), it) - 1);
}

CHS
TrackDevice::chs(isize b) const
{
    auto t = block2track(b);
    auto c = t / numHeads();
    auto h = t % numHeads();
    auto s = b - track2block[t];

    return { c, h, s };
}

CHS
TrackDevice::chs(isize t, isize s) const
{
    assert(0 <= t && t < numTracks());
    assert(0 <= s && s < numSectors(t));

    auto c = t / numHeads();
    auto h = t % numHeads();

    return { c, h, s };
}

TS
TrackDevice::ts(isize b) const
{
    assert(0 <= b && b < capacity());

    auto t = block2track(b);
    auto s = b - track2block[t];

    return { t, s };
}

TS
TrackDevice::ts(isize c, isize h, isize s) const
{
    auto t = c * numHeads() + h;

    assert(0 <= t && t < numTracks());
    assert(0 <= s && s < numSectors(t));

    return { t, s };
}

isize
TrackDevice::bindex(CHS chs) const
{
    return bindex(TS {chs.cylinder * numHeads() + chs.head, chs.sector});
}

isize
TrackDevice::bindex(TS ts) const
{
    assert(0 <= ts.track && ts.track < numTracks());
    assert(0 <= ts.sector && ts.sector < numSectors(ts.track));

    if (track2block.empty()) buildTrackMap();

    return track2block[ts.track] + ts.sector;
}

}
