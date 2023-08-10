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


    //
    // Constructors
    //

    Float80() { };
    Float80(u32 value);
    Float80(double value);
    Float80(long double value);
    Float80(u16 high, u64 low);
    Float80(bool mSign, i16 e, u64 m);
    Float80(const struct FPUReg &reg);


    //
    // Constants
    //

    static Float80 NaN() { return Float80(0x7FFF, 0xFFFFFFFFFFFFFFFF); }
    static constexpr i64 bias = 0x3FFF;
    

    //
    // Analyzers
    //

    i64 sgn() const { return (raw.high & 0x8000) ? -1 : 1; }
    i64 exp() const { return i64(raw.high & 0x7FFF) - bias; }
    u64 man() const { return raw.low; }

    std::pair<int, long double> frexp10() const;

    bool isNegative() const;
    bool isZero() const;
    bool isInfinity() const;
    bool isNaN() const;
    bool isSignalingNaN() const;
    bool isNonsignalingNaN() const;
    bool isNormalized() const;


    //
    // Converters
    //

    double asDouble() const;
    long double asLongDouble() const;
    long asLong() const;


    //
    // Arithmetic
    //

    void normalize();


    //
    // Operators
    //

    inline bool operator==(const Float80 &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }
};

}
