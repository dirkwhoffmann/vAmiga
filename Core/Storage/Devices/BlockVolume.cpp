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

isize
BlockVolume::bsize() const
{
    return device.bsize();
}

void
BlockVolume::freeBlock(isize nr)
{
    if (inRange(nr)) device.freeBlock(translate(nr));
}

Buffer<u8> *
BlockVolume::readBlock(isize nr)
{
    return inRange(nr) ? device.readBlock(translate(nr)) : nullptr;
}

Buffer<u8> *
BlockVolume::ensureBlock(isize nr)
{
    return inRange(nr) ? device.ensureBlock(translate(nr)) : nullptr;
}

void
BlockVolume::writeBlock(isize nr, const Buffer<u8> &buffer)
{
    if (inRange(nr)) device.writeBlock(translate(nr), buffer);
}

}
