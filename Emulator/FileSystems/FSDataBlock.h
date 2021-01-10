// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_DATA_BLOCK_H
#define _FS_DATA_BLOCK_H

#include "FSBlock.h"

struct FSDataBlock : FSBlock {
      
    FSDataBlock(FSPartition &p, u32 nr);
    ~FSDataBlock();
    
    
    //
    // Methods from Block class
    //

    virtual u32 getDataBlockNr() = 0;
    virtual void setDataBlockNr(u32 val) = 0;

    virtual u32  getDataBytesInBlock() = 0;
    virtual void setDataBytesInBlock(u32 val) = 0;
    
    virtual size_t writeData(FILE *file, size_t size) = 0;
    
    
    //
    // Block specific methods
    //

    // Returns the number of data bytes stored in this block
    virtual size_t dsize() = 0;
};

struct OFSDataBlock : FSDataBlock {

    static u32 headerSize() { return 24; }

    OFSDataBlock(FSPartition &p, u32 nr);

    const char *getDescription() const override { return "OFSDataBlock"; }
    FSBlockType type() override { return FS_DATA_BLOCK_OFS; }
    FSItemType itemType(u32 byte) override;
    void dump() override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    u32 checksumLocation() override { return 5; }

    u32  getFileHeaderRef() override                { return get32(1);        }
    void setFileHeaderRef(u32 ref) override         {        set32(1, ref);   }

    u32  getDataBlockNr() override                  { return get32(2);        }
    void setDataBlockNr(u32 val) override           {        set32(2, val);   }

    u32  getDataBytesInBlock() override             { return get32(3);        }
    void setDataBytesInBlock(u32 val) override      {        set32(3, val);   }

    u32  getNextDataBlockRef() override             { return get32(4);        }
    void setNextDataBlockRef(u32 ref) override      {        set32(4, ref);   }

    size_t writeData(FILE *file, size_t size) override;
    size_t addData(const u8 *buffer, size_t size) override;
    
    size_t dsize() override;
};

struct FFSDataBlock : FSDataBlock {
      
    static u32 headerSize() { return 0; }

    FFSDataBlock(FSPartition &p, u32 nr);

    const char *getDescription() const override { return "FFSDataBlock"; }
    FSBlockType type() override { return FS_DATA_BLOCK_FFS; }
    FSItemType itemType(u32 byte) override;
    void dump() override;

    u32 getDataBlockNr() override { return 0; }
    void setDataBlockNr(u32 val) override { }

    u32  getDataBytesInBlock() override { return 0; }
    void setDataBytesInBlock(u32 val) override { };

    size_t writeData(FILE *file, size_t size) override;
    size_t addData(const u8 *buffer, size_t size) override;
    
    size_t dsize() override;
};

#endif
