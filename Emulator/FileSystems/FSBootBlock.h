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
    
    // Indicates if this is the first (= 0) or second (= 1) boot block
    u32 bootBlockNr;
    
    FSBootBlock(FSDevice &ref, u32 nr, FSVolumeType type);
    ~FSBootBlock();
    
    const char *getDescription() override { return "FSBootBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() override { return FS_BOOT_BLOCK; }
    FSVolumeType fileSystem() override;
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    void dump() override;
    
    
    //
    // Block specific methods
    //

    void writeBootCode();
};

#endif
