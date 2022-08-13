// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
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
template <Core C, Instr I>         u32      mul(u32 op1, u32 op2);
template <Core C, Instr I>         u32      div(u32 op1, u32 op2);
template <Core C, Instr I, Size S> u32      bcd(u32 op1, u32 op2);
template <Core C,          Size S> void     cmp(u32 op1, u32 op2);
template <Core C, Instr I, Size S> u32    logic(u32 op1);
template <Core C, Instr I, Size S> u32    logic(u32 op1, u32 op2);
template <Core C, Instr I>         u32      bit(u32 op,  u8 nr);

template <Core C, Instr I>         int  cyclesBit(u8 nr);
template <Core C, Instr I>         int  cyclesMul(u16 data);
template <Core C, Instr I>         int  cyclesDiv(u32 dividend, u16 divisor);

// Bitfield instructions (68020)
template <Instr I> u32 bitfield(u32 data, u32 offset, u32 width, u32 mask);

// Musashi compatibility mode
template <Core C, Instr I> u32 mulMusashi(u32 op1, u32 op2);
template <Size S> u64 mullsMusashi(u32 op1, u32 op2);
template <Size S> u64 mulluMusashi(u32 op1, u32 op2);

template <Core C, Instr I> u32 divMusashi(u32 op1, u32 op2);
template <Size S> std::pair<u32,u32> divlsMusashi(u64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divluMusashi(u64 op1, u32 op2);

