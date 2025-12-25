// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockDevice.h"

namespace vamiga {

using namespace utl;

class TrackDevice : public BlockDevice {

public:

    TrackDevice() { }
    virtual ~TrackDevice() = default;

    /*
    // Block size in bytes
    virtual isize bsize() const = 0;

    // Number of blocks
    virtual isize capacity() const = 0;

    // Reads a block
    virtual void readBlock(u8 *dst, isize nr) = 0;

    // Writes a block
    virtual void writeBlock(const u8 *src, isize nr) = 0;+
    */
};

}
