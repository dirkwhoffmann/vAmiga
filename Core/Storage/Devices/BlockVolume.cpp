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
Volume::freeBlock(isize nr)
{
    device.freeBlock(range.translate(nr));
}

Buffer<u8> *
Volume::readBlock(isize nr)
{
    return device.readBlock(range.translate(nr));
}

Buffer<u8> *
Volume::ensureBlock(isize nr)
{
    return device.ensureBlock(range.translate(nr));
}

void
Volume::writeBlock(isize nr, const Buffer<u8> &buffer)
{
    device.writeBlock(range.translate(nr), buffer);
}

}
