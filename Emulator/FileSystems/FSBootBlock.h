// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BOOT_BLOCK_H
#define _FS_BOOT_BLOCK_H

#include "FSBlock.h"

struct FSBootBlock : FSBlock {
        
    FSBootBlock(FSPartition &p, u32 nr);
    ~FSBootBlock();
    
    const char *getDescription() const override { return "FSBootBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BOOT_BLOCK; }
    FSVolumeType dos() override;
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    u32 checksumLocation() override;
    u32 checksum() override;
    void dump() override;
    
    
    //
    // Block specific methods
    //

    void writeBootBlock(long id, int page);
};

#endif
