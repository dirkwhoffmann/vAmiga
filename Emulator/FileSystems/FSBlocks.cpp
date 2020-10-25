// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSBlocks.h"

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
HashableBlock::link(HashableBlock *block)
{
    assert(block != nullptr);
    
    if (next) {
        next->link(block);
    } else {
        next = block;
    }
}

void
BootBlock::write(u8 *dst)
{
    assert(dst != NULL);
 
    // Start from scratch
    memset(dst, 0, 512);
        
    u8 ofsData[] = {
        
        0xc0, 0x20, 0x0f, 0x19, 0x00, 0x00, 0x03, 0x70, 0x43, 0xfa, 0x00, 0x18,
        0x4e, 0xae, 0xff, 0xa0, 0x4a, 0x80, 0x67, 0x0a, 0x20, 0x40, 0x20, 0x68,
        0x00, 0x16, 0x70, 0x00, 0x4e, 0x75, 0x70, 0xff, 0x60, 0xfa, 0x64, 0x6f,
        0x73, 0x2e, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79
    };
    
    u8 ffsData[] = {
        
        0xE3, 0x3D, 0x0E, 0x72, 0x00, 0x00, 0x03, 0x70, 0x43, 0xFA, 0x00, 0x3E,
        0x70, 0x25, 0x4E, 0xAE, 0xFD, 0xD8, 0x4A, 0x80, 0x67, 0x0C, 0x22, 0x40,
        0x08, 0xE9, 0x00, 0x06, 0x00, 0x22, 0x4E, 0xAE, 0xFE, 0x62, 0x43, 0xFA,
        0x00, 0x18, 0x4E, 0xAE, 0xFF, 0xA0, 0x4A, 0x80, 0x67, 0x0A, 0x20, 0x40,
        0x20, 0x68, 0x00, 0x16, 0x70, 0x00, 0x4E, 0x75, 0x70, 0xFF, 0x4E, 0x75,
        0x64, 0x6F, 0x73, 0x2E, 0x6C, 0x69, 0x62, 0x72, 0x61, 0x72, 0x79, 0x00,
        0x65, 0x78, 0x70, 0x61, 0x6E, 0x73, 0x69, 0x6F, 0x6E, 0x2E, 0x6C, 0x69,
        0x62, 0x72, 0x61, 0x72, 0x79, 0x00, 0x00, 0x00
    };
        
    // Write header
    dst[0] = 'D';
    dst[1] = 'O';
    dst[2] = 'S';
    dst[3] = ffs ? 1 : 0;
    
    // Write data
    if (ffs) {
        memcpy(dst + 4, ffsData, sizeof(ffsData));
    } else {
        memcpy(dst + 4, ofsData, sizeof(ofsData));
    }
}

void
RootBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);

    // Type
    p[3] = 0x02;
    
    // Hashtable size
    p[15] = 0x48;
    
    // Hashtable entries
    hashTable.write(p + 24);
    
    // BM flag (true if bitmap on disk is valid)
    p[312] = p[313] = p[314] = p[315] = 0xFF;
    
    // BM pages (indicates the blocks containing the bitmap)
    p[318] = HI_BYTE(881);
    p[319] = LO_BYTE(881);
    
    // Last recent change of the root directory of this volume
    lastModified.write(p + 420);
    
    // Date and time when this volume was formatted
    created.write(p + 484);
    
    // Volume name
    name.write(p + 432);
    
    // Secondary block type
    p[511] = 0x01;
    
    // Compute checksum
    u32 checksum = Block::checksum(p);
    p[20] = BYTE3(checksum);
    p[21] = BYTE2(checksum);
    p[22] = BYTE1(checksum);
    p[23] = BYTE0(checksum);
}

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
    u32 checksum = Block::checksum(p);
    p[0] = BYTE3(checksum);
    p[1] = BYTE2(checksum);
    p[2] = BYTE1(checksum);
    p[3] = BYTE0(checksum);
    
    for (unsigned i = 0; i < 256; i++) {
        printf("%d:%x ", i, p[i]);
    }
    printf("\n");
}

UserDirBlock::UserDirBlock(FSName name)
{
    this->name = name;
}

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
    printf("nr = %ld\n", nr);
    p[4] = BYTE3(nr);
    p[5] = BYTE2(nr);
    p[6] = BYTE1(nr);
    p[7] = BYTE0(nr);

    // Protection status bits
    u32 protection = 0;
    p[320] = BYTE3(protection);
    p[321] = BYTE2(protection);
    p[322] = BYTE1(protection);
    p[323] = BYTE0(protection);

    // Creation data
    date.write(p + 420);
    
    // Directory name
    name.write(p + 432);
    
    // Next block with same hash
    if (next) {
        printf("next = %ld\n", next->nr);
        p[496] = BYTE3(next->nr);
        p[497] = BYTE2(next->nr);
        p[498] = BYTE1(next->nr);
        p[499] = BYTE0(next->nr);
    }

    // Block pointer to parent directory
    assert(parent != NULL);
    printf("parent->nr = %ld\n", parent->nr);
    p[500] = BYTE3(parent->nr);
    p[501] = BYTE2(parent->nr);
    p[502] = BYTE1(parent->nr);
    p[503] = BYTE0(parent->nr);
    
    // Subtype
    p[508] = 2;
        
    // Checksum
    u32 checksum = Block::checksum(p);
    p[20] = BYTE3(checksum);
    p[21] = BYTE2(checksum);
    p[22] = BYTE1(checksum);
    p[23] = BYTE0(checksum);
}
