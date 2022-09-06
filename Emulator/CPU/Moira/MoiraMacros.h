// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#ifdef _MSC_VER
#define unreachable     __assume(false)
#else
#define unreachable     __builtin_unreachable()
#endif
#define fatalError      assert(false); unreachable

#if PRECISE_TIMING == true

#define SYNC(x)         { if constexpr (C != C68020) sync(x); }
#define SYNC_68000(x)   { if constexpr (C == C68000) sync(x); }
#define SYNC_68010(x)   { if constexpr (C == C68010) sync(x); }

#define CYCLES_68000(c) { }
#define CYCLES_68010(c) { }
#define CYCLES_68020(c) { if constexpr (C == C68020) sync((c) + cp); }

#else

#define SYNC(x)         { }
#define SYNC_68000(x)   { }
#define SYNC_68010(x)   { }

#define CYCLES_68000(c) { if constexpr (C == C68000) sync(c); }
#define CYCLES_68010(c) { if constexpr (C == C68010) sync(c); }
#define CYCLES_68020(c) { if constexpr (C == C68020) sync((c) + cp); }

#endif

#define CYCLES(c) { CYCLES_68000(c) CYCLES_68010(c) CYCLES_68020(c) }

#define CYCLES_BWL_00(b,w,l) CYCLES_68000(S == Byte ? (b) : S == Word ? (w) : (l))
#define CYCLES_BWL_10(b,w,l) CYCLES_68010(S == Byte ? (b) : S == Word ? (w) : (l))
#define CYCLES_BWL_20(b,w,l) CYCLES_68020(S == Byte ? (b) : S == Word ? (w) : (l))

#define CYCLES_MBWL(m,b0,b1,b2,w0,w1,w2,l0,l1,l2) \
{ if constexpr (M == m) { CYCLES_BWL_00(b0,w0,l0) } } \
{ if constexpr (M == m) { CYCLES_BWL_10(b1,w1,l1) } } \
{ if constexpr (M == m) { CYCLES_BWL_20(b2,w2,l2) } }

#define CYCLES_DN(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_DN,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_AN(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_AN,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_AI(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_AI,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_PI(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_PI,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_PD(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_PD,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_DI(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_DI,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_IX(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_IX,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_AW(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_AW,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_AL(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_AL,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_DIPC(b0,b1,b2,w0,w1,w2,l0,l1,l2)   CYCLES_MBWL(MODE_DIPC, b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_IXPC(b0,b1,b2,w0,w1,w2,l0,l1,l2)   CYCLES_MBWL(MODE_IXPC, b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_IM(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_IM,   b0,b1,b2,w0,w1,w2,l0,l1,l2)
#define CYCLES_IP(b0,b1,b2,w0,w1,w2,l0,l1,l2)     CYCLES_MBWL(MODE_IP,   b0,b1,b2,w0,w1,w2,l0,l1,l2)

#define REVERSE_8(x) (u8)(((x) * 0x0202020202ULL & 0x010884422010ULL) % 1023)
#define REVERSE_16(x) (u16)((REVERSE_8((x) & 0xFF) << 8) | REVERSE_8(((x) >> 8) & 0xFF))
#define U32_ADD(x,y) (u32)((i64)(x) + (i64)(y))
#define U32_SUB(x,y) (u32)((i64)(x) - (i64)(y))
#define U32_INC(x,y) x = U32_ADD(x,y)
#define U32_DEC(x,y) x = U32_SUB(x,y)

#define ______________xx(opcode) (u8)((opcode >> 0)  & 0b11)
#define _____________xxx(opcode) (u8)((opcode >> 0)  & 0b111)
#define ____________xxxx(opcode) (u8)((opcode >> 0)  & 0b1111)
#define ___________xxxxx(opcode) (u8)((opcode >> 0)  & 0b11111)
#define __________xxxxxx(opcode) (u8)((opcode >> 0)  & 0b111111)
#define _________xxxxxxx(opcode) (u8)((opcode >> 0)  & 0b1111111)
#define ________xxxxxxxx(opcode) (u8)((opcode >> 0)  & 0b11111111)
#define ____xxxxxxxxxxxx(opcode) (u16)((opcode >> 0) & 0b111111111111)
#define ___________xxx__(opcode) (u8)((opcode >> 2)  & 0b111)
#define ___________xx___(opcode) (u8)((opcode >> 3)  & 0b11)
#define __________xxx___(opcode) (u8)((opcode >> 3)  & 0b111)
#define __________xx____(opcode) (u8)((opcode >> 4)  & 0b11)
#define _________xxx____(opcode) (u8)((opcode >> 4)  & 0b111)
#define __________x_____(opcode) (u8)((opcode >> 5)  & 0b1)
#define ________xxx_____(opcode) (u8)((opcode >> 5)  & 0b111)
#define _______xxxx_____(opcode) (u8)((opcode >> 5)  & 0b1111)
#define _________x______(opcode) (u8)((opcode >> 6)  & 0b1)
#define ________xx______(opcode) (u8)((opcode >> 6)  & 0b11)
#define _______xxx______(opcode) (u8)((opcode >> 6)  & 0b111)
#define ______xxxx______(opcode) (u8)((opcode >> 6)  & 0b1111)
#define _____xxxxx______(opcode) (u8)((opcode >> 6)  & 0b11111)
#define ________x_______(opcode) (u8)((opcode >> 7)  & 0b1)
#define ______xxx_______(opcode) (u8)((opcode >> 7)  & 0b111)
#define _______x________(opcode) (u8)((opcode >> 8)  & 0b1)
#define ______x_________(opcode) (u8)((opcode >> 9)  & 0b1)
#define _____xx_________(opcode) (u8)((opcode >> 9)  & 0b11)
#define ____xxx_________(opcode) (u8)((opcode >> 9)  & 0b111)
#define ____xx__________(opcode) (u8)((opcode >> 10) & 0b11)
#define ___xxx__________(opcode) (u8)((opcode >> 10) & 0b111)
#define ____x___________(opcode) (u8)((opcode >> 11) & 0b1)
#define ___xx___________(opcode) (u8)((opcode >> 11) & 0b11)
#define _xxx____________(opcode) (u8)((opcode >> 12) & 0b111)
#define xxxx____________(opcode) (u8)((opcode >> 12) & 0b1111)
#define xxx_____________(opcode) (u8)((opcode >> 13) & 0b111)
