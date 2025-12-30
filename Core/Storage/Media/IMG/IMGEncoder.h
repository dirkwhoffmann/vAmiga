// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "IMGFile.h"
#include "FloppyDisk.h"

namespace vamiga {

class IMGEncoder {

    /*
public:

    static void encode(const IMGFile &source, FloppyDisk &target);
    static void decode(IMGFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const IMGFile &img, FloppyDisk &disk, TrackNr t);
    static void encodeSector(const IMGFile &img, FloppyDisk &disk, TrackNr t, SectorNr s);

    static void decodeTrack(IMGFile &img, const FloppyDisk &disk, TrackNr t);
    static void decodeSector(IMGFile &img, u8 *dst, const u8 *src);
    */
};

}
