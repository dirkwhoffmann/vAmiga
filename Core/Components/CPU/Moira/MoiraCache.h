// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------


// Invalidates all cache lines if the instruction cache
void flushInstructionCache();

// Clears the instruction cache latch
void flushInstructionLatch();

// Invalidates the cache entry associated with the provided address
void invalidateCacheEntry(u32 addr);

bool fillInstructionCache(u32 addr);
u16 readInstructionCache(u32 addr, bool &busAccess);
