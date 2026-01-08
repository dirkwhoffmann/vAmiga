// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DeviceTypes.h"
#include "utl/abilities/Loggable.h"
#include "utl/primitives/BitView.h"

namespace retro::vault {

using namespace utl;

class DiskDecoder : public Loggable {

public:

    virtual ~DiskDecoder() = default;
    virtual ByteView decodeTrack(TrackNr t, BitView src) = 0;
};

}
