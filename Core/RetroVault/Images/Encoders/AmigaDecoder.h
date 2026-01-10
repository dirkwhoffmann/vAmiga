// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskDecoder.h"

namespace retro::vault {

class AmigaDecoder : public DiskDecoder {

public:

    //
    // Methods from DiskDecoder
    //

    using DiskDecoder::decodeTrack;
    using DiskDecoder::decodeSector;
    ByteView decodeTrack(BitView track, TrackNr t, std::span<u8> out) override;
    ByteView decodeSector(BitView track, TrackNr t, SectorNr s, std::span<u8> out) override;

// private:

    // Returns the start offset of a sector (empty if not found)
    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);

    // Returns the start offset of a sector (throws if not found)
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);
};

}
