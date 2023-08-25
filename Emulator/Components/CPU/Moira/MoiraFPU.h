// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "FpuFormats.h"

namespace vamiga::moira {

struct FPUReg {

    // Reference to the FPU
    class FPU &fpu;

    // Register value
    Float80 val;

    FPUReg(FPU& fpu) : fpu(fpu) { }

    void reset() { val = Float80::NaN(); }
    
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

    u32 fpiar;
    u32 fpsr;
    u32 fpcr;

    // Experimental
    int oldRoundingMode;


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

    FpuPrecision getPrecision() const;
    FpuRoundingMode getRoundingMode() const;

    void pushRoundingMode(int mode);
    void pushRoundingMode() { pushRoundingMode(getRoundingMode()); }
    void popRoundingMode();


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

    // Rounds a long double (experimental)
    long roundmantissa(long double value, int digits);

    // Converts the extended format into the packed decimal real format
    void pack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3);
    Packed pack(const Float80 &value, int k);

    // Converts the extended format into the packed decimal real format
    void unpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result);
    Float80 unpack(const Packed &packed);

    
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
