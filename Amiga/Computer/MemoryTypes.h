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
    MEM_OCS,
    MEM_AUTOCONF,
    MEM_KICK,
    MEM_WOM,
    MEM_EXTROM
}
MemorySource;

static inline bool isMemorySource(long value) { return value >= 0 && value <= MEM_EXTROM; }

// Known Roms
typedef enum
{
    ROM_UNKNOWN,

    // Boot Roms
    ROM_BOOT_A1000_1985_8K,
    ROM_BOOT_A1000_1985_64K,

    // Kickstart Roms
    ROM_AROS,

    ROM_KICK12_33_180,
    ROM_KICK12_33_180_O,

    ROM_KICK13_35_5,
    ROM_KICK13_35_5_B,
    ROM_KICK13_35_5_B2,
    ROM_KICK13_35_5_B3,
    ROM_KICK13_35_5_H,
    ROM_KICK13_35_5_O,

    ROM_KICK204_37_175,
    ROM_KICK31_40_63,
    ROM_KICK31_40_63_A,
    ROM_KICK31_40_63_B,

    ROM_KICK314_46_143,

    ROM_DIAG11,
    ROM_LOGICA20
}
RomRevision;

static inline bool isRomRevision(long value) { return value >= 0 && value <= ROM_LOGICA20; }

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
    size_t bootRomSize;
    size_t kickRomSize;
    size_t womSize;
    size_t extRomSize;
    size_t chipRamSize;
    size_t slowRamSize;
    size_t fastRamSize;
}
MemoryConfig;

typedef struct
{
    long chipReads;
    long chipWrites;
    long fastReads;
    long fastWrites;
    long romReads;
    long romWrites;
}
MemoryStats;

#endif
