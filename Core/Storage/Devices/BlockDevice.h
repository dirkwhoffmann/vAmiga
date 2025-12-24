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
#include "utl/primitives.h"

namespace vamiga {

using namespace utl;

class BlockDevice {

public:
    
    BlockDevice() { }
    virtual ~BlockDevice() = default;

    // Returns the number of blocks
    virtual isize capacity() const = 0;

    // Returns the block size in bytes
    virtual isize bsize() const = 0;

    // Reads a block
    virtual void readBlock(u8 *dst, isize nr) = 0;

    // Writes a block
    virtual void writeBlock(const u8 *src, isize nr) = 0;
};

}
