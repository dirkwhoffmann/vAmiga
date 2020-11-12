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

#include "FSBlock.h"

struct FSFileListBlock : FSBlock {
        
    FSFileListBlock(FSVolume &ref, u32 nr);
    ~FSFileListBlock();

    FSBlockType type() override { return FS_FILELIST_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    // void exportBlock(u8 *p, size_t size) override;
    void updateChecksum() override;

    u32 getFileHeaderRef() override { return read32(data + bsize() - 12); }
    void setFileHeaderRef(u32 ref) override { write32(data + bsize() - 12, ref); }
    
    u32 getFirstDataBlockRef() override { return read32(data + 16); }
    void setFirstDataBlockRef(u32 ref) override { write32(data + 16, ref); }

    u32 getDataBlockRef(int nr) { return read32(data + bsize() - (51 + nr) * 4); }
    void setDataBlockRef(int nr, u32 ref) { write32(data + bsize() - (51 + nr) * 4, ref); }

    u32 numDataBlockRefs() override { return read32(data + 8); }
    u32 maxDataBlockRefs() override { return bsize() / 4 - 56; }
    void incDataBlockRefs() override { write32(data + 8, read32(data + 8) + 1); }

    bool addDataBlockRef(u32 first, u32 ref) override;

    u32 getNextExtBlockRef() override { return read32(data + bsize() - 8); }
    void setNextExtBlockRef(u32 ref) override { write32(data + bsize() - 8, ref); }
};

#endif
