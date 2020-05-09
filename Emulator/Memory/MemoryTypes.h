// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _MEMORY_T_INC
#define _MEMORY_T_INC

/* Memory source identifiers
 * The identifiers are used in the mem source lookup table to specify the
 * source and target of a peek or poke operation, respectively.
 */
typedef enum
{
    MEM_UNMAPPED,
    MEM_CHIP,
    MEM_FAST,
    MEM_SLOW,
    MEM_CIA,
    MEM_RTC,
    MEM_CUSTOM,
    MEM_AUTOCONF,
    MEM_ROM,
    MEM_WOM,
    MEM_EXT
}
MemorySource;

static inline bool isMemorySource(long value) { return value >= 0 && value <= MEM_EXT; }

// Known Roms
typedef enum
{
    ROM_MISSING,
    ROM_UNKNOWN,

    // Boot Roms (A1000)
    ROM_BOOT_A1000_8K,
    ROM_BOOT_A1000_64K,

    // Kickstart V1.x
    ROM_KICK11_31_034,
    ROM_KICK12_33_166,
    ROM_KICK12_33_180,
    ROM_KICK121_34_004,
    ROM_KICK13_34_005,

    // Kickstart V2.x
    ROM_KICK20_36_028,
    ROM_KICK202_36_207,
    ROM_KICK204_37_175,
    ROM_KICK205_37_299,
    ROM_KICK205_37_300,
    ROM_KICK205_37_350,

    // Kickstart V3.x
    ROM_KICK30_39_106,
    ROM_KICK31_40_063,

    // Hyperion
    ROM_HYP314_46_143,

    // Free Kickstart Rom replacements
    ROM_AROS_55696,
    ROM_AROS_55696_EXT,

    // Diagnostic cartridges
    ROM_DIAG11,
    ROM_DIAG12,
    ROM_LOGICA20,

    ROM_CNT
}
RomRevision;

static inline bool isRomRevision(long value) { return value >= 0 && value <= ROM_CNT; }

/* Poke source.
 * Some poke methods need to know who called them.
 */
typedef enum
{
    POKE_CPU,
    POKE_COPPER,
    POKE_SOURCE_COUNT
}
PokeSource;

static inline bool isPokeSource(long value) { return value >= 0 && value < POKE_SOURCE_COUNT; }

inline const char *pokeSourceName(PokeSource source)
{
    return
    source == POKE_CPU ? "CPU" :
    source == POKE_COPPER ? "Copper" : "???";
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
