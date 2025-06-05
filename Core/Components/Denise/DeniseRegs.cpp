// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"

namespace vamiga {

void
Denise::setDIWSTRT(u16 value)
{
    trace(DIW_DEBUG, "setDIWSTRT(%x)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 0
    
    diwstrt = value;
    setHSTRT(LO_BYTE(value));
}

void
Denise::setDIWSTOP(u16 value)
{
    trace(DIW_DEBUG, "setDIWSTOP(%x)\n", value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- -- -- -- -- -- -- H7 H6 H5 H4 H3 H2 H1 H0  and  H8 = 1

    diwstop = value;
    setHSTOP(LO_BYTE(value) | 0x100);
}

void
Denise::setDIWHIGH(u16 value)
{
    trace(DIW_DEBUG, "setDIWHIGH(%x)\n", value);

    if (!isECS()) return;

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // -- -- H8 -- -- -- -- -- -- -- H8 -- -- -- -- --
    //     (stop)                  (strt)

    diwhigh = value;
    setHSTRT(LO_BYTE(diwstrt) | (GET_BIT(diwhigh,  5) ? 0x100 : 0x000));
    setHSTOP(LO_BYTE(diwstop) | (GET_BIT(diwhigh, 13) ? 0x100 : 0x000));
}

void
Denise::setHSTRT(isize val)
{
    trace(DIW_DEBUG, "setHSTRT(%lx)\n", val);

    // Record register change
    diwChanges.insert(agnus.pos.pixel(), RegChange { .reg = Reg::DIWSTRT, .value = (u16)val });
    markBorderBufferAsDirty();
}

void
Denise::setHSTOP(isize val)
{
    trace(DIW_DEBUG, "setHSTOP(%lx)\n", val);

    // Record register change
    diwChanges.insert(agnus.pos.pixel(), RegChange { .reg = Reg::DIWSTOP, .value = (u16)val });
    markBorderBufferAsDirty();
}

u16
Denise::peekJOY0DATR() const
{
    u16 result = controlPort1.joydat();
    trace(JOYREG_DEBUG, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

u16
Denise::peekJOY1DATR() const
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
    u16 result = isECS() ? 0xFFFC : 0xFFFF;
    trace(ECSREG_DEBUG, "peekDENISEID() = $%04X (%d)\n", result, result);
    return result;
}

u16
Denise::spypeekDENISEID() const
{
    return isECS() ? 0xFFFC : 0xFFFF;
}

template <Accessor s> void
Denise::pokeBPLCON0(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON0, value, Accessor::DENISE);
}

void
Denise::setBPLCON0(u16 oldValue, u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON0(%04x,%04x)\n", oldValue, newValue);

    // Record the register change
    i64 pixel = std::max(agnus.pos.pixel() - 4, (isize)0);
    conChanges.insert(pixel, RegChange { .reg = Reg::BPLCON0, .value = newValue });
    
    // Check if the HAM bit or the SHRES bit have changed
    if ((ham(oldValue) ^ ham(newValue)) || (shres(oldValue) ^ shres(newValue))) {
        pixelEngine.colChanges.insert(pixel, RegChange { .reg = Reg::BPLCON0, .value = newValue, .accessor = Accessor::DENISE } );
    }

    // Update value
    bplcon0 = newValue;

    // Determine the new bitmap resolution
    res = resolution(newValue);

    // Update border color index, because the ECSENA bit might have changed
    updateBorderColor();
    
    // Check if the BPU bits have changed
    u16 newBpuBits = (newValue >> 12) & 0b111;
    
    // Report a suspicious BPU value
    if (newBpuBits > ((res == Resolution::LORES) ? 6 : (res == Resolution::HIRES) ? 4 : 2)) {
        xfiles("BPLCON0: BPU set to irregular value %d\n", newBpuBits);
    }
}

template <Accessor s> void
Denise::pokeBPLCON1(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON1(%X)\n", value);

    // Record the register change
    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON1, value, Accessor::DENISE);
}

void
Denise::setBPLCON1(u16 oldValue, u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON1(%x,%x)\n", oldValue, newValue);

    bplcon1 = newValue & 0xFF;

    pixelOffsetOdd  = (i8)((bplcon1 & 0b00000001) << 1);
    pixelOffsetEven = (i8)((bplcon1 & 0b00010000) >> 3);
}

template <Accessor s> void
Denise::pokeBPLCON2(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON2(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON2, value);
}

void
Denise::setBPLCON2(u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON2(%X)\n", newValue);

    bplcon2 = newValue;

    if (pf1px() > 4) { xfiles("BPLCON2: PF1P = %d\n", pf1px()); }
    if (pf2px() > 4) { xfiles("BPLCON2: PF2P = %d\n", pf2px()); }
    
    // Record the register change
    i64 pixel = agnus.pos.pixel() + 4;
    conChanges.insert(pixel, RegChange { .reg = Reg::BPLCON2, .value = newValue });
}

template <Accessor s> void
Denise::pokeBPLCON3(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON3(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), Reg::BPLCON3, value);
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

u16
Denise::spypeekCLXDAT() const
{
    return clxdat | 0x8000;
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

    if constexpr (s == Accessor::AGNUS) {
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

        spriteClipBegin = std::min(spriteClipBegin, Pixel(agnus.pos.pixel() + 4));
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
    i64 pos = agnus.pos.pixel() + 6;
    constexpr auto reg = Reg(isize(Reg::SPR0POS) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

template <isize x> void
Denise::pokeSPRxCTL(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldCTL(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)

    // Record the register change
    i64 pos = agnus.pos.pixel() + 6;
    constexpr auto reg = Reg(isize(Reg::SPR0CTL) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
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
    i64 pos = agnus.pos.pixel() + 4;
    constexpr auto reg = Reg(isize(Reg::SPR0DATA) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value } );
}

template <isize x> void
Denise::pokeSPRxDATB(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%ldDATB(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;

    // Record the register change
    i64 pos = agnus.pos.pixel() + 4;
    constexpr auto reg = Reg(isize(Reg::SPR0DATB) + 4 * x);
    sprChanges[x/2].insert(pos, RegChange { .reg = reg, .value = value });
}

template <isize xx, Accessor s> void
Denise::pokeCOLORxx(u16 value)
{
    trace(COLREG_DEBUG, "pokeCOLOR%02ld(%X)\n", xx, value);

    // Record the color change
    constexpr auto reg = Reg(isize(Reg::COLOR00) + xx);
    pixelEngine.colChanges.insert(agnus.pos.pixel(), RegChange { .reg = reg, .value = value } );
}

Resolution
Denise::resolution(u16 v)
{
    if (GET_BIT(v,6) && isECS()) {
        return Resolution::SHRES;
    } else if (GET_BIT(v,15)) {
        return Resolution::HIRES;
    } else {
        return Resolution::LORES;
    }
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

template void Denise::pokeBPLCON0<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON0<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON1<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON1<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON2<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON2<Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLCON3<Accessor::CPU>(u16 value);
template void Denise::pokeBPLCON3<Accessor::AGNUS>(u16 value);

template void Denise::pokeBPLxDAT<0,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<0,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<1,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<1,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<2,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<2,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<3,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<3,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<4,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<4,Accessor::AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<5,Accessor::CPU>(u16 value);
template void Denise::pokeBPLxDAT<5,Accessor::AGNUS>(u16 value);

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

template void Denise::pokeCOLORxx<0, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<0, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<1, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<1, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<2, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<2, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<3, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<3, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<4, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<4, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<5, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<5, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<6, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<6, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<7, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<7, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<8, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<8, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<9, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<9, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<10, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<10, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<11, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<11, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<12, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<12, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<13, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<13, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<14, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<14, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<15, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<15, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<16, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<16, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<17, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<17, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<18, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<18, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<19, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<19, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<20, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<20, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<21, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<21, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<22, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<22, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<23, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<23, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<24, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<24, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<25, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<25, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<26, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<26, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<27, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<27, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<28, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<28, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<29, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<29, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<30, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<30, Accessor::AGNUS>(u16 value);
template void Denise::pokeCOLORxx<31, Accessor::CPU>(u16 value);
template void Denise::pokeCOLORxx<31, Accessor::AGNUS>(u16 value);

}
