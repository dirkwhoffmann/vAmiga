// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

namespace moira {

//
// Wrapper structures controlling the output format
//

// Numerical values
struct Int { i32 raw; Int(i32 v) : raw(v) { } };
struct UInt { u32 raw; UInt(u32 v) : raw(v) { } };
struct UInt8 { u8 raw; UInt8(u8 v) : raw(v) { } };
struct UInt16 { u16 raw; UInt16(u16 v) : raw(v) { } };
struct UInt32 { u32 raw; UInt32(u32 v) : raw(v) { } };

// Immediate operands
struct Imu { u32 raw; Imu(u32 v) : raw(v) { } };
template <Size S> struct Ims { i32 raw; Ims(i32 v) : raw(v) { } };
struct Imd { u32 raw; Imd(u32 v) : raw(v) { } };

// Mnemonics
template <Instr I> struct Ins { };
template <Size S> struct Sz { };
template <Size S> struct Szb { };
struct Cc { int raw; Cc(int v) : raw(v) { } };
struct Cpcc { int raw; Cpcc(int v) : raw(v) { } };
struct Fcc { int raw; Fcc(int v) : raw(v) { } };
struct Pcc { int raw; Pcc(int v) : raw(v) { } };

// Registers
struct Dn { int raw; Dn(int v) : raw(v) { } };
struct An { int raw; An(int v) : raw(v) { } };
struct Rn { int raw;  Rn(int v) : raw(v) { } };
struct Cn { u16 raw; Cn(u16 v) : raw(v) { } };
struct Ccr { };
struct Pc { };
struct Zpc { };
struct Sr { };
struct Usp { };

// Register lists
struct RegList { u16 raw; RegList(u16 v) : raw(v) { } };
struct RegRegList { u16 raw; RegRegList(u16 v) : raw(v) { } };
struct FRegList { u16 raw; FRegList(u16 v) : raw(v) { } };

// Addressing modes
template <Mode M, Size S = Word> struct Ea {
    u32 pc; u16 reg; u8 dw; u8 ow; u32 ext1; i32 ext2; i32 ext3;
};
template <Mode M, Size S> struct Ai { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Pi { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Pd { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Di { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Ix { const Ea<M,S> &ea; };
template <Mode M, Size S> struct IxMot { const Ea<M,S> &ea; };
template <Mode M, Size S> struct IxMit { const Ea<M,S> &ea; };
template <Mode M, Size S> struct IxMus { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Aw { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Al { const Ea<M,S> &ea; };
template <Mode M, Size S> struct DiPc { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Im { const Ea<M,S> &ea; };
template <Mode M, Size S> struct Ip { const Ea<M,S> &ea; };

struct Scale { int raw; Scale(int v) : raw(v) { } };

// MMU
struct Fc { int raw; Fc(int v) : raw(v) { } };
struct Sfc { };
struct Dfc { };

// FPU
struct Fp { int raw; Fp(int v) : raw(v) { } };
struct Ffmt { int raw; Ffmt(int v) : raw(v) { } };
struct Fctrl { int raw; Fctrl(int v) : raw(v) { } };

// Indentation
struct Tab { int raw;  Tab(int v) : raw(v) { } };
struct Sep { };

// Misc
template <Instr I, Mode M, Size S> struct Av { u32 ext1 = 0; };
struct Finish { };

class StrWriter
{
    
public:
    
    char comment[32];       // Appended to the end of the disassembled string
    char *base;             // Start address of the destination string
    char *ptr;              // Write pointer

    DasmStyle style;
    DasmNumberFormat nf;
    
public:
    
    StrWriter(char *p, DasmStyle s, DasmNumberFormat n) {
        
        comment[0] = 0;
        base = ptr = p;
        style = s;
        nf = n;
    };

    StrWriter& operator<<(char);
    StrWriter& operator<<(const char *);
    StrWriter& operator<<(int);
    StrWriter& operator<<(unsigned int);

    StrWriter& operator<<(Int);
    StrWriter& operator<<(UInt);
    StrWriter& operator<<(UInt8);
    StrWriter& operator<<(UInt16);
    StrWriter& operator<<(UInt32);

    StrWriter& operator<<(Imu);
    template <Size S> StrWriter& operator<<(Ims<S>);
    StrWriter& operator<<(Imd);

    template <Instr I> StrWriter& operator<<(Ins<I>);
    template <Size S> StrWriter& operator<<(Sz<S>);
    template <Size S> StrWriter& operator<<(Szb<S>);
    StrWriter& operator<<(Cc);
    StrWriter& operator<<(Cpcc);
    StrWriter& operator<<(Fcc);
    StrWriter& operator<<(Pcc);

    StrWriter& operator<<(Dn);
    StrWriter& operator<<(An);
    StrWriter& operator<<(Rn);
    StrWriter& operator<<(Cn);
    StrWriter& operator<<(Ccr);
    StrWriter& operator<<(Pc);
    StrWriter& operator<<(Zpc);
    StrWriter& operator<<(Sr);
    StrWriter& operator<<(Usp);

    StrWriter& operator<<(RegList);
    StrWriter& operator<<(RegRegList);
    StrWriter& operator<<(FRegList);

    template <Mode M, Size S> StrWriter& operator<<(const Ea<M,S> &);

    template <Mode M, Size S> StrWriter& operator<<(Ai<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Pi<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Pd<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Di<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Ix<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(IxMot<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(IxMit<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(IxMus<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Aw<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Al<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(DiPc<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Im<M,S>);
    template <Mode M, Size S> StrWriter& operator<<(Ip<M,S>);

    StrWriter& operator<<(Scale);
    StrWriter& operator<<(Fc);
    StrWriter& operator<<(Sfc);
    StrWriter& operator<<(Dfc);
    StrWriter& operator<<(Fp);
    StrWriter& operator<<(Ffmt);
    StrWriter& operator<<(Fctrl);
    StrWriter& operator<<(Tab);
    StrWriter& operator<<(Sep);
    template <Instr I, Mode M, Size S> StrWriter& operator<<(const Av<I,M,S> &);
    StrWriter& operator<<(Finish);
};

}
