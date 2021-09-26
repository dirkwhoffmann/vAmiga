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

struct FSBitmapBlock : FSBlock {
                    
    FSBitmapBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSBitmapBlock();
     
    
    //
    // Methods from Block class
    //
};
