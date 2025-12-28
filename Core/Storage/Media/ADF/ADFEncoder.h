// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"
#include "FloppyDisk.h"

namespace vamiga {

class ADFEncoder {

public:

    [[deprecated]] static void encode(const ADFFile &source, FloppyDisk &target);
    static void decode(ADFFile &target, const FloppyDisk &source);

private:

    static void encodeTrack(const ADFFile &adf, FloppyDisk &disk, Track t);
    static void encodeSector(const ADFFile &adf, FloppyDisk &disk, Track t, Sector s);

    static void decodeTrack(ADFFile &adf, const FloppyDisk &disk, Track t);
    static void decodeSector(ADFFile &adf, u8 *dst, const u8 *src);
};

}
