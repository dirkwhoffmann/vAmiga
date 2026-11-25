// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include <cassert>
#include "../MoiraConfig.h"
#include "../Moira.h"

namespace MOIRA_NAMESPACE {

void
Cache::reset(Layout l)
{
    layout = l;
    flush();
}

void
Cache::reset(CPUModel model)
{
    reset(model >= CPUModel::M68EC030 ? Layout::M68030 : Layout::M68020);
}

bool
Cache::hit(u32 addr, bool supervisor) const
{
    return valid[slot(addr)] && tags[index(addr)] == tag(addr, supervisor);
}

void
Cache::invalidateLine(u32 addr)
{
    for (int i = 0; i < numLongs(); i++) valid[first(addr) + i] = false;
}

void
Cache::insert(u32 addr, bool supervisor, u32 value)
{
    auto t = tag(addr, supervisor);

    // Invalidate the entire line if the tag differs
    if (tags[index(addr)] != t) invalidateLine(addr);

    tags[index(addr)] = t;
    data[slot(addr)] = value;
    valid[slot(addr)] = true;
}

int
Cache::validCount(u32 addr) const
{
    int count = 0;
    for (int i = 0; i < numLongs(); i++) count += valid[first(addr) + i];
    return count;
}

void
Cache::flush()
{
    for (auto &v : valid) v = false;
}

void
Cache::copyContents(const Cache &other)
{
    for (int i = 0; i < capacity; i++) {

        data[i] = other.data[i];
        tags[i] = other.tags[i];
        valid[i] = other.valid[i];
    }
}

void
Cache::invalidate(u32 addr)
{
    valid[slot(addr)] = false;
}

void
Cache::burstFill(u32 logical, u32 physical, bool supervisor)
{
    // A burst fill loads the remaining longwords of the line right behind the
    // one that has just been loaded. The MMU translates the first longword.
    // All others follow in the same physical line. The longword index wraps
    // around within the line.
    
    auto lbase = logical & ~15;
    auto pbase = physical & ~15;
    auto lw = longword(logical);

    for (int i = 0; i < 3; i++) {

        lw = (lw + 1) & 3;
        insert(lbase + 4 * lw, supervisor, moira.read32((pbase + 4 * lw) & moira.getAddrMask()));
    }
}

void
ICache::refresh()
{
    active = emulated && (moira.reg.cacr & CACR::E);
    frozen = moira.reg.cacr & CACR::F;
}

bool
ICache::burstEnabled(u32 phys) const
{
    return getLayout() == Layout::M68030 && (moira.reg.cacr & CACR::IBE) && portSize(moira.dsack(phys)) == 4;
}

void
ICache::flush()
{
    Cache::flush();
    flushLatch();
}

void
ICache::cacrDidChange(u32 value)
{
    // Setting the CE bit invalidates the entry for the address in CAAR
    if (value & CACR::CE) invalidate(moira.reg.caar);

    // Setting the C bit invalidates all entries
    if (value & CACR::C) flush();
}

void
ICache::caarDidChange(u32 value)
{

}

bool
ICache::fill(u32 base, u32 &phys)
{
    bool supervisor = moira.reg.sr.s;
    phys = base;


    if (active && hit(base, supervisor)) {

        //
        // Cache hit
        //

        latch = { base, data[slot(base)] };
        return false;
    
    } else {
        
        //
        // Cache miss
        //
        
        bool ci;
        
        // Read new value from memory
        phys = moira.translate<Core::C68020, false, Word>(base, moira.getFC(), &ci) & moira.getAddrMask() & ~3;
        u32 value = moira.readLongSplit<Core::C68020>(phys);
        
        // Update the cache
        if (active && !frozen && !ci) {
            
            insert(base, supervisor, value);
            
            if (burstEnabled(phys) && validCount(base) == 1) {
                burstFill(base, phys, supervisor);
            }
        }
     
        latch = { base, value };
        return true;
    }
}

u16
ICache::fetch(u32 addr, bool &busAccess, u32 &phys)
{
    assert(emulated);

    auto base = addr & moira.getAddrMask() & ~3;

    // Check if the latch provides the information we need
    if (latch.addr == base) {

        busAccess = false;
        phys = addr;

    } else {

        busAccess = fill(base, phys);
    }

    return (addr & 2) ? u16(latch.data & 0xFFFF) : u16(latch.data >> 16);
}

void
DCache::refresh()
{
    active = emulated && (moira.reg.cacr & CACR::ED);
    frozen = moira.reg.cacr & CACR::FD;
}

bool
DCache::burstEnabled(u32 phys) const
{
    return getLayout() == Layout::M68030 && (moira.reg.cacr & CACR::DBE) && portSize(moira.dsack(phys)) == 4;
}

void
DCache::cacrDidChange(u32 value)
{
    if (!emulated) return;

    // Setting the CED bit invalidates the entry for the address in CAAR
    if (value & CACR::CED) invalidate(moira.reg.caar);

    // Setting the CD bit invalidates all entries
    if (value & CACR::CD) flush();
}

void
DCache::caarDidChange(u32 value)
{

}

bool
DCache::read(u32 addr, u8 fc, u32 &result)
{
    bool supervisor = fc & 4;

    // Serve the longword from the cache if possible
    if (hit(addr, supervisor)) {
        
        result = data[slot(addr)];
        return true;
    }

    // On a miss, the MMU provides the physical address and the cache-inhibit attribute
    bool ci;
    u32 phys = moira.translate<Core::C68020, false, Long>(addr, fc, &ci) & moira.getAddrMask() & ~3;
    if (ci) return false;

    // Load the entire longword, regardless of the size of the access
    result = moira.readLongSplit<Core::C68020>(phys);

    if (!frozen) {

        insert(addr, supervisor, result);

        if (burstEnabled(phys) && validCount(addr) == 1) {
            burstFill(addr, phys, supervisor);
        }
    }
    return true;
}

void
DCache::write(u32 addr, bool supervisor, u32 mask, u32 value, bool ci)
{
    bool writeAllocate = moira.reg.cacr & CACR::WA;
    bool full = mask == 0xFFFFFFFF;

    if (hit(addr, supervisor)) {

        // Drop the entry for a full write to an inhibited page
        if (ci && full && writeAllocate) {
            
            invalidate(addr);
            return;
        }
     
        // Update the cache entry by replacing the bits selected by the mask
        auto &d = data[slot(addr)];
        d = (d & ~mask) | (value & mask);

    } else if (writeAllocate && !frozen && !ci && full) {

        // An aligned longword write creates an entry
        insert(addr, supervisor, value);
    }
}

}
