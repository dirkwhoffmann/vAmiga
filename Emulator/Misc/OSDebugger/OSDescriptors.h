// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Exception.h"
#include <vector>

#define HUNK_UNIT       999
#define HUNK_NAME       1000
#define HUNK_CODE       1001
#define HUNK_DATA       1002
#define HUNK_BSS        1003
#define HUNK_RELOC32    1004
#define HUNK_ABSRELOC32 HUNK_RELOC32
#define HUNK_RELOC16    1005
#define HUNK_RELRELOC16 HUNK_RELOC16
#define HUNK_RELOC8     1006
#define HUNK_RELRELOC8  HUNK_RELOC8
#define HUNK_EXT        1007
#define HUNK_SYMBOL     1008
#define HUNK_DEBUG      1009
#define HUNK_END        1010
#define HUNK_HEADER     1011
#define HUNK_OVERLAY    1013
#define HUNK_BREAK      1014
#define HUNK_DREL32     1015
#define HUNK_DREL16     1016
#define HUNK_DREL8      1017
#define HUNK_LIB        1018
#define HUNK_INDEX      1019

#define MEMF_ANY        0
#define MEMF_PUBLIC     (1 << 0)
#define MEMF_CHIP       (1 << 1)
#define MEMF_FAST       (1 << 2)

struct HunkTypeEnum { static string key(u32 value); };
struct MemFlagsEnum { static string key(u32 value); };

struct HunkDescriptor {
  
    // Hunk type
    u32 type = 0;
    
    // First byte belonging to this hunk
    u32 offset = 0;
    
    // Hunk size in bytes
    u32 size = 0;
    
    // Memory size in bytes
    u32 memSize = 0;
    
    // Memory flags
    u32 memFlags = 0;
    
    
    //
    // Printing debug information
    //
    
    void dump() const;
    void dump(std::ostream& os) const;
};

struct ExeDescriptor {

    // The hunk list
    std::vector <HunkDescriptor> hunks;
    
    //
    // Initializing
    //
    
    ExeDescriptor(const u8 *buf, isize len) throws { init(buf, len); }
    void init(const u8 *buf, isize len) throws;
    
    //
    // Querying hunk information
    //
    
    isize numHunks() const { return isize(hunks.size()); }
    
    
    //
    // Printing debug information
    //
    
    void dump() const;
    void dump(std::ostream& os) const;
    
};
