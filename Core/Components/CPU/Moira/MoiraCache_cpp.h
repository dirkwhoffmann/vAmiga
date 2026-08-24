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
    
    for (int i = 0; i < iCache.numCacheLines; i++) {
        iCache.cache[i].valid = false;
    }
}

void
Moira::flushInstructionLatch()
{
    iCache.latch.addr = 0xffffffff;
    iCache.latch.data = 0xffffffff;
}

bool
Moira::fillInstructionCache(u32 addr)
{
    auto base  = addr & addrMask() & ~3;
    auto index = (addr & 0x000C) >> 2;
    auto tag   = (addr & 0xFFF0);
    
    iCache.latch.addr = base;
    
    if (iCache.cache[index].tag == tag && iCache.cache[index].valid) {
        
        //
        // Cache hit
        //
        
        iCache.latch.data = iCache.cache[index].data;
        return false;
        
    } else {
        
        //
        // Cache miss
        //
        
        iCache.latch.data = read32(base);
        
        if ((reg.cacr & 1) && !(reg.cacr & 2)) {
            
            iCache.cache[index].tag   = tag;
            iCache.cache[index].data  = iCache.latch.data;
            iCache.cache[index].valid = true;
        }
        return true;
    }
}

u16
Moira::readInstructionCache(u32 addr, bool &busAccess)
{
    auto base = addr & addrMask() & ~3;

    if (iCache.latch.data == base) {
        
        // Direct hit
        busAccess = false;
        
    } else {
        
        // Query the cache
        busAccess = fillInstructionCache(base);
    }

    return (addr & 2) ? LO_WORD(iCache.latch.data) : HI_WORD(iCache.latch.data);
}
