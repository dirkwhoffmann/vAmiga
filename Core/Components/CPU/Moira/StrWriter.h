// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

namespace vamiga::moira {

//
// Wrapper structures controlling the output format
//

// Numerical values
struct Int { i32 raw; Int(i32 v) : raw(v) { } };
struct UInt { u32 raw; UInt(u32 v) : raw(v) { } };
struct UInt8 { u8 raw; UInt8(u8 v) : raw(v) { } };
struct UInt16 { u16 raw; UInt16(u16 v) : raw(v) { } };
struct UInt24 { u32 raw; UInt24(u32 v) : raw(v) { } };
struct UInt32 { u32 raw; UInt32(u32 v) : raw(v) { } };

// Immediate operands
struct Imu { u32 raw; Size sz; Imu(u32 v, Size s = Long) : raw(v), sz(s) { } };
struct Ims { i32 raw; Size sz; Ims(i32 v, Size s = Long) : raw(v), sz(s) { } };
struct Imd { u32 raw; Imd(u32 v) : raw(v) { } };

// Mnemonics
struct Ins { Instr raw; Ins(Instr v) : raw(v) { } };
struct Sz { Size raw; Sz(Size v) : raw(v) { } };
struct Szb { Size raw; Szb(Size v) : raw(v) { } };
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
struct Ea {
    Mode m; Size sz;
    u32 pc; u16 reg; u8 dw; u8 ow; u32 ext1; i32 ext2; i32 ext3;
};
struct Ai { const Ea &ea; };
struct Pi { const Ea &ea; };
struct Pd { const Ea &ea; };
struct Di { const Ea &ea; };
struct Ix { const Ea &ea; };
struct IxMot { const Ea &ea; };
struct IxMit { const Ea &ea; };
struct IxMus { const Ea &ea; };
struct Aw { const Ea &ea; };
struct Al { const Ea &ea; };
struct DiPc { const Ea &ea; };
struct Im { const Ea &ea; };
struct Ip { const Ea &ea; };

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
struct Av { Instr I; Mode M; Size S; u32 ext1 = 0; };
struct Finish { };

class StrWriter
{

public:

    char comment[32];       // Appended to the end of the disassembled string
    char *base;             // Start address of the destination string
    char *ptr;              // Write pointer

    const DasmStyle &style;
    Tab tab {8};

public:

    StrWriter(char *p, const DasmStyle &style) : style(style) {

        comment[0] = 0;
        base = ptr = p;
    };

    StrWriter& operator<<(char);
    StrWriter& operator<<(const char *);
    StrWriter& operator<<(int);
    StrWriter& operator<<(unsigned int);

    StrWriter& operator<<(Int);
    StrWriter& operator<<(UInt);
    StrWriter& operator<<(UInt8);
    StrWriter& operator<<(UInt16);
    StrWriter& operator<<(UInt24);
    StrWriter& operator<<(UInt32);

    StrWriter& operator<<(Imu);
    StrWriter& operator<<(Ims);
    StrWriter& operator<<(Imd);

    StrWriter& operator<<(Ins);
    StrWriter& operator<<(Sz);
    StrWriter& operator<<(Szb);
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

    StrWriter& operator<<(const Ea &);

    StrWriter& operator<<(Ai);
    StrWriter& operator<<(Pi);
    StrWriter& operator<<(Pd);
    StrWriter& operator<<(Di);
    StrWriter& operator<<(Ix);
    StrWriter& operator<<(IxMot);
    StrWriter& operator<<(IxMit);
    StrWriter& operator<<(IxMus);
    StrWriter& operator<<(Aw);
    StrWriter& operator<<(Al);
    StrWriter& operator<<(DiPc);
    StrWriter& operator<<(Im);
    StrWriter& operator<<(Ip);

    StrWriter& operator<<(Scale);
    StrWriter& operator<<(Fc);
    StrWriter& operator<<(Sfc);
    StrWriter& operator<<(Dfc);
    StrWriter& operator<<(Fp);
    StrWriter& operator<<(Ffmt);
    StrWriter& operator<<(Fctrl);
    StrWriter& operator<<(Tab);
    StrWriter& operator<<(Sep);
    StrWriter& operator<<(const Av &);
    StrWriter& operator<<(Finish);
};

}
