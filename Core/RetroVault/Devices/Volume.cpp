// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "Volume.h"

namespace retro::vault::device {

Volume::Volume(BlockDevice &device) : device(device)
{
    this->range = { 0, device.capacity() };
}

Volume::Volume(BlockDevice &device, Range<isize> range) : device(device)
{
    this->range = range;
}

void
Volume::read(u8 *dst, isize offset, isize count) const
{
    if(offset < 0 || count < 0 || offset + count > range.size() * bsize()) {
        throw Error(offset, "Range out of bounds");
    }

    device.read(dst, range.translate(0) * bsize() + offset, count);
}

void
Volume::write(const u8 *src, isize offset, isize count)
{
    if(offset < 0 || count < 0 || offset + count > range.size() * bsize()) {
        throw Error(offset, "Range out of bounds");
    }

    device.write(src, range.translate(0) * bsize() + offset, count);
}

void
Volume::readBlock(u8 *dst, isize nr) const
{
    device.readBlock(dst, range.translate(nr));
}

void
Volume::writeBlock(const u8 *src, isize nr)
{
    device.writeBlock(src, range.translate(nr));
}

}
