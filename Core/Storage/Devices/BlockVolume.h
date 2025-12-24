// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockDevice.h"
#include "DeviceDescriptors.h"

namespace vamiga {

class Volume : public BlockDevice {

    // The underlying block device
    class BlockDevice &device;

    // Blocks belonging to this volume
    Range<isize> range;

public:

    Volume(BlockDevice &d);
    Volume(BlockDevice &d, Range<isize> partition);
    virtual ~Volume() = default;
    
    isize capacity() const override;
    isize bsize() const override;
    // void freeBlock(isize nr) override;
    void readBlock(u8 *dst, isize nr) override;
    void writeBlock(const u8 *src, isize nr) override;
};

}
