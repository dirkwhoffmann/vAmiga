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

    FSBlockType type() override { return FS_FILELIST_BLOCK;   }
    FSItemType itemType(u32 byte) override;
    FSError check(u32 pos, u8 *expected, bool strict) override;
    void dump() override;
    u32 checksumLocation() override { return 5; }

    
    //
    // Accessing block items
    //
    
    u32 getNumDataBlockRefs() override          { return get32(2);            }
    void setNumDataBlockRefs(u32 val) override  {           set32(2, val);    }
    void incNumDataBlockRefs() override         {        inc32(2);            }

    u32 getFirstDataBlockRef() override         { return get32(4);            }
    void setFirstDataBlockRef(u32 ref) override {        set32(4, ref);       }

    u32 getDataBlockRef(int nr)                 { return get32(-51-nr);       }
    void setDataBlockRef(int nr, u32 ref)       {        set32(-51-nr, ref);  }

    u32 getFileHeaderRef() override             { return get32(-3);           }
    void setFileHeaderRef(u32 ref) override     {        set32(-3, ref);      }
    
    u32 getNextListBlockRef() override          { return get32(-2);           }
    void setNextListBlockRef(u32 ref) override  {        set32(-2, ref);      }
    
    bool addDataBlockRef(u32 first, u32 ref) override;
};

#endif
