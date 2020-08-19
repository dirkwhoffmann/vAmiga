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

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
typedef VA_ENUM(long, MemorySource)
{
    MEM_NONE_FAST,
    MEM_NONE_SLOW,
    MEM_CHIP,
    MEM_SLOW,
    MEM_FAST,
    MEM_CIA,
    MEM_RTC,
    MEM_CUSTOM,
    MEM_AUTOCONF,
    MEM_ROM,
    MEM_WOM,
    MEM_EXT
};

static inline bool isMemorySource(long value) { return value >= 0 && value <= MEM_EXT; }

/* Access identifiers
 * Some memory methods need to know who called them.
 */
typedef enum
{
    CPU_ACCESS,
    AGNUS_ACCESS
}
Accessor;

static inline bool isAccessor(long value) { return value >= 0 && value <= AGNUS_ACCESS; }

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
