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

    double asDouble();
    long asLong();

    bool isNormalized();
    void normalize();

    inline bool operator==(const Float80 &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }


};

class FPU {

public: // REMOVE ASAP
    
    // Reference to the CPU
    class Moira &moira;

    // Emulated FPU model
    FPUModel model = FPU_NONE;

    // Registers
    Float80 fpr[8];
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

    Float80 getFPR(int n) const;
    void setFPR(int n, Float80 value);
    void setFPR(int n, u16 high, u64 low);

    u32 getFPCR() const { return fpcr & 0x0000FFF0; }
    void setFPCR(u32 value);

    u32 getFPSR() const { return fpsr & 0x0FFFFFF8; }
    void setFPSR(u32 value);

    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);

    void setExcStatusBit(u32 mask);
    void clearExcStatusBit(u32 mask);
    void clearExcStatusBits();

    void setFlags(int reg);
    void setFlags(const Float80 &value);


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
