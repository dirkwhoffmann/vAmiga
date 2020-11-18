// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_EMPTY_BLOCK_H
#define _FS_EMPTY_BLOCK_H

#include "FSBlock.h"

struct FSEmptyBlock : FSBlock {
    
    FSEmptyBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr) { }
     
    FSBlockType type() override { return FS_EMPTY_BLOCK; }
    u32 typeID() override { return 0; }
    u32 subtypeID() override { return 0; }

    void importBlock(const u8 *src, size_t bsize) override;
    void exportBlock(u8 *dst, size_t bsize) override;
};

#endif
