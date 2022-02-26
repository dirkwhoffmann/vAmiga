
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSPartition.h"
#include "FSDevice.h"
#include <algorithm>
#include <vector>

FSPartition::FSPartition(FSDevice &dev, FSDeviceDescriptor &layout) : FSPartition(dev)
{
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
}
