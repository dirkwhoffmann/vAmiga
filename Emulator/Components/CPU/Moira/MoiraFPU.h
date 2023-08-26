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
    FpuExtended val;


    //
    // Constructing
    //

    FPUReg(FPU& fpu) : fpu(fpu) { }
    void reset() { val = FpuExtended::NaN(); }


    //
    // Getting and setting
    //

    FpuExtended get();
    void set(const FpuExtended other);
    void move(FPUReg &dest);


    //
    // Classifying
    //

    bool isNegative() const { return val.isNegative(); }
    bool isZero() const  { return val.isZero(); }
    bool isInfinity() const { return val.isInfinity(); }
    bool isNaN() const { return val.isNaN(); }
    bool isSignalingNaN() const { return val.isSignalingNaN(); }
    bool isNonsignalingNaN() const { return val.isNonsignalingNaN(); }
    bool isNormalized() const { return val.isNormalized(); }


    //
    // Converting
    //

    u8 asByte();
    u16 asWord();
    u32 asLong();
    u32 asSingle();
    u64 asDouble();
    FpuExtended asExtended();
    FpuPacked asPacked(int k = 0);
};

class FPU {
    
    // Reference to the CPU
    class Moira &moira;

    // Emulated FPU model
    FPUModel model = FPU_NONE;

public:

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

    // DEPRECATED
    void pushRoundingMode(int mode);
    void pushRoundingMode() { pushRoundingMode(getRoundingMode()); }
    void popRoundingMode();

    static int setRoundingMode(int mode);


    //
    // Accessing registers
    //

public:

    u32 getFPCR() const { return fpcr & 0x0000FFF0; }
    void setFPCR(u32 value);

    void clearFPSR() { fpsr &= 0xFFFF00F8; }
    u32 getFPSR() const { return fpsr & 0x0FFFFFF8; }
    void setFPSR(u32 value);

    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);

    void setExcStatusBit(u32 mask);
    void clearExcStatusBit(u32 mask);

    void setConditionCodes(int reg);
    void setConditionCodes(const FpuExtended &value);


    //
    // Accessing the constant Rom
    //

    // Reads a value from the constant Rom
    FpuExtended readCR(unsigned nr);

    
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
