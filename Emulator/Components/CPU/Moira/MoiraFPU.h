// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"

namespace vamiga::moira {

struct Float80 {

    softfloat::floatx80 raw = { };

    Float80() { };
    Float80(u32 value);
    Float80(double value);
    Float80(long double value);
    Float80(class ExtendedDouble value);
    Float80(u16 high, u64 low);
    Float80(bool mSign, i16 e, u64 m);
    Float80(const struct FPUReg &reg);

    double asDouble();
    long asLong();

    bool isNormalized();
    void normalize();

    inline bool operator==(const Float80 &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }
};

struct Packed {

    u32 data[3];
};

struct FPUReg {

    // Reference to the FPU
    class FPU &fpu;

    // Register value
    Float80 val;

    FPUReg(FPU& fpu) : fpu(fpu) { }

    void clear() { val.raw = { }; }
    
    // Getter
    Float80 get();
    u8 asByte();
    u16 asWord();
    u32 asLong();
    u32 asSingle();
    u64 asDouble();
    Float80 asExtended();
    Packed asPacked(int k = 0);

    // Setter
    void set(const Float80 other);
    void move(FPUReg &dest);
};

/*
struct FPUBitRep {

    union {
        u8 b;
        u16 w;
        u32 l;
        u32 s;
        u64 d;
        Float80 e;
        u32 p[3];
    };
};
*/

class FPU {

public: // REMOVE ASAP
    
    // Reference to the CPU
    class Moira &moira;

    // Emulated FPU model
    FPUModel model = FPU_NONE;

    // Registers
    FPUReg fpr[8] = {
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this),
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this)
    };
    FPUReg tmp = FPUReg(*this);

    u32 fpiar;
    u32 fpsr;
    u32 fpcr;


    //
    // Constructing
    //

public:

    FPU(Moira& ref);
    void reset();


    //
    // Configuring
    //

public:

    // Selects the emulated CPU model
    void setModel(FPUModel model);
    FPUModel getModel() const { return model; }


    //
    // Accessing registers
    //

public:

    u32 getFPCR() const { return fpcr & 0x0000FFF0; }
    void setFPCR(u32 value);

    void clearFPSR() { fpsr &= 0x00FF00F8; }
    u32 getFPSR() const { return fpsr & 0x0FFFFFF8; }
    void setFPSR(u32 value);

    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);

    void setExcStatusBit(u32 mask);
    void clearExcStatusBit(u32 mask);
    // void clearExcStatusBits();

    void setFlags(int reg);
    void setFlags(const Float80 &value);


    //
    // Accessing the constant Rom
    //

    // Reads a value from the constant Rom
    Float80 readCR(unsigned nr);


    //
    // Converting
    //

public:

    // Converts the extended format into the packed decimal real format
    void musashiPack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3); // DEPRECATED
    void pack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3);

    // Converts the extended format into the packed decimal real format
    void musashiUnpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result); // DEPRECATED
    void unpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result);

    // Experimental
    Float80 round(const Float80 value);
    u8 roundB(const Float80 value);
    u16 roundW(const Float80 value);
    u32 roundL(const Float80 value);
    u32 roundS(const Float80 value);
    u64 roundD(const Float80 value);


    //
    // Analyzing instructions
    //

public:

    // Checks the validity of the extension words
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;


    //
    // Executing instructions
    //

    // void execFMovecr(u16 op, u16 ext);

};

}
