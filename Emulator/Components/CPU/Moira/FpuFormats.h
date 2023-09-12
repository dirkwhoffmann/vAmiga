// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "softfloat.h"
#include <functional>

namespace vamiga::moira {

/* The Motorola floating-point unit supports seven data formats:
 *
 *    Byte Integer (B)
 *    Word Integer (W)
 *    Long Word Integer (L)
 *    Single Precision Real (S)
 *    Double Precision Real (D)
 *    Extended Precision Real (X)
 *    Packed Decimal String Real (P)
 *
 * Each of these formats is managed by a seperate struct.
 */

struct FpuByte;
struct FpuWord;
struct FpuLong;
struct FpuSingle;
struct FpuDouble;
struct FpuExtended;
struct FpuPacked;

class FPUReg;

typedef std::function<void(u32)> ExcHandler;


//
// FpuByte
//

struct FpuByte {

    i8 raw;

    FpuByte() : raw(0) { };
    FpuByte(i8 value) : raw(value) { };
    FpuByte(u32 value) : raw((i8)value) { };
    FpuByte(const FpuExtended &value, ExcHandler handler = [](auto&&...) {});
    FpuByte(const FPUReg &value, ExcHandler handler = [](auto&&...) {});
};


//
// FpuWord
//

struct FpuWord {

    i16 raw;

    FpuWord() : raw(0) { };
    FpuWord(i16 value) : raw(value) { };
    FpuWord(u32 value) : raw((i16)value) { };
    FpuWord(const FpuExtended &value, ExcHandler handler = [](auto&&...) {});
    FpuWord(const FPUReg &value, ExcHandler handler = [](auto&&...) {});
};


//
// FpuLong
//

struct FpuLong {

    i32 raw;

    FpuLong() : raw(0) { };
    FpuLong(i32 value) : raw(value) { };
    FpuLong(u32 value) : raw((i32)value) { };
    FpuLong(const FpuExtended &value, ExcHandler handler = [](auto&&...) {});
    FpuLong(const FPUReg &value, ExcHandler handler = [](auto&&...) {});
};


//
// FpuSingle
//

struct FpuSingle {

    u32 raw;

    FpuSingle() : raw(0) { };
    FpuSingle(u32 value) : raw(value) { };
    FpuSingle(float value);
    FpuSingle(const FpuExtended &value, ExcHandler handler = [](auto&&...) {});
    FpuSingle(const FPUReg &value, ExcHandler handler = [](auto&&...) {});

    bool signbit() const;
    bool isinf() const;
    bool isposinf() const;
    bool isneginf() const;
};


//
// FpuDouble
//

struct FpuDouble {

    u64 raw;

    FpuDouble() : raw(0) { };
    FpuDouble(u64 value) : raw(value) { };
    FpuDouble(u32 hi, u32 lo) : raw(u64(hi) << 32 | lo) { };
    FpuDouble(double value);
    FpuDouble(const FpuExtended &value, ExcHandler handler = [](auto&&...) {});
    FpuDouble(const class FPUReg &value, ExcHandler handler = [](auto&&...) {});

    bool signbit() const;
    bool isinf() const;
    bool isposinf() const;
    bool isneginf() const;
};


//
// FpuExtended
//

struct FpuExtended {

    softfloat::floatx80 raw;

    // Constants
    static constexpr i64 bias = 0x3FFF;

    static FpuExtended nan;
    static FpuExtended zero;
    static FpuExtended posZero;
    static FpuExtended negZero;
    static FpuExtended inf;
    static FpuExtended posInf;
    static FpuExtended negInf;

    // Constructors
    FpuExtended() { raw = { }; }
    FpuExtended(u32 hi, u64 lo) { raw.high = u16(hi); raw.low = lo; }
    FpuExtended(u32 hi, u32 lo1, u32 lo2) { raw.high = u16(hi); raw.low = u64(lo1) << 32 | lo2; }
    FpuExtended(const FpuByte &value, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const FpuWord &value, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const FpuLong &value, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const FpuSingle &value, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const FpuDouble &value, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const FpuPacked &value, FpuRoundingMode mode, ExcHandler handler = [](auto&&...) {});
    FpuExtended(const std::string &s, FpuRoundingMode mode, ExcHandler handler = [](auto&&...) {});
    FpuExtended(long double value, FpuRoundingMode mode = FPU_RND_NEAREST, ExcHandler handler = [](auto&&...) {});
    FpuExtended(bool mSign, i64 e, u64 m, ExcHandler handler = [](auto&&...) {});

    // Converters
    long double asLongDouble() const;

    // Operators
    FpuExtended operator-() const;
    bool operator==(const FpuExtended& rhs) const;
    bool operator!=(const FpuExtended& rhs) const;
    bool operator<=(const FpuExtended& rhs) const;
    bool operator>=(const FpuExtended& rhs) const;
    bool operator<(const FpuExtended& rhs) const;
    bool operator>(const FpuExtended& rhs) const;

    // Analyzers
    bool signbit() const { return raw.high & 0x8000; }
    i64 exp() const { return i64(raw.high & 0x7FFF) - bias; }
    u64 man() const { return raw.low; }
    bool minexp() const { return (raw.high & 0x7FFF) == 0; }
    bool maxexp() const { return (raw.high & 0x7FFF) == 0x7FFF; }
    bool m62() const { return raw.low & (1L << 62); }
    bool m63() const { return raw.low & (1L << 63); }

    int fpclassify() const;
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isinf() const { return (raw.high & 0x7FFF) == 0x7FFF && raw.low == 0; }
    bool isnan() const { return (raw.high & 0x7FFF) == 0x7FFF && raw.low != 0; }
    bool isSignalingNaN() const { return isnan() && !m62(); }
    bool isNonsignalingNaN() const { return isnan() && m62(); }
    bool isnormal() const { return !minexp() && !maxexp() && m63(); }
    bool issubnormal() const { return (raw.high & 0x7FFF) == 0 && raw.low && !m63(); }
    bool isnegative() const { return signbit(); }
    bool ispositive() const { return !signbit(); }
    bool iszero() const { return (raw.high & 0x7FFF) == 0 && raw.low == 0; }

    // Misc
    void normalize();
    std::pair<int, long double> frexp10() const;
    FpuExtended copysign(bool sign);
    FpuExtended copysign(const FpuExtended &other);
};


//
// FpuPacked
//

struct FpuPacked {

    u32 data[3] = { };

    FpuPacked() { }
    FpuPacked(u32 dw1, u32 dw2, u32 dw3) { data[0] = dw1; data[1] = dw2; data[2] = dw3; }
    FpuPacked(const FpuExtended &value, int kfactor, FpuRoundingMode mode, ExcHandler handler = [](auto&&...) {});
    FpuPacked(const class FPUReg &value, int kfactor, FpuRoundingMode mode, ExcHandler handler = [](auto&&...) {});
};

}
