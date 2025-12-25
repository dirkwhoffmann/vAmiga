// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/common.h"

namespace vamiga {

using namespace utl;

class LinearDevice {

public:

    LinearDevice() { }
    virtual ~LinearDevice() = default;

    // Total size in bytes
    virtual isize size() const = 0;

    // Reads a sequence of bytes starting at byte offset
    virtual void read(u8 *dst, isize offset, isize count) = 0;

    // Writes a sequence of bytes starting at byte offset
    virtual void write(const u8 *src, isize offset, isize count) = 0;

    // Reads a single byte
    virtual u8 readByte(isize offset);

    // Writes a single byte
    virtual void writeByte(u8 value, isize offset);
};

}
