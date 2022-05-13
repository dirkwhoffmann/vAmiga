// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Denise.h"

template <> bool Agnus::auddma<0>(u16 v) { return (v & DMAEN) && (v & AUD0EN); }
template <> bool Agnus::auddma<1>(u16 v) { return (v & DMAEN) && (v & AUD1EN); }
template <> bool Agnus::auddma<2>(u16 v) { return (v & DMAEN) && (v & AUD2EN); }
template <> bool Agnus::auddma<3>(u16 v) { return (v & DMAEN) && (v & AUD3EN); }

template <BusOwner owner> bool
Agnus::busIsFree()
{
    // Deny if the bus is in use
    if (busOwner[pos.h] != BUS_NONE) return false;

    if constexpr (owner == BUS_COPPER) {
        
        // Deny if Copper DMA is disabled
        if (!copdma()) return false;
        
        // Deny in cycle E0
        if (pos.h == 0xE0) {
         
            // If the Copper wants the bus in E0, nobody can have it
            busOwner[pos.h] = BUS_BLOCKED;
            
            return false;
        }
        
        return true;
    }
    
    if constexpr (owner == BUS_BLITTER) {
        
        // Deny if Blitter DMA is disabled
        if (!bltdma()) return false;
        
        // Deny if the CPU has precedence
        if (bls && !bltpri()) return false;
        
        return true;
    }
}

template <BusOwner owner> bool
Agnus::allocateBus()
{
    // Deny if the bus has been allocated already
    if (busOwner[pos.h] != BUS_NONE) return false;
    
    if constexpr (owner == BUS_COPPER) {
        
        // Assign bus to the Copper
        busOwner[pos.h] = BUS_COPPER;
        
        return true;
    }
    if constexpr (owner == BUS_BLITTER) {
        
        // Deny if Blitter DMA is off
        if (!bltdma()) return false;
        
        // Deny if the CPU has precedence
        if (bls && !bltpri()) return false;
        
        // Assign the bus to the Blitter
        busOwner[pos.h] = BUS_BLITTER;
        
        return true;
    }
}

u16
Agnus::doDiskDmaRead()
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (dskpt);
    dskpt += 2;

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = result;
    stats.usage[BUS_DISK]++;

    return result;
}

template <int channel> u16
Agnus::doAudioDmaRead()
{
    constexpr BusOwner owner = BusOwner(BUS_AUD0 + channel);
    
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (audpt[channel]);
    audpt[channel] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

template <int bitplane> u16
Agnus::doBitplaneDmaRead()
{
    assert(bitplane >= 0 && bitplane <= 5);
    constexpr BusOwner owner = BusOwner(BUS_BPL1 + bitplane);
    
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (bplpt[bitplane]);
    bplpt[bitplane] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

template <int channel> u16
Agnus::doSpriteDmaRead()
{
    assert(channel >= 0 && channel <= 7);
    constexpr BusOwner owner = BusOwner(BUS_SPRITE0 + channel);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (sprpt[channel]);
    sprpt[channel] += 2;

    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

u16
Agnus::doCopperDmaRead(u32 addr)
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = result;
    stats.usage[BUS_COPPER]++;

    return result;
}

u16
Agnus::doBlitterDmaRead(u32 addr)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(busOwner[pos.h] == BUS_BLITTER);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    busOwner[pos.h] = BUS_BLITTER;
    busValue[pos.h] = result;
    stats.usage[BUS_BLITTER]++;

    return result;
}

void
Agnus::doDiskDmaWrite(u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (dskpt, value);
    dskpt += 2;

    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = value;
    stats.usage[BUS_DISK]++;
}

void
Agnus::doCopperDmaWrite(u32 addr, u16 value)
{
    mem.pokeCustom16<ACCESSOR_AGNUS>(addr, value);

    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = value;
    stats.usage[BUS_COPPER]++;
}

void
Agnus::doBlitterDmaWrite(u32 addr, u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (addr, value);

    assert(busOwner[pos.h] == BUS_BLITTER); // Bus is already allocated
    busValue[pos.h] = value;
    stats.usage[BUS_BLITTER]++;
}

template u16 Agnus::doAudioDmaRead<0>();
template u16 Agnus::doAudioDmaRead<1>();
template u16 Agnus::doAudioDmaRead<2>();
template u16 Agnus::doAudioDmaRead<3>();

template u16 Agnus::doBitplaneDmaRead<0>();
template u16 Agnus::doBitplaneDmaRead<1>();
template u16 Agnus::doBitplaneDmaRead<2>();
template u16 Agnus::doBitplaneDmaRead<3>();
template u16 Agnus::doBitplaneDmaRead<4>();
template u16 Agnus::doBitplaneDmaRead<5>();

template u16 Agnus::doSpriteDmaRead<0>();
template u16 Agnus::doSpriteDmaRead<1>();
template u16 Agnus::doSpriteDmaRead<2>();
template u16 Agnus::doSpriteDmaRead<3>();
template u16 Agnus::doSpriteDmaRead<4>();
template u16 Agnus::doSpriteDmaRead<5>();
template u16 Agnus::doSpriteDmaRead<6>();
template u16 Agnus::doSpriteDmaRead<7>();

template bool Agnus::allocateBus<BUS_COPPER>();
template bool Agnus::allocateBus<BUS_BLITTER>();

template bool Agnus::busIsFree<BUS_COPPER>();
template bool Agnus::busIsFree<BUS_BLITTER>();
