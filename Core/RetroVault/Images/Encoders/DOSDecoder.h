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

class DOSDecoder : public DiskDecoder {

    static constexpr isize bsize  = 512;  // Block size in bytes
    static constexpr isize ssize  = 1300; // MFM sector size in bytes
    static constexpr isize maxsec = 22;   // Maximum number of sectors

public:

    //
    // Methods from DiskDecoder
    //

    using DiskDecoder::decodeTrack;
    using DiskDecoder::decodeSector;
    ByteView decodeTrack(BitView track, TrackNr t, std::span<u8> out) override;
    ByteView decodeSector(BitView track, TrackNr t, SectorNr s, std::span<u8> out) override;

    // void decodeTrack(ByteView track, TrackNr t, MutableByteView dst) override;
    void decodeSector(ByteView track, isize offset, MutableByteView dst);

    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);
};

namespace Decoder { extern DOSDecoder dos; }

}
