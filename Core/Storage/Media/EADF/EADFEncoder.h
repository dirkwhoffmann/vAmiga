// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "EADFFile.h"
#include "FloppyDisk.h"

namespace vamiga {

class EADFEncoder {

public:

    static void encode(const EADFFile &source, FloppyDisk &target);
    static void decode(EADFFile &target, const FloppyDisk &source);

private:

    static void encodeStandardTrack(const EADFFile &eadf, ADFFile &adf, TrackNr t);
    static void encodeExtendedTrack(const EADFFile &eadf, FloppyDisk &disk, TrackNr t);
};

}
