// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"

struct FSDataBlock : FSBlock {
      
    FSDataBlock(FSPartition &p, Block nr, FSBlockType t);
    ~FSDataBlock();
    
    
    //
    // Methods from Block class
    //
    
    virtual isize writeData(FILE *file, isize size) = 0;
};

struct OFSDataBlock : FSDataBlock {

    OFSDataBlock(FSPartition &p, u32 nr, FSBlockType t);

    Block  getFileHeaderRef() const override        { return get32(1);        }
    void setFileHeaderRef(Block ref) override       {        set32(1, ref);   }

    Block  getNextDataBlockRef() const override     { return get32(4);        }
    void setNextDataBlockRef(Block ref) override    {        set32(4, ref);   }

    isize writeData(FILE *file, isize size) override;
    isize addData(const u8 *buffer, isize size) override;
};

struct FFSDataBlock : FSDataBlock {
      
    FFSDataBlock(FSPartition &p, u32 nr, FSBlockType t);

    const char *getDescription() const override { return "FFSDataBlock"; }

    isize writeData(FILE *file, isize size) override;
    isize addData(const u8 *buffer, isize size) override;
};
