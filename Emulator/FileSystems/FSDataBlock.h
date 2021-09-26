// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"

struct FSDataBlock : FSBlock {
      
    FSDataBlock(FSPartition &p, Block nr, FSBlockType t);
};

struct OFSDataBlock : FSDataBlock {

    OFSDataBlock(FSPartition &p, u32 nr, FSBlockType t);
};

struct FFSDataBlock : FSDataBlock {
      
    FFSDataBlock(FSPartition &p, u32 nr, FSBlockType t);
};
