// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

template <int nr>
StateMachine<nr>::StateMachine(Amiga& ref) : AmigaComponent(ref)
{
    // Set description
    switch (nr) {
        case 0: setDescription("StateMachine 0"); break;
        case 1: setDescription("StateMachine 1"); break;
        case 2: setDescription("StateMachine 2"); break;
        case 3: setDescription("StateMachine 3"); break;
        default: assert(false);
    }
}

template <int nr> void
StateMachine<nr>::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    taggedSamples.clear();
    
    /* Some methods assume that the sample buffer is never empty. We assure
     * this by initializing the buffer with a dummy element.
     */
    assert(taggedSamples.isEmpty());
    taggedSamples.write( TaggedSample { 0, 0 } );
}

template <int nr> void
StateMachine<nr>::_inspect()
{
    synchronized {
        
        info.state = state;
        info.dma = AUDxON();
        info.audlenLatch = audlenLatch;
        info.audlen = audlen;
        info.audperLatch = audperLatch;
        info.audper = audper;
        info.audvolLatch = audvolLatch;
        info.audvol = audvol;
        info.auddat = auddat;
    }
}

template <int nr> void
StateMachine<nr>::_dump()
{
    printf("   State: %d\n", state);
    printf("  AUDxIP: %d\n", AUDxIP());
    printf("  AUDxON: %d\n", AUDxON());
}

template <int nr> void
StateMachine<nr>::enableDMA()
{
    debug(AUD_DEBUG, "Enable DMA\n");

    switch (state) {

         case 0b000:

             move_000_001();
             break;
     }
}

template <int nr> void
StateMachine<nr>::disableDMA()
{
    debug(AUD_DEBUG, "Disable DMA\n");

    switch (state) {

        case 0b001:

            move_001_000();
            break;

        case 0b101:

            move_101_000();
            break;
    }
}

template <int nr> bool
StateMachine<nr>::AUDxON()
{
    return agnus.auddma<nr>();
}

template <int nr> bool
StateMachine<nr>::AUDxIP()
{
    return GET_BIT(paula.intreq, 7 + nr);
}

template <int nr> void
StateMachine<nr>::AUDxIR()
{
    if (DISABLE_AUDIRQ) return;
    
    IrqSource source =
    nr == 0 ? INT_AUD0 : nr == 1 ? INT_AUD1 : nr == 2 ? INT_AUD2 : INT_AUD3;
    
    paula.scheduleIrqRel(source, DMA_CYCLES(1));
}

template <int nr> void
StateMachine<nr>::percntrld()
{
    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);

    
    u64 delay = (audperLatch == 0) ? 0x10000 : audperLatch;

    agnus.scheduleRel<slot>(DMA_CYCLES(delay), CHX_PERFIN);
}

template <int nr> void
StateMachine<nr>::pbufld1()
{
    if (AUDxAV()) {
        // debug("Volume modulation %d (%d)\n", auddat & 0x7F, (i16)auddat);
        if (nr == 0) audioUnit.channel1.pokeAUDxVOL(auddat);
        if (nr == 1) audioUnit.channel2.pokeAUDxVOL(auddat);
        if (nr == 2) audioUnit.channel3.pokeAUDxVOL(auddat);
    } else {
        buffer = auddat;
    }
}

template <int nr> void
StateMachine<nr>::pbufld2()
{
    assert(AUDxAP());
    if (nr < 3) {
        // debug("Period modulation %d\n", auddat);
        audioUnit.pokeAUDxPER(nr + 1, auddat);
    }
}

template <int nr> bool
StateMachine<nr>::AUDxAV()
{
    return (paula.adkcon >> nr) & 0x01;
}

template <int nr> bool
StateMachine<nr>::AUDxAP()
{
    return (paula.adkcon >> nr) & 0x10;
}

template <int nr> void
StateMachine<nr>::penhi()
{
    if (!enablePenhi) return;
    
    i8 sample = (i8)HI_BYTE(buffer);
    i16 scaled = sample * audvol;
    
    debug(AUD_DEBUG, "penhi: %d %d\n", sample, scaled);
    
    if (!taggedSamples.isFull()) {
        taggedSamples.write( TaggedSample { agnus.clock, scaled } );
    } else {
        debug("penhi: Sample buffer is full\n");
    }
    
    enablePenhi = false;
}

template <int nr> void
StateMachine<nr>::penlo()
{
    if (!enablePenlo) return;

    i8 sample = (i8)LO_BYTE(buffer);
    i16 scaled = sample * audvol;

    debug(AUD_DEBUG, "penlo: %d %d\n", sample, scaled);

    if (!taggedSamples.isFull()) {
        taggedSamples.write( TaggedSample { agnus.clock, scaled } );
    } else {
        debug("penlo: Sample buffer is full\n");
    }
    
    enablePenlo = false;
}

template <int nr> void
StateMachine<nr>::move_000_010() {

    debug(AUD_DEBUG, "move_000_010\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());
    assert(!AUDxIP());

    volcntrld();
    percntrld();
    pbufld1();
    AUDxIR();

    state = 0b010;
    penhi();
}

template <int nr> void
StateMachine<nr>::move_000_001() {

    debug(AUD_DEBUG, "move_000_001\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    lencntrld();
    AUDxDR();

    state = 0b001;
}

template <int nr> void
StateMachine<nr>::move_001_000() {

    debug(AUD_DEBUG, "move_001_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_001_101() {

    debug(AUD_DEBUG, "move_001_101\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    AUDxIR();
    AUDxDR();
    AUDxDSR();
    if (!lenfin()) lencount();

    state = 0b101;
}

template <int nr> void
StateMachine<nr>::move_101_000() {

    debug(AUD_DEBUG, "move_101_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_101_010() {

    debug(AUD_DEBUG, "move_101_010\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    percntrld();
    volcntrld();
    pbufld1();
    if (napnav()) AUDxDR();

    state = 0b010;
    penhi();
}

template <int nr> void
StateMachine<nr>::move_010_011() {

    debug(AUD_DEBUG, "move_010_011\n");
    
    percntrld();
    
    // Check for attach period mode
    if (AUDxAP()) {
        
        pbufld2();
        
        if (AUDxON()) {
            
            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }
            
        } else {
            
            // Additional IRQ mode action
            AUDxIR();
        }
    }

    state = 0b011;
    penlo();
}

template <int nr> void
StateMachine<nr>::move_011_000() {

    debug(AUD_DEBUG, "move_011_000\n");

    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);
    agnus.cancel<slot>();

    intreq2 = false;
    state = 0b000;
}

template <int nr> void
StateMachine<nr>::move_011_010()
{
    debug(AUD_DEBUG, "move_011_010\n");

    percntrld();
    pbufld1();
    volcntrld();
    
    if (napnav()) {

        if (AUDxON()) {

            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }

        } else {

            // Additional IRQ mode action
            AUDxIR();
        }
        
        // intreq2 = false;
    }

    state = 0b010;
    penhi();
}

template <int nr> template <SamplingMethod method> i16
StateMachine<nr>::interpolate(Cycle clock)
{
    int w  = taggedSamples.w;
    int r1 = taggedSamples.r;
    int r2 = taggedSamples.next(r1);

    assert(!taggedSamples.isEmpty());

    // Remove all outdated entries
    while (r2 != w && taggedSamples.elements[r2].tag <= clock) {
        (void)taggedSamples.read();
        r1 = r2;
        r2 = taggedSamples.next(r1);
    }

    // If the buffer contains a single element only, return that element
    if (r2 == w) {
        return taggedSamples.elements[r1].sample;
    }

    // Interpolate between position r1 and p2
    Cycle c1 = taggedSamples.elements[r1].tag;
    Cycle c2 = taggedSamples.elements[r2].tag;
    i16 s1 = taggedSamples.elements[r1].sample;
    i16 s2 = taggedSamples.elements[r2].sample;
    assert(clock >= c1 && clock < c2);

    switch (method) {

        case SMP_NONE:
        {
            return s1;
        }
        case SMP_NEAREST:
        {
            if (clock - c1 < c2 - clock) {
                return s1;
            } else {
                return s2;
            }
        }
        case SMP_LINEAR:
        {
            double dx = (double)(c2 - c1);
            double dy = (double)(s2 - s1);
            double weight = (double)(clock - c1) / dx;
            return (i16)(s1 + weight * dy);
        }
        default:
            assert(false);
            return 0;
    }
}

template StateMachine<0>::StateMachine(Amiga &ref);
template StateMachine<1>::StateMachine(Amiga &ref);
template StateMachine<2>::StateMachine(Amiga &ref);
template StateMachine<3>::StateMachine(Amiga &ref);

template AudioChannelInfo StateMachine<0>::getInfo();
template AudioChannelInfo StateMachine<1>::getInfo();
template AudioChannelInfo StateMachine<2>::getInfo();
template AudioChannelInfo StateMachine<3>::getInfo();

template void StateMachine<0>::enableDMA();
template void StateMachine<1>::enableDMA();
template void StateMachine<2>::enableDMA();
template void StateMachine<3>::enableDMA();

template void StateMachine<0>::disableDMA();
template void StateMachine<1>::disableDMA();
template void StateMachine<2>::disableDMA();
template void StateMachine<3>::disableDMA();

template bool StateMachine<0>::AUDxIP();
template bool StateMachine<1>::AUDxIP();
template bool StateMachine<2>::AUDxIP();
template bool StateMachine<3>::AUDxIP();

template bool StateMachine<0>::AUDxON();
template bool StateMachine<1>::AUDxON();
template bool StateMachine<2>::AUDxON();
template bool StateMachine<3>::AUDxON();

template void StateMachine<0>::move_000_010();
template void StateMachine<1>::move_000_010();
template void StateMachine<2>::move_000_010();
template void StateMachine<3>::move_000_010();

template void StateMachine<0>::move_001_101();
template void StateMachine<1>::move_001_101();
template void StateMachine<2>::move_001_101();
template void StateMachine<3>::move_001_101();

template void StateMachine<0>::move_010_011();
template void StateMachine<1>::move_010_011();
template void StateMachine<2>::move_010_011();
template void StateMachine<3>::move_010_011();

template void StateMachine<0>::move_101_010();
template void StateMachine<1>::move_101_010();
template void StateMachine<2>::move_101_010();
template void StateMachine<3>::move_101_010();

template void StateMachine<0>::move_011_000();
template void StateMachine<1>::move_011_000();
template void StateMachine<2>::move_011_000();
template void StateMachine<3>::move_011_000();

template void StateMachine<0>::move_011_010();
template void StateMachine<1>::move_011_010();
template void StateMachine<2>::move_011_010();
template void StateMachine<3>::move_011_010();

template i16 StateMachine<0>::interpolate<SMP_NONE>(Cycle clock);
template i16 StateMachine<1>::interpolate<SMP_NONE>(Cycle clock);
template i16 StateMachine<2>::interpolate<SMP_NONE>(Cycle clock);
template i16 StateMachine<3>::interpolate<SMP_NONE>(Cycle clock);

template i16 StateMachine<0>::interpolate<SMP_NEAREST>(Cycle clock);
template i16 StateMachine<1>::interpolate<SMP_NEAREST>(Cycle clock);
template i16 StateMachine<2>::interpolate<SMP_NEAREST>(Cycle clock);
template i16 StateMachine<3>::interpolate<SMP_NEAREST>(Cycle clock);

template i16 StateMachine<0>::interpolate<SMP_LINEAR>(Cycle clock);
template i16 StateMachine<1>::interpolate<SMP_LINEAR>(Cycle clock);
template i16 StateMachine<2>::interpolate<SMP_LINEAR>(Cycle clock);
template i16 StateMachine<3>::interpolate<SMP_LINEAR>(Cycle clock);
