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

namespace retro::device {

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

    // Safety wrappers
    void readBlock(span<u8> dst, isize nr) const;
    void writeBlock(span<const u8> src, isize nr);

    // Exports a single block or a block range to a file
    void exportBlock(const fs::path& path, isize nr) const;
    void exportBlocks(const fs::path& path, Range<isize> range) const;
};

}
