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

struct BootBlock : Block {
  
    bool ffs;
    
    BootBlock(bool ffs = false) { this->ffs = ffs; }
    
    FSBlockType type() override { return FS_BOOT_BLOCK; }
    void write(u8 *dst) override;
};

#endif
