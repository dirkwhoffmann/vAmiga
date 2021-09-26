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
#include "BootBlockImage.h"

struct FSBootBlock : FSBlock {
        
    FSBootBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSBootBlock();
    
    //
    // Block specific methods
    //

    void writeBootBlock(BootBlockId id, isize page);
};
