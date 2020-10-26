// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSUserDirBlock.h"

void
UserDirBlock::dump()
{
    
}

void
UserDirBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    p[3] = 0x02;
    
    // Block pointer to itself
    write32(p + 4, nr);
    /*
    p[4] = BYTE3(nr);
    p[5] = BYTE2(nr);
    p[6] = BYTE1(nr);
    p[7] = BYTE0(nr);
    */
    
    // Hashtable
    hashTable.write(p + 24);

    // Protection status bits
    u32 protection = 0;
    write32(p + 320, protection);
    /*
    p[320] = BYTE3(protection);
    p[321] = BYTE2(protection);
    p[322] = BYTE1(protection);
    p[323] = BYTE0(protection);
    */
    
    // Creation data
    created.write(p + 420);
    
    // Directory name
    name.write(p + 432);
    
    // Next block with same hash
    if (next) write32(p + 496, next->nr);
        /*
        p[496] = BYTE3(next->nr);
        p[497] = BYTE2(next->nr);
        p[498] = BYTE1(next->nr);
        p[499] = BYTE0(next->nr);
        */

    // Block pointer to parent directory
    assert(parent != NULL);
    
    write32(p + 500, parent->nr);
    /*
    p[500] = BYTE3(parent->nr);
    p[501] = BYTE2(parent->nr);
    p[502] = BYTE1(parent->nr);
    p[503] = BYTE0(parent->nr);
    */
    
    // Subtype
    p[508] = 2;
        
    // Checksum
    write32(p + 20, Block::checksum(p));
    /*
    u32 checksum = Block::checksum(p);
    p[20] = BYTE3(checksum);
    p[21] = BYTE2(checksum);
    p[22] = BYTE1(checksum);
    p[23] = BYTE0(checksum);
    */
}
