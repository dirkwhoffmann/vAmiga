// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSBitmapBlock.h"

BitmapBlock::BitmapBlock(long cap) : capacity(cap)
{
    allocated = new bool[capacity]();
    
    // The first two blocks are always allocated
    allocated[0] = true;
    allocated[1] = true;
}

void
BitmapBlock::dump()
{
    for (int i = 0; i < capacity; i++) {
        printf("%c", allocated[i] ? '1' : '0');
    }
    printf("\n");
}

void
BitmapBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);

    // Write allocation map
    for (long i = 2; i < capacity; i++) {

        if (allocated[i]) continue;
        
        // Determine bit position for this block inside the bitmap. Layout:
        //
        //     Position: p[00] p[01] p[02] ... p[31] p[32] p[33] ... p[63]
        //       Sector:   29    28    27         2    61    60        34
        //
        // Remember: The first two sectors are always allocated and not part
        // the map.

        long bit = (i - 2) % 8;
        long byte = (i - 2) / 8;
        switch (byte % 4) {
            case 0: byte += 3; break;
            case 1: byte += 1; break;
            case 2: byte -= 1; break;
            case 3: byte -= 3; break;
        }
        
        SET_BIT(p[4 + byte], bit);
    }
        
    // Compute checksum
    write32(p, Block::checksum(p));
    /*
    u32 checksum = Block::checksum(p);
    p[0] = BYTE3(checksum);
    p[1] = BYTE2(checksum);
    p[2] = BYTE1(checksum);
    p[3] = BYTE0(checksum);
    */
    
    for (unsigned i = 0; i < 256; i++) {
        printf("%d:%x ", i, p[i]);
    }
    printf("\n");
}
