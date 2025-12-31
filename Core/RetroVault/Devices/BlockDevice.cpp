// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockDevice.h"

namespace vamiga {

isize
BlockDevice::capacity() const
{
    return size() / bsize();
}

void
BlockDevice::readBlock(u8 *dst, isize nr) const
{
    assert(0 <= nr && nr < capacity());
    read(dst, nr * bsize(), bsize());
}

void
BlockDevice::writeBlock(const u8 *src, isize nr)
{
    assert(0 <= nr && nr < capacity());
    write(src, nr * bsize(), bsize());
}

void
BlockDevice::readBlock(span<u8> dst, isize nr) const
{
    assert(bsize() <= (isize)dst.size());
    readBlock(dst.data(), nr);
}

void
BlockDevice::writeBlock(span<const u8> src, isize nr)
{
    assert(bsize() <= (isize)src.size());
    writeBlock(src.data(), nr);
}

void
BlockDevice::exportBlock(const fs::path& path, isize nr) const
{
    exportBlocks(path, Range(nr, nr + 1));
}

void
BlockDevice::exportBlocks(const fs::path& path, Range<isize> range) const
{
    exportBytes(path, Range(range.lower * bsize(), range.upper * bsize()));
}

}
