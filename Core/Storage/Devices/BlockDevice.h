// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlockView.h"
#include "utl/primitives.h"

namespace vamiga {

using namespace utl;

class BlockDevice : public BlockView {

    using BlockView::BlockView;
};

class PartitionedDevice : public BlockDevice {

public:

    virtual isize numPartitions() const = 0;
    virtual Range<isize> range(isize partition) const = 0;

    using BlockDevice::BlockDevice;
};

}
