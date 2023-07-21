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
    // Analyzing instructions
    //

public:

    // Checks the validity of the extension words
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;

};

}
