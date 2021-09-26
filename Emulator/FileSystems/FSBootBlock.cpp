// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "BootBlockImage.h"
#include "FSBootBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSBootBlock::FSBootBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t)
{    

}
