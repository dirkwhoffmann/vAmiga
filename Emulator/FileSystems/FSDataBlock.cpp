// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDataBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSDataBlock::FSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSBlock(p, nr, t)
{
}

OFSDataBlock::OFSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSDataBlock(p, nr, t)
{    
}

FFSDataBlock::FFSDataBlock(FSPartition &p, u32 nr, FSBlockType t) : FSDataBlock(p, nr, t) { }
