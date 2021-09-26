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

struct FSEmptyBlock : FSBlock {
    
    FSEmptyBlock(FSPartition &p, Block nr, FSBlockType t) : FSBlock(p, nr, t) { }
         
    
    //
    // Methods from Block class
    //

    u32 typeID() const override { return 0; }
    u32 subtypeID() const override { return 0; }

    void importBlock(const u8 *src, isize bsize) override;
    void exportBlock(u8 *dst, isize bsize) override;
};
