// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "softfloat.h"

namespace vamiga::moira {

class Float80 {

public:

    // TODO: Make this private. Afterwards, remove softfloat lib
    softfloat::floatx80 raw = { };

    Float80() { };
    Float80(u32 value);
    Float80(double value);
    Float80(long double value);
    Float80(u16 high, u64 low);
    Float80(bool mSign, i16 e, u64 m);
    Float80(const struct FPUReg &reg);

    static Float80 NaN() { return Float80(0x7FFF, 0xFFFFFFFFFFFFFFFF); }

    double asDouble();
    long asLong();

    bool isSignalingNaN();
    bool isNonsignalingNaN();
    bool isNaN();
    bool isNormalized();
    void normalize();

    inline bool operator==(const Float80 &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }
};

}
