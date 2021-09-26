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

struct FSFileHeaderBlock : FSBlock {
                
    FSFileHeaderBlock(FSPartition &p, Block nr, FSBlockType t);

    //
    // Block specific methods
    //
    
    isize writeData(FILE *file);
};
