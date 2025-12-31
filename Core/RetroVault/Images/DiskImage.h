// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyImage.h"
#include "TrackDevice.h"

namespace vamiga {

class DiskImage : public AnyImage, public TrackDevice {

    //
    // Methods from LinearDevice
    //

    isize size() const override { return data.size; }
    void read(u8 *dst, isize offset, isize count) const override;
    void write(const u8 *src, isize offset, isize count) override;


    //
    // Obtaining views on tracks and sectors
    //

public:

    using AnyImage::byteView;
    ByteView byteView(TrackNr t) const;
    ByteView byteView(TrackNr t, SectorNr s) const;
    MutableByteView byteView(TrackNr t);
    MutableByteView byteView(TrackNr t, SectorNr s);
};

}
