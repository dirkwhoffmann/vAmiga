// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "softfloat.h"

namespace xdb {
template <class T> class XDouble;
}
namespace vamiga::moira {

class Float32;
class Float64;
class Float80;
class Packed;

typedef std::function<void(u32)> ExceptionHandler;

struct Float32 {

    u32 raw = 0;

public:

    Float32(const class Float80 &value, ExceptionHandler handler);
};

struct Float64 {

    u64 raw = 0;

public:

    Float64(const class Float80 &value, ExceptionHandler handler);
};

class Float80 {

public: // REMOVE ASAP

    // TODO: Make this private. Afterwards, remove softfloat lib
    softfloat::floatx80 raw = { };


    //
    // Constructors
    //

public:
    
    Float80() { };
    Float80(u32 value); // DEPRECATED
    Float80(const Float32 &value);
    Float80(const Float64 &value);
    Float80(double value);
    Float80(long double value, FpuRoundingMode mode);
    Float80(u16 high, u64 low);
    Float80(bool mSign, i16 e, u64 m);
    Float80(const struct Packed &packed, FpuRoundingMode mode);
    Float80(const std::string &s, FpuRoundingMode mode);
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

    u8 asByte(ExceptionHandler handler) const;
    u16 asWord(ExceptionHandler handler) const;

    double asDouble() const;
    long double asLongDouble() const;
    long asLong() const;
    struct Packed asPacked(int kfactor, FpuRoundingMode mode, u32 *statusbits) const;


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

struct Packed {

    u32 data[3] = { };
};

}
