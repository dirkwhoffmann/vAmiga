// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DeviceTypes.h"
#include "utl/abilities/Loggable.h"
#include "utl/primitives/BitView.h"

namespace retro::vault {

using namespace utl;

class DiskDecoder : public Loggable {

    // Backing buffers
    std::vector<u8> trackBuffer;
    std::vector<u8> sectorBuffer;

public:

    virtual ~DiskDecoder() = default;

    virtual isize requiredTrackSize(TrackNr t) { return 16384; }
    virtual isize requiredSectorSize(TrackNr t, SectorNr s) { return 512; }

    virtual ByteView decodeTrack(BitView track, TrackNr t, std::span<u8> out) = 0;
    virtual ByteView decodeSector(BitView track, TrackNr t, SectorNr s, std::span<u8> out) = 0;

    ByteView decodeTrack(BitView track, TrackNr t);
    ByteView decodeSector(BitView track, TrackNr t, SectorNr s);
};

}
