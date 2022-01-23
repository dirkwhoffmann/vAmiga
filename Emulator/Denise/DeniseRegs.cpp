// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"

void
Denise::setDIWSTRT(u16 value)
{
    trace(DIW_DEBUG, "setDIWSTRT(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0
    
    diwstrt = value;
    isize newDiwHstrt = LO_BYTE(value);
        
    // Invalidate the horizontal coordinate if it is out of range
    if (newDiwHstrt < 2) {
        
        trace(DIW_DEBUG, "newDiwHstrt is too small\n");
        newDiwHstrt = -1;
    }
    
    /* Check if the change takes effect in the current rasterline.
     *
     *     old: Old trigger coordinate (diwHstrt)
     *     new: New trigger coordinate (newDiwHstrt)
     *     cur: Position of the electron beam (derivable from pos.h)
     *
     * The following cases have to be taken into accout:
     *
     *    1) cur < old < new : Change takes effect in this rasterline.
     *    2) cur < new < old : Change takes effect in this rasterline.
     *    3) new < cur < old : Neither the old nor the new trigger hits.
     *    4) new < old < cur : Already triggered. Nothing to do in this line.
     *    5) old < cur < new : Already triggered. Nothing to do in this line.
     *    6) old < new < cur : Already triggered. Nothing to do in this line.
     */
    
    isize cur = 2 * agnus.pos.h;
    
    // (1) and (2)
    if (cur < denise.hstrt && cur < newDiwHstrt) {
        
        trace(DIW_DEBUG, "Updating DIW hflop immediately at %ld\n", cur);
        hflopOn = newDiwHstrt;
    }
    
    // (3)
    if (newDiwHstrt < cur && cur < hstrt) {
        
        trace(DIW_DEBUG, "DIW hflop not switched on in current line\n");
        hflopOn = -1;
    }
    
    hstrt = newDiwHstrt;
}

void
Denise::setDIWSTOP(u16 value)
{
    trace(DIW_DEBUG, "setDIWSTOP(%X)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1
        
    diwstop = value;
    isize newDiwHstop = LO_BYTE(value) | 0x100;
        
    // Invalidate the coordinate if it is out of range
    if (newDiwHstop > 0x1C7) {
        trace(DIW_DEBUG, "newDiwHstop is too large\n");
        newDiwHstop = -1;
    }
    
    // Check if the change already takes effect in the current rasterline.
    isize cur = 2 * agnus.pos.h;
    
    // (1) and (2) (see setDIWSTRT)
    if (cur < hstop && cur < newDiwHstop) {
        
        trace(DIW_DEBUG, "Updating hFlopOff immediately at %ld\n", cur);
        hflopOff = newDiwHstop;
    }
    
    // (3) (see setDIWSTRT)
    if (newDiwHstop < cur && cur < hstop) {
        
        trace(DIW_DEBUG, "hFlop not switched off in current line\n");
        hflopOff = -1;
    }
    
    hstop = newDiwHstop;
}

u16
Denise::peekJOY0DATR()
{
    u16 result = controlPort1.joydat();
    trace(JOYREG_DEBUG, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

u16
Denise::peekJOY1DATR()
{
    u16 result = controlPort2.joydat();
    trace(JOYREG_DEBUG, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(u16 value)
{
    trace(JOYREG_DEBUG, "pokeJOYTEST(%04X)\n", value);

    controlPort1.pokeJOYTEST(value);
    controlPort2.pokeJOYTEST(value);
}

u16
Denise::peekDENISEID()
{
    u16 result;

    if (config.revision == DENISE_ECS) {
        result = 0xFFFC;                           // ECS
    } else {
        result = mem.peekCustomFaulty16(0xDFF07C); // OCS
    }

    trace(ECSREG_DEBUG, "peekDENISEID() = $%04X (%d)\n", result, result);
    return result;
}

template <Accessor s> void
Denise::pokeBPLCON0(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON0_DENISE, value);
}

void
Denise::setBPLCON0(u16 oldValue, u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON0(%X,%X)\n", oldValue, newValue);

    // Record the register change
    i64 pixel = std::max(4 * agnus.pos.h - 4, (isize)0);
    conChanges.insert(pixel, RegChange { SET_BPLCON0_DENISE, newValue });
    
    // Check if the HAM bit has changed
    if (ham(oldValue) ^ ham(newValue)) {
        pixelEngine.colChanges.insert(pixel, RegChange { 0x100, newValue } );
    }
    
    // Update value
    bplcon0 = newValue;
    
    // Update border color index, because the ECSENA bit might have changed
    updateBorderColor();
    
    // Check if the BPU bits have changed
    u16 oldBpuBits = (oldValue >> 12) & 0b111;
    u16 newBpuBits = (newValue >> 12) & 0b111;

    if (oldBpuBits != newBpuBits) {
        // trace("Changing BPU bits from %d to %d\n", oldBpuBits, newBpuBits);
    }
    
    // Report a suspicious BPU value
    if (newBpuBits > (hires(bplcon0) ? 4 : 6)) {
        trace(XFILES, "XFILES (BPLCON0): BPU = %d\n", newBpuBits);
    }
}

template <Accessor s> void
Denise::pokeBPLCON1(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON1(%X)\n", value);

    // Record the register change
    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON1_DENISE, value);
}

void
Denise::setBPLCON1(u16 oldValue, u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON1(%X)\n", newValue);

    bplcon1 = newValue & 0xFF;

    pixelOffsetOdd  = (i8)((bplcon1 & 0b00000001) << 1);
    pixelOffsetEven = (i8)((bplcon1 & 0b00010000) >> 3);
}

template <Accessor s> void
Denise::pokeBPLCON2(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON2(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON2, value);
}

void
Denise::setBPLCON2(u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON2(%X)\n", newValue);

    bplcon2 = newValue;

    if (pf1px() > 4) { trace(XFILES, "XFILES (BPLCON2): PF1P = %d\n", pf1px()); }
    if (pf2px() > 4) { trace(XFILES, "XFILES (BPLCON2): PF2P = %d\n", pf2px()); }
    
    // Record the register change
    i64 pixel = 4 * agnus.pos.h + 4;
    conChanges.insert(pixel, RegChange { SET_BPLCON2, newValue });    
}

template <Accessor s> void
Denise::pokeBPLCON3(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON3(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON3, value);
}

void
Denise::setBPLCON3(u16 value)
{
    trace(BPLREG_DEBUG, "setBPLCON3(%X)\n", value);

    bplcon3 = value;
    
    // Update border color index, because the BRDRBLNK bit might have changed
    updateBorderColor();
}

u16
Denise::peekCLXDAT()
{
    u16 result = clxdat | 0x8000;
    clxdat = 0;
    
    trace(CLXREG_DEBUG, "peekCLXDAT() = %x\n", result);
    return result;
}

void
Denise::pokeCLXCON(u16 value)
{
    trace(CLXREG_DEBUG, "pokeCLXCON(%x)\n", value);
    clxcon = value;
}

template <isize x, Accessor s> void
Denise::pokeBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLREG_DEBUG, "pokeBPL%ldDAT(%X)\n", x + 1, value);

    if constexpr (s == ACCESSOR_AGNUS) {
        /*
        debug("BPL%dDAT written by Agnus (%x)\n", x, value);
        */
    }
    
    setBPLxDAT<x>(value);
}

template <isize x> void
Denise::setBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLDAT_DEBUG, "setBPL%ldDAT(%X)\n", x + 1, value);

    bpldat[x] = value;

    if constexpr (x == 0) {
        
        // Feed data registers into pipe
        for (isize i = 0; i < 6; i++) bpldatPipe[i] = bpldat[i];

        armedOdd = true;
        armedEven = true;
        
        spriteClipBegin = std::min(spriteClipBegin,
                                   (Pixel)((agnus.pos.h + 1) * 4));
    }
}

template <isize x> void
Denise::pokeSPRxPOS(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0POS + x, value } );
}

template <isize x> void
Denise::pokeSPRxCTL(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldCTL(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
    
    // Disarm the sprite
    // CLR_BIT(armed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0CTL + x, value } );
}

template <isize x> void
Denise::pokeSPRxDATA(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldDATA(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;
    
    // Remember that the sprite was armed at least once in this rasterline
    SET_BIT(wasArmed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATA + x, value } );
}

template <isize x> void
Denise::pokeSPRxDATB(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldDATB(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATB + x, value });
}

template <Accessor s, isize xx> void
Denise::pokeCOLORxx(u16 value)
{
    trace(COLREG_DEBUG, "pokeCOLOR%02ld(%X)\n", xx, value);

    u32 reg = 0x180 + 2*xx;
    isize pos = agnus.pos.h;

    if constexpr (s == ACCESSOR_CPU) {

        // If the CPU writes, the change takes effect one DMA cycle earlier
        if (agnus.pos.h != 0) pos--;
    }
    
    // Record the color change
    pixelEngine.colChanges.insert(4 * pos, RegChange { reg, value } );
}

u16
Denise::zPF(u16 prioBits)
{
    switch (prioBits) {

        case 0: return Z_0;
        case 1: return Z_1;
        case 2: return Z_2;
        case 3: return Z_3;
        case 4: return Z_4;
    }

    return 0;
}

u8
Denise::bpu(u16 v)
{
    // Extract the three BPU bits
    u8 bpu = (v >> 12) & 0b111;
    
    // An invalid value enables all 6 planes
    return  bpu < 7 ? bpu : 6;
}

template void Denise::pokeBPLCON0<ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLCON0<ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLCON1<ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLCON1<ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLCON2<ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLCON2<ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLCON3<ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLCON3<ACCESSOR_AGNUS>(u16 value);

template void Denise::pokeBPLxDAT<0,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<0,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<1,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<1,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<2,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<2,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<3,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<3,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<4,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<4,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<5,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<5,ACCESSOR_AGNUS>(u16 value);

template void Denise::setBPLxDAT<0>(u16 value);
template void Denise::setBPLxDAT<1>(u16 value);
template void Denise::setBPLxDAT<2>(u16 value);
template void Denise::setBPLxDAT<3>(u16 value);
template void Denise::setBPLxDAT<4>(u16 value);
template void Denise::setBPLxDAT<5>(u16 value);

template void Denise::pokeSPRxPOS<0>(u16 value);
template void Denise::pokeSPRxPOS<1>(u16 value);
template void Denise::pokeSPRxPOS<2>(u16 value);
template void Denise::pokeSPRxPOS<3>(u16 value);
template void Denise::pokeSPRxPOS<4>(u16 value);
template void Denise::pokeSPRxPOS<5>(u16 value);
template void Denise::pokeSPRxPOS<6>(u16 value);
template void Denise::pokeSPRxPOS<7>(u16 value);

template void Denise::pokeSPRxCTL<0>(u16 value);
template void Denise::pokeSPRxCTL<1>(u16 value);
template void Denise::pokeSPRxCTL<2>(u16 value);
template void Denise::pokeSPRxCTL<3>(u16 value);
template void Denise::pokeSPRxCTL<4>(u16 value);
template void Denise::pokeSPRxCTL<5>(u16 value);
template void Denise::pokeSPRxCTL<6>(u16 value);
template void Denise::pokeSPRxCTL<7>(u16 value);

template void Denise::pokeSPRxDATA<0>(u16 value);
template void Denise::pokeSPRxDATA<1>(u16 value);
template void Denise::pokeSPRxDATA<2>(u16 value);
template void Denise::pokeSPRxDATA<3>(u16 value);
template void Denise::pokeSPRxDATA<4>(u16 value);
template void Denise::pokeSPRxDATA<5>(u16 value);
template void Denise::pokeSPRxDATA<6>(u16 value);
template void Denise::pokeSPRxDATA<7>(u16 value);

template void Denise::pokeSPRxDATB<0>(u16 value);
template void Denise::pokeSPRxDATB<1>(u16 value);
template void Denise::pokeSPRxDATB<2>(u16 value);
template void Denise::pokeSPRxDATB<3>(u16 value);
template void Denise::pokeSPRxDATB<4>(u16 value);
template void Denise::pokeSPRxDATB<5>(u16 value);
template void Denise::pokeSPRxDATB<6>(u16 value);
template void Denise::pokeSPRxDATB<7>(u16 value);

template void Denise::pokeCOLORxx<ACCESSOR_CPU, 0>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 0>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 1>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 1>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 2>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 2>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 3>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 3>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 4>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 4>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 5>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 5>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 6>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 6>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 7>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 7>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 8>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 8>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 9>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 9>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 10>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 10>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 11>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 11>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 12>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 12>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 13>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 13>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 14>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 14>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 15>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 15>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 16>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 16>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 17>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 17>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 18>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 18>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 19>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 19>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 20>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 20>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 21>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 21>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 22>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 22>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 23>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 23>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 24>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 24>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 25>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 25>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 26>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 26>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 27>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 27>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 28>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 28>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 29>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 29>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 30>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 30>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 31>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 31>(u16 value);
