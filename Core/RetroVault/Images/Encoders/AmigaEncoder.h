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

namespace retro::vault {

class AmigaEncoder : public DiskEncoder {

    // Backing buffer
    std::vector<u8> mfm;

public:

    // Methods from DiskDecoder
    BitView encodeTrack(TrackNr t, ByteView src) override;

    // Encodes a single sector
    void encodeSector(MutableByteView track, isize offset, TrackNr t, SectorNr s, ByteView src);
};

}
