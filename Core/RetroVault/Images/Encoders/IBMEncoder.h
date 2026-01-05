// -----------------------------------------------------------------------------
// This file is part of vAmiga
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

public:

    void encodeTrack(MutableByteView track, TrackNr t, ByteView src) override;
    void encodeSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView src);

    void decodeTrack(ByteView track, TrackNr t, MutableByteView dst) override;
    void decodeSector(ByteView track, isize offset, MutableByteView dst);

    optional<isize> trySeekSector(ByteView track, SectorNr s, isize offset = 0);
    isize seekSector(ByteView track, SectorNr s, isize offset = 0);

    // Computes a map from sector numbers to byte offsets
    std::unordered_map<isize, isize> seekSectors(ByteView track);
};

namespace Encoder { extern IBMEncoder ibm; }

}
