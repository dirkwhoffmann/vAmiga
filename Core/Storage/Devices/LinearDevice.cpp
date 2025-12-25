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

namespace vamiga {

u8
LinearDevice::readByte(isize offset)
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

}
