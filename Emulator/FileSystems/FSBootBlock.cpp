// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"
#include "BootBlocks.h"

FSBootBlock::FSBootBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
    
    if (nr == p.firstBlock && p.dos != FS_NODOS) {
        data[0] = 'D';
        data[1] = 'O';
        data[2] = 'S';
        data[3] = (u8)p.dos;
    }
}

FSBootBlock::~FSBootBlock()
{
    delete [] data;
}


FSVolumeType
FSBootBlock::dos()
{
    // Only proceed if the header begins with 'DOS'
    if (strncmp((const char *)data, "DOS", 3)) return FS_NODOS;
        
    // Only proceed if the DOS version number is valid
    if (data[3] > 7) return FS_NODOS;
    
    return (FSVolumeType)data[3];
}

FSItemType
FSBootBlock::itemType(u32 byte)
{
    if (byte <= 3 && nr == 0) {
        
        switch(byte) {
            case 0:
            case 1:
            case 2: return FSI_DOS_HEADER;
            case 3: return FSI_DOS_VERSION;
        }
    }
    
    return FSI_BOOTCODE;
}

FSError
FSBootBlock::check(u32 byte, u8 *expected, bool strict)
{
    if (byte <= 3 && nr == 0) {
        
        u8 value = data[byte];
        
        switch(byte) {
            case 0: EXPECT_BYTE('D'); break;
            case 1: EXPECT_BYTE('O'); break;
            case 2: EXPECT_BYTE('S'); break;
            case 3: EXPECT_DOS_REVISION; break;
        }
    }
    
    return FS_OK;
}

void
FSBootBlock::dump()
{
    msg("     Header : ");
    for (int i = 0; i < 8; i++) msg("%02X ", data[i]);
    msg("\n");
}

void
FSBootBlock::writeBootCode(FSBootCode bootCode, int page)
{
    assert(page == 0 || page == 1);
    
    const u8 *code;
    size_t size;
    
    switch (bootCode) {
            
        case FS_BB_KICK_1_3:
            code = os13_bb; size = sizeof(os13_bb); break;
            
        case FS_BB_KICK_2_0:
            code = os20_bb; size = sizeof(os20_bb); break;
            
        case FS_BB_SCA_VIRUS:
            code = sca_virus_bb; size = sizeof(sca_virus_bb); break;

        case FS_BB_BYTE_BANDIT_VIRUS:
            code = bbandit_virus_bb; size = sizeof(bbandit_virus_bb); break;

        default:
            warn("Invalid boot block ID %d\n", bootCode);
            return;
    }
    
    if (page == 0) {
        
        memset(data + 4, 0, bsize() - 4);
        memcpy(data + 4, code + 4, MIN(bsize() - 4, size));
        
    } else {
        
        memset(data, 0, bsize());
        if (size > bsize()) memcpy(data, code + bsize(), size - bsize());
    }
}
