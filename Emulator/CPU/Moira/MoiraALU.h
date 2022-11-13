// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

//
// Logic functions
//

// Branch conditions (Bcc, DBcc, Scc, TRAPcc)
template <Cond C> bool evalCond();
template <Instr I> bool cond();

// Shift instructions (ASx, LSx, ROx, ROXx)
template <Core C, Instr I, Size S> u32 shift(int cnt, u64 data);

// BCD arithmetic (ABCD, NBCD, SBCD)
template <Core C, Instr I, Size S> u32 bcd(u32 op1, u32 op2);

// Compare instructions (CMPx)
template <Core C, Size S> void cmp(u32 op1, u32 op2);

// Logic instructions (NOT, NEG, NEGX, ANDx, ORx, EORx)
template <Core C, Instr I, Size S> u32 logic(u32 op1);
template <Core C, Instr I, Size S> u32 logic(u32 op1, u32 op2);

// Bit instructions (BCHG, BSET, BCLR, BTST)
template <Core C, Instr I> u32 bit(u32 op,  u8 nr);

// Bitfield instructions (68020+)
template <Instr I> u32 bitfield(u32 data, u32 offset, u32 width, u32 mask);

// Addition and subtraction (MULS, MULU, MULLS, MULLU)
template <Core C, Instr I, Size S> u32 addsub(u32 op1, u32 op2);

// Multiplication (MULS, MULU, MULLS, MULLU)
template <Core C> u32 muls(u32 op1, u32 op2);
template <Core C> u32 mulu(u32 op1, u32 op2);
template <Core C, Size S> u64 mulls(u32 op1, u32 op2);
template <Core C, Size S> u64 mullu(u32 op1, u32 op2);

// Division (DIVS, DIVU, DIVSL, DIVUL)
template <Core C> u32 divsMoira(u32 op1, u32 op2);
template <Core C> u32 divuMoira(u32 op1, u32 op2);
template <Core C> u32 divsMusashi(u32 op1, u32 op2);
template <Core C> u32 divuMusashi(u32 op1, u32 op2);
template <Size S> std::pair<u32,u32> divlsMoira(i64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divluMoira(u64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divlsMusashi(u64 op1, u32 op2);
template <Size S> std::pair<u32,u32> divluMusashi(u64 op1, u32 op2);


//
// Cycle counts
//

template <Core C, Instr I> int cyclesBit(u8 nr) const;
template <Core C, Instr I> int cyclesMul(u16 data) const;
template <Core C, Instr I> int cyclesDiv(u32 dividend, u16 divisor) const;


//
// Undefined flags
//

template <Core C, Size S> void setUndefinedCHK(i32 arg1, i32 arg2);
template <Core C, Size S> void setUndefinedCHK2(i32 lower, i32 upper, i32 value);
template <Core C, Size S> void setUndefinedDIVU(u32 dividend, u16 divisor);
template <Core C, Size S> void setUndefinedDIVS(i32 dividend, i16 divisor);
template <Core C, Size S> void setUndefinedDIVUL(i64 dividend, i32 divisor);
template <Core C, Size S> void setUndefinedDIVSL(i64 dividend, i32 divisor);
template <Core C, Size S> void setDivZeroDIVU(u32 dividend);
template <Core C, Size S> void setDivZeroDIVS(u32 dividend);
template <Core C, Size S> void setDivZeroDIVUL(i64 dividend);
template <Core C, Size S> void setDivZeroDIVSL(i64 dividend);
