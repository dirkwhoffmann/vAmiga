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

    softfloat::floatx80 raw;
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

    FPU(Moira& ref) : moira(ref) { }

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

    Float80 getFPR(int n) const;
    void setFPR(int n, u16 high, u64 low);

    u32 getFPCR() const { return fpcr; }
    void setFPCR(u32 value);

    u32 getFPSR() const { return fpsr; }
    void setFPSR(u32 value);

    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);


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
