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
    MEM_BOOT,
    MEM_KICK,
    MEM_EXTROM
}
MemorySource;

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

#endif
