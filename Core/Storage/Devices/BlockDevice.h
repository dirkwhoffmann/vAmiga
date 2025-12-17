// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/storage.h"

namespace vamiga {

using namespace utl;

class BlockDevice {

public:

    virtual ~BlockDevice() = default;

    virtual isize capacity() const = 0;
    virtual isize bsize() const = 0;
    virtual void freeBlock(isize nr) = 0;
    virtual Buffer<u8> *readBlock(isize nr) = 0;
    virtual Buffer<u8> *ensureBlock(isize nr) = 0;
    virtual void writeBlock(isize nr, const Buffer<u8> &) = 0;

    bool inRange(isize nr) { return nr >= 0 && nr < capacity(); }
};

}
