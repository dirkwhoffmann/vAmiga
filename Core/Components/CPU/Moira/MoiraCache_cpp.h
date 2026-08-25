// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

void
Moira::flushInstructionCache()
{
    flushInstructionLatch();
    
    for (int i = 0; i < iCache.lineCnt; i++) {
        iCache.cache[i].valid = false;
    }
}

void
Moira::flushInstructionLatch()
{
    iCache.latch.addr = 0xffffffff;
    iCache.latch.data = 0xffffffff;
}

void
Moira::invalidateCacheEntry(u32 addr)
{
    iCache.cache[cacheIdx(addr)].valid = false;
}

u32 Moira::cacheIdx(u32 addr) const
{
    return (addr & InstructionCache::idxMask) >> 2;
}

u32 Moira::cacheTag(u32 addr) const
{
    return (addr & InstructionCache::tagMask) | u32(reg.sr.s);
}

bool
Moira::fillInstructionCache(u32 addr)
{
    auto base = addr & addrMask() & ~3;
    auto idx  = cacheIdx(base);
    auto tag  = cacheTag(base);
    
    /* The cache is only consulted while it is enabled. Bit 0 (E) gates the
     * lookup, not just the fill: clearing it has to stop valid lines from
     * being served, or the CPU would keep executing stale opcodes after a
     * program switches the cache off. Bit 1 (F) is deliberately not part of
     * this condition. Freezing only prevents new entries from being
     * allocated; entries that are already valid keep hitting. Both match
     * fill_icache020() in WinUAE / Amiberry.
     */
    if ((reg.cacr & 1) && iCache.cache[idx].valid && iCache.cache[idx].tag == tag) {
        
        //
        // Cache hit
        //
        
        iCache.latch.data = iCache.cache[idx].data;
        iCache.latch.addr = base;
        return false;
        
    } else {
        
        //
        // Cache miss
        //
        
        /* Filling a cache line is a longword fetch, so it is split into as
         * many bus cycles as the addressed port provides (see dsack). The
         * cycle penalty for a narrow port is accounted for by the caller.
         */
        u32 data;
        
        switch (portSize(dsack(base))) {
                
            case 4:  data = read32(base); break;
            case 2:  data = u32(read16(base)) << 16 | read16(base + 2); break;
            default: data = u32(read8(base))  << 24 | u32(read8(base + 1)) << 16 |
                            u32(read8(base + 2)) << 8 | u32(read8(base + 3)); break;
        }
        
        if ((reg.cacr & 1) && !(reg.cacr & 2)) {
            
            iCache.cache[idx].tag   = tag;
            iCache.cache[idx].data  = data;
            iCache.cache[idx].valid = true;
        }
        
        /* Updated last, so that a read32 that throws leaves the latch
         * untouched instead of marking stale data as valid.
         */
        iCache.latch.data = data;
        iCache.latch.addr = base;
        return true;
    }
}

u16
Moira::readInstructionCache(u32 addr, bool &busAccess)
{
    auto base = addr & addrMask() & ~3;

    if (iCache.latch.addr == base) {
        
        // Direct hit
        busAccess = false;
        
    } else {
        
        // Query the cache
        busAccess = fillInstructionCache(base);
    }

    return (addr & 2) ? LO_WORD(iCache.latch.data) : HI_WORD(iCache.latch.data);
}
