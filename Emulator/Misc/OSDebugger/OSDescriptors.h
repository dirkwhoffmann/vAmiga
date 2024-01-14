// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CoreObject.h"
#include "Buffer.h"
#include <vector>

using util::Buffer;

namespace vamiga {

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

struct SectionDescriptor {

    // Section type
    u32 type = 0;
    
    // First byte belonging to this section
    u32 offset = 0;
    
    // Size of this section in bytes
    u32 size = 0;
    
    // Relocation target
    isize target = 0;
    
    // Relocation symbols
    std::vector<u32> relocations;
};

struct HunkDescriptor {

    // A hunk consists of several sections
    std::vector <SectionDescriptor> sections;

    // Memory size and flags (packed format)
    u32 memRaw = 0;
    
    // Memory size in bytes (extracted from memRaw)
    u32 memSize = 0;
    
    // Memory flags (extracted from memRaw)
    u32 memFlags = 0;

    
    //
    // Querying information
    //
    
    // Returns the number of sections
    isize numSections() const { return isize(sections.size()); }
    
    // Returns the offset to the first section of a certain type
    std::optional <isize> seek(u32 type);


    //
    // Printing debug information
    //
    
    void dump(Category category) const;
    void dump(Category category, std::ostream& os) const;
};

struct ProgramUnitDescriptor {

    // A program unit consists of several hunks
    std::vector <HunkDescriptor> hunks;
    
    
    //
    // Initializing
    //
    
    ProgramUnitDescriptor(const u8 *buf, isize len) throws { init(buf, len); }
    ProgramUnitDescriptor(const Buffer<u8> &buf) throws { init(buf); }

    void init(const u8 *buf, isize len) throws;
    void init(const Buffer<u8> &buf) throws;

    
    //
    // Querying information
    //
    
    // Returns the number of hunks
    isize numHunks() const { return isize(hunks.size()); }
    
    // Returns the offset to the first section of a certain type
    std::optional <isize> seek(u32 type);
    
    
    //
    // Printing debug information
    //
    
    void dump(Category category) const;
    void dump(Category category, std::ostream& os) const;
};

}
