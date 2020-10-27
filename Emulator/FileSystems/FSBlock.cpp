// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

Block::Block(FSVolume &ref) : volume(ref)
{

}

u32
Block::checksum(u8 *p)
{
    assert(p != nullptr);
    
    u32 result = 0;

    for (int i = 0; i < 512; i += 4, p += 4) {
        result += HI_HI_LO_LO(p[0], p[1], p[2], p[3]);
    }
    
    return ~result + 1;
}

void
Block::write32(u8 *p, u32 value)
{
    p[0] = BYTE3(value);
    p[1] = BYTE2(value);
    p[2] = BYTE1(value);
    p[3] = BYTE0(value);
}

void
HashableBlock::link(HashableBlock *block)
{
    assert(block != nullptr);
    
    if (next) {
        next->link(block);
    } else {
        next = block;
    }
}
