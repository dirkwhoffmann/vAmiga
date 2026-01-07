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
#include "DeviceError.h"
#include "utl/abilities/Loggable.h"
#include "utl/primitives/BitView.h"

namespace retro::vault::image {

using namespace utl;
using namespace retro::vault::device;

class DiskEncoder : public Loggable {

public:

    virtual ~DiskEncoder() = default;

    virtual BitView encodeTrack(TrackNr t, ByteView src) = 0;

    // DEPRECATED API
    [[deprecated]] virtual void encodeTrack(MutableByteView track, TrackNr t, ByteView src) = 0;
    virtual void decodeTrack(ByteView track, TrackNr t, MutableByteView dst) = 0;
};

}
