// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "Agnus.h"
#include "Denise.h"

namespace vamiga {

template <> bool Agnus::auddma<0>(u16 v) { return (v & DMAEN) && (v & AUD0EN); }
template <> bool Agnus::auddma<1>(u16 v) { return (v & DMAEN) && (v & AUD1EN); }
template <> bool Agnus::auddma<2>(u16 v) { return (v & DMAEN) && (v & AUD2EN); }
template <> bool Agnus::auddma<3>(u16 v) { return (v & DMAEN) && (v & AUD3EN); }

template <BusOwner owner> bool
Agnus::busIsFree()
{
    // Deny if the bus is in use
    if (busOwner[pos.h] != BusOwner::NONE) return false;

    if constexpr (owner == BusOwner::COPPER) {
        
        // Deny if Copper DMA is disabled
        if (!copdma()) return false;
        
        // Deny in cycle E0 / E1
        if (pos.h == (pos.lol ? 0xE1 : 0xE0)) {

            // If the Copper wants the bus in E0, nobody can have it
            busOwner[pos.h] = BusOwner::BLOCKED;
            
            return false;
        }
        
        return true;
    }
    
    if constexpr (owner == BusOwner::BLITTER) {
        
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
    if (busOwner[pos.h] != BusOwner::NONE) return false;
    
    if constexpr (owner == BusOwner::COPPER) {
        
        // Assign bus to the Copper
        busOwner[pos.h] = BusOwner::COPPER;
        
        return true;
    }
    if constexpr (owner == BusOwner::BLITTER) {
        
        // Deny if Blitter DMA is off
        if (!bltdma()) return false;
        
        // Deny if the CPU has precedence
        if (bls && !bltpri()) return false;
        
        // Assign the bus to the Blitter
        busOwner[pos.h] = BusOwner::BLITTER;
        
        return true;
    }
}

u16
Agnus::doDiskDmaRead()
{
    u16 result = mem.peek16 <Accessor::AGNUS> (dskpt);

    busOwner[pos.h] = BusOwner::DISK;
    busAddr[pos.h] = dskpt;
    busData[pos.h] = result;
    stats.usage[BUS_DISK]++;

    dskpt += 2;
    return result;
}

template <int channel> u16
Agnus::doAudioDmaRead()
{
    constexpr BusOwner owner = BusOwner(BUS_AUD0 + channel);
    
    u16 result = mem.peek16 <Accessor::AGNUS> (audpt[channel]);
    
    busOwner[pos.h] = owner;
    busAddr[pos.h] = audpt[channel];
    busData[pos.h] = result;
    stats.usage[isize(owner)]++;

    audpt[channel] += 2;
    return result;
}

template <int bitplane> u16
Agnus::doBitplaneDmaRead()
{
    assert(bitplane >= 0 && bitplane <= 5);
    constexpr BusOwner owner = BusOwner(BUS_BPL1 + bitplane);
    
    u16 result = mem.peek16 <Accessor::AGNUS> (bplpt[bitplane]);

    busOwner[pos.h] = owner;
    busAddr[pos.h] = bplpt[bitplane];
    busData[pos.h] = result;
    stats.usage[isize(owner)]++;

    bplpt[bitplane] += 2;
    return result;
}

template <int channel> u16
Agnus::doSpriteDmaRead()
{
    assert(channel >= 0 && channel <= 7);
    constexpr BusOwner owner = BusOwner(BUS_SPRITE0 + channel);

    u16 result = mem.peek16 <Accessor::AGNUS> (sprpt[channel]);

    busOwner[pos.h] = owner;
    busAddr[pos.h] = sprpt[channel];
    busData[pos.h] = result;
    stats.usage[isize(owner)]++;

    sprpt[channel] += 2;
    return result;
}

u16
Agnus::doCopperDmaRead(u32 addr)
{
    u16 result = mem.peek16 <Accessor::AGNUS> (addr);

    busOwner[pos.h] = BusOwner::COPPER;
    busAddr[pos.h] = addr;
    busData[pos.h] = result;
    stats.usage[BUS_COPPER]++;

    return result;
}

u16
Agnus::doBlitterDmaRead(u32 addr)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(busOwner[pos.h] == BusOwner::BLITTER);

    u16 result = mem.peek16 <Accessor::AGNUS> (addr);

    busOwner[pos.h] = BusOwner::BLITTER;
    busAddr[pos.h] = addr;
    busData[pos.h] = result;
    stats.usage[BUS_BLITTER]++;

    return result;
}

void
Agnus::doDiskDmaWrite(u16 value)
{
    mem.poke16 <Accessor::AGNUS> (dskpt, value);
    
    busOwner[pos.h] = BusOwner::DISK;
    busAddr[pos.h] = dskpt;
    busData[pos.h] = value;
    stats.usage[BUS_DISK]++;

    dskpt += 2;
}

void
Agnus::doCopperDmaWrite(u32 addr, u16 value)
{
    mem.pokeCustom16<Accessor::AGNUS>(addr, value);

    busOwner[pos.h] = BusOwner::COPPER;
    busAddr[pos.h] = addr;
    busData[pos.h] = value;
    stats.usage[BUS_COPPER]++;
}

void
Agnus::doBlitterDmaWrite(u32 addr, u16 value)
{
    mem.poke16 <Accessor::AGNUS> (addr, value);

    assert(busOwner[pos.h] == BusOwner::BLITTER); // Bus is already allocated
    busAddr[pos.h] = addr;
    busData[pos.h] = value;
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

template bool Agnus::allocateBus<BusOwner::COPPER>();
template bool Agnus::allocateBus<BusOwner::BLITTER>();

template bool Agnus::busIsFree<BusOwner::COPPER>();
template bool Agnus::busIsFree<BusOwner::BLITTER>();

}
