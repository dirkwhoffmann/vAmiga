// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "LinearDevice.h"

namespace vamiga {

using namespace utl;

class BlockDevice : public LinearDevice {

public:
    
    BlockDevice() { }
    virtual ~BlockDevice() = default;

    // Block size in bytes
    virtual isize bsize() const = 0;

    // Number of blocks
    virtual isize capacity() const;

    // Reads a block
    virtual void readBlock(u8 *dst, isize nr) const;

    // Writes a block
    virtual void writeBlock(const u8 *src, isize nr);
};

}
