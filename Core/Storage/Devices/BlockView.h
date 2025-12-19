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

class BlockView {

public:

    BlockView() { }
    virtual ~BlockView() = default;

    virtual isize capacity() const = 0;
    virtual isize bsize() const = 0;
    virtual void readBlock(u8 *dst, isize nr) = 0;
    virtual void writeBlock(const u8 *src, isize nr) = 0;
};

}
