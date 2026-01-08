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

    // Backing buffer
    std::vector<u8> bytes;

public:

    // Methods from DiskDecoder
    ByteView decodeTrack(TrackNr t, BitView src) override;

    // Returns the start offset of a sector (empty if not found)
    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);

    // Returns the start offset of a sector (throws if not found)
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);

    // Decodes a single sector
    void decodeSector(ByteView track, isize offset, MutableByteView dst);

};

// DEPRECATED
namespace Decoder { extern AmigaDecoder amiga; }

}
