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

    static constexpr isize bsize  = 512;  // Block size in bytes
    static constexpr isize ssize  = 1088; // MFM sector size in bytes
    static constexpr isize maxsec = 22;   // Maximum number of sectors

    // Backing buffer
    std::vector<u8> decoded;

public:

    ByteView decodeTrack(TrackNr t, BitView src) override;

    void decodeSector(ByteView track, isize offset, MutableByteView dst);

    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);
};

namespace Decoder { extern AmigaDecoder amiga; }

}
