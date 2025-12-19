// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "BlockVolume.h"

namespace vamiga {

Volume::Volume(BlockDevice &device) : device(device)
{
    range = { 0, device.capacity() };
}

Volume::Volume(PartitionedDevice &pd, isize partition) : device(pd) {

    range = pd.range(partition);
}

isize
Volume::capacity() const
{
    return range.size();
}

isize
Volume::bsize() const
{
    return device.bsize();
}

void
Volume::readBlock(u8 *dst, isize nr)
{
    device.readBlock(dst, range.translate(nr));
}

void
Volume::writeBlock(const u8 *src, isize nr)
{
    device.writeBlock(src, range.translate(nr));
}

}
