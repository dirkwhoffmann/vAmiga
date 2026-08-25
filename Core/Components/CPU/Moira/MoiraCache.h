// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* The 68020 instruction cache
 *
 * The 68020 has a small on-chip cache for instruction fetches only; data
 * accesses always go to the bus. It holds 64 lines of one longword each, is
 * direct-mapped (no set associativity), and is indexed by the address, so
 * each of the 16K possible longword addresses maps to exactly one of the 64
 * lines
 *
 *   index = -------- -------- -------- xxxxxx--      ( (addr >> 2) & 63   )
 *   tag   = xxxxxxxx xxxxxxxx xxxxxxxx -------S      ( S | (addr & ~0xFF) )
 *
 * where S is the supervisor bit. Folding it into the tag keeps user and
 * supervisor code from aliasing the same line, mirroring the real CPU's
 * separate treatment of the two modes.
 *
 * A line becomes valid when it is filled on a miss and is tagged with the
 * address (and S bit) it was fetched for. On a later access, the line at
 * the computed index is used only if it is valid and its tag matches -
 * otherwise it's a miss and the line is refilled from the bus.
 *
 * The Cache Control Register (CACR) gates the whole mechanism:
 *
 *   Bit 0 (E)  Enable. Cleared, the cache is bypassed entirely - lookups
 *              never hit and fills never happen, so every fetch goes to
 *              the bus. This has to be checked on every lookup, not just
 *              on fill, or the CPU would keep executing stale opcodes
 *              after a program disables the cache mid-flight.
 *   Bit 1 (F)  Freeze. With E set, freezing stops new lines from being
 *              allocated on a miss, but lines that are already valid keep
 *              being served as hits.
 *   Bit 2 (CE) Clear entry, together with CAAR selecting the line.
 *   Bit 3 (C)  Clear all - invalidates every line.
 *
 * Moira additionally keeps a one-entry "latch" (CacheLatch) in front of the
 * cache array. It records the longword last fetched, keyed by its base
 * address; a hit on the latch needs no cache lookup and no bus cycle at
 * all, modeling the CPU re-reading the same longword for the high and low
 * instruction word of a single fetch, or for consecutive fetches that
 * happen to land in the same longword.
 *
 * A cache line is filled with a single 32 bit bus transaction on a 32 bit
 * port; a narrower port (see dsack) needs two or four smaller transactions.
 */

// Invalidates all cache lines and clears the latch (CACR bit 3, "clear all")
void flushInstructionCache();

// Clears the latch only, forcing the next fetch to consult the cache again
void flushInstructionLatch();

// Invalidates the single line the given address maps to (CACR bit 2, "clear entry")
void invalidateCacheEntry(u32 addr);

// Maps an address to its cache line index
u32 cacheIndex(u32 addr) const;

// Computes the tag a line must carry to be considered valid for the given address
u32 cacheTag(u32 addr) const;

// Serves addr from the cache or refills the line from the bus; returns true on a miss
bool fillInstructionCache(u32 addr);

// Reads one instruction word via the latch/cache, reporting whether a bus access was made
u16 readInstructionCache(u32 addr, bool &busAccess);
