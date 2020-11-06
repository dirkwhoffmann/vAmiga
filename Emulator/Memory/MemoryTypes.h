// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _MEMORY_TYPES_H
#define _MEMORY_TYPES_H

#include "Aliases.h"

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
typedef VA_ENUM(long, MemorySource)
{
    MEM_NONE,
    MEM_CHIP,
    MEM_CHIP_MIRROR,
    MEM_SLOW,
    MEM_SLOW_MIRROR,
    MEM_FAST,
    MEM_CIA,
    MEM_CIA_MIRROR,
    MEM_RTC,
    MEM_CUSTOM,
    MEM_CUSTOM_MIRROR,
    MEM_AUTOCONF,
    MEM_ROM,
    MEM_ROM_MIRROR,
    MEM_WOM,
    MEM_EXT
};

static inline bool isMemorySource(long value) { return value >= 0 && value <= MEM_EXT; }

// Access identifiers. Some memory methods need to know who called them.
typedef VA_ENUM(long, Accessor)
{
    CPU_ACCESS,
    AGNUS_ACCESS
};

static inline bool isAccessor(long value) { return value >= 0 && value <= AGNUS_ACCESS; }

typedef VA_ENUM(long, BankMap)
{
    BMAP_A500,
    BMAP_A1000,
    BMAP_A2000A,
    BMAP_A2000B
};

static inline bool isBankMap(long value) { return value >= 0 && value <= BMAP_A2000B; }

// Configuration options for the initial RAM pattern
typedef VA_ENUM(long, RamInitPattern)
{
    INIT_RANDOMIZED,
    INIT_ALL_ZEROES,
    INIT_ALL_ONES
};

static inline bool isRamInitPattern(long value) { return value >= 0 && value <= INIT_ALL_ONES; }

// Configuration options for dealing with unmapped RAM
typedef VA_ENUM(long, UnmappingType)
{
    UNMAPPED_FLOATING,
    UNMAPPED_ALL_ZEROES,
    UNMAPPED_ALL_ONES
};

static inline bool isUnmappingType(long value) { return value >= 0 && value <= UNMAPPED_ALL_ONES; }

inline const char *AccessorName(Accessor accessor)
{
    return
    accessor == CPU_ACCESS ? "CPU" :
    accessor == AGNUS_ACCESS ? "Agnus" : "???";
}

typedef struct
{
    // RAM size in bytes
    size_t chipSize;
    size_t slowSize;
    size_t fastSize;

    // ROM size in bytes
    size_t romSize;
    size_t womSize;
    size_t extSize;

    // Indicates if slow Ram accesses need a free bus
    bool slowRamDelay;
    
    // Memory layout
    BankMap bankMap;
    
    // Ram contents on startup
    RamInitPattern ramInitPattern;
    
    // Specifies how to deal with unmapped memory
    UnmappingType unmappingType;
    
    // Bank map properties
    MemorySource bankD8DB;
    MemorySource bankDC;
    MemorySource bankE0E7;
    MemorySource bankF0F7;

    // First memory page where the extended ROM is blended it
    u32 extStart;
}
MemoryConfig;

typedef struct
{
    struct { long raw; double accumulated; } chipReads;
    struct { long raw; double accumulated; } chipWrites;
    struct { long raw; double accumulated; } slowReads;
    struct { long raw; double accumulated; } slowWrites;
    struct { long raw; double accumulated; } fastReads;
    struct { long raw; double accumulated; } fastWrites;
    struct { long raw; double accumulated; } kickReads;
    struct { long raw; double accumulated; } kickWrites;
}
MemoryStats;

#endif
