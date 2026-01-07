// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskEncoder.h"

namespace retro::vault::image {

class IBMEncoder : public DiskEncoder {

    static constexpr isize bsize  = 512;  // Block size in bytes
    static constexpr isize ssize  = 1300; // MFM sector size in bytes
    static constexpr isize maxsec = 22;   // Maximum number of sectors

    // Backing buffer used by the MFM encoder
    u8 mfmBuffer[ssize * maxsec];

    // Backing buffer
    unique_ptr<u8> decoded;

public:

    BitView encodeTrack(TrackNr t, ByteView src) override;
    ByteView decodeTrack(TrackNr t, BitView src) override;

    // void encodeTrack(MutableByteView track, TrackNr t, ByteView src) override;
    void encodeSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView src);

    // void decodeTrack(ByteView track, TrackNr t, MutableByteView dst) override;
    void decodeSector(ByteView track, isize offset, MutableByteView dst);

    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);
};

namespace Encoder { extern IBMEncoder ibm; }

}
