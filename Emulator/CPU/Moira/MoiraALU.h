// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* The following functions emulate the arithmetic logical unit of the CPU:
 *
 *       shift : ASx, LSx, ROx, ROXx
 *      addsub : ADDx, SUBx
 *         bcd : ABCD, NBCD, SBCD
 *         cmp : CMPx
 *       logic : NOT, NEG, NEGX, ANDx, ORx, EORx
 *         bit : BCHG, BSET, BCLR, BTST
 *        cond : Bxx, DBxx, Sx
 */

bool cond(Instr I);
bool cond(Cond C);

template <Core C, Instr I, Size S> u32    shift(int cnt, u64 data);
template <Core C, Instr I, Size S> u32   addsub(u32 op1, u32 op2);
template <Core C>                  u32     muls(u32 op1, u32 op2);
template <Core C>                  u32     mulu(u32 op1, u32 op2);
template <Core C,          Size S> u64    mulls(u32 op1, u32 op2);
template <Core C,          Size S> u64    mullu(u32 op1, u32 op2);
template <Core C, Instr I>         u32      div(u32 op1, u32 op2);
template <Core C, Instr I, Size S> u32      bcd(u32 op1, u32 op2);
template <Core C,          Size S> void     cmp(u32 op1, u32 op2);
template <Core C, Instr I, Size S> u32    logic(u32 op1);
template <Core C, Instr I, Size S> u32    logic(u32 op1, u32 op2);
template <Core C, Instr I>         u32      bit(u32 op,  u8 nr);

template <Core C, Instr I>         int  cyclesBit(u8 nr) const;
template <Core C, Instr I>         int  cyclesMul(u16 data) const;
template <Core C, Instr I>         int  cyclesDiv(u32 dividend, u16 divisor) const;

// Bitfield instructions (68020)
template <Instr I> u32 bitfield(u32 data, u32 offset, u32 width, u32 mask);

// Division
template <Core C, Instr I> u32 divMusashi(u32 op1, u32 op2);
template <Size S> std::pair<u32,u32> divlsMusashi(u64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divluMusashi(u64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divlsMoira(i64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divluMoira(u64 op1, u32 op2);

// Computes the values of undefined flags for certain instructions
template <Core C, Instr I, Size S> void setUndefinedFlags(i32 arg1, i32 arg2, i32 arg3 = 0);

template <Core C, Size S> void setUndefinedDIVU(u32 dividend, u16 divisor);
template <Core C, Size S> void setUndefinedDIVS(i32 dividend, i16 divisor);
template <Core C, Size S> void setUndefinedDIVUL(i64 dividend, i32 divisor);
template <Core C, Size S> void setUndefinedDIVSL(i64 dividend, i32 divisor);
template <Core C, Size S> void setDivZeroDIVU(u32 dividend);
template <Core C, Size S> void setDivZeroDIVS(u32 dividend);
template <Core C, Size S> void setDivZeroDIVUL(i64 dividend);
template <Core C, Size S> void setDivZeroDIVSL(i64 dividend);
