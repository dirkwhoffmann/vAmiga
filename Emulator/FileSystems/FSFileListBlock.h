// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_FILELIST_BLOCK_H
#define _FS_FILELIST_BLOCK_H

#include "FSFileBlock.h"

struct FSFileListBlock : FSFileBlock {
        
    FSFileListBlock(FSVolume &ref, u32 nr);

    void dump() override;
    bool check(bool verbose) override;
    void write(u8 *dst) override;
};

#endif
