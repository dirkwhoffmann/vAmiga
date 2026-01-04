// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "LinearDevice.h"
#include "Buffer.h"

namespace retro::device {

u8
LinearDevice::readByte(isize offset) const
{
    u8 value;
    read(&value, offset, 1);
    return value;
}

void
LinearDevice::writeByte(u8 value, isize offset)
{
    write(&value, offset, 1);
}

void
LinearDevice::exportBytes(const fs::path& path) const
{
    // Read data
    auto buffer = Buffer<u8>(size());
    read(buffer.ptr, 0, size());

    // Write to file
    ByteView(span<u8>(buffer.ptr, size())).writeToFile(path);
}

void
LinearDevice::exportBytes(const fs::path& path, Range<isize> range) const
{
    assert(range.inside(0, size()));

    // Read data
    auto buffer = Buffer<u8>(range.size());
    read(buffer.ptr, range.lower, range.size());

    // Write to file
    buffer.byteView().writeToFile(path);
}

}
