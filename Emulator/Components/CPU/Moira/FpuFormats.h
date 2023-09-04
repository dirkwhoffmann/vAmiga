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

typedef std::function<void(u32)> ExceptionHandler;


//
// FpuByte
//

struct FpuByte {

    i8 raw;

    FpuByte() : raw(0) { };
    FpuByte(i8 value) : raw(value) { };
    FpuByte(u8 value) : raw((i8)value) { };
    FpuByte(const FpuExtended &value, ExceptionHandler handler = [](auto&&...) {});
    FpuByte(const class FPUReg &value, ExceptionHandler handler = [](auto&&...) {});
};


//
// FpuWord
//

struct FpuWord {

    i16 raw;

    FpuWord() : raw(0) { };
    FpuWord(i16 value) : raw(value) { };
    FpuWord(u16 value) : raw((i16)value) { };
    FpuWord(const FpuExtended &value, ExceptionHandler handler = [](auto&&...) {});
    FpuWord(const class FPUReg &value, ExceptionHandler handler = [](auto&&...) {});
};


//
// FpuLong
//

struct FpuLong {

    i32 raw;

    FpuLong() : raw(0) { };
    FpuLong(i32 value) : raw(value) { };
    FpuLong(u32 value) : raw((i32)value) { };
    FpuLong(const FpuExtended &value, ExceptionHandler handler = [](auto&&...) {});
    FpuLong(const class FPUReg &value, ExceptionHandler handler = [](auto&&...) {});
};


//
// FpuSingle
//

struct FpuSingle {

    u32 raw;

    FpuSingle() : raw(0) { };
    FpuSingle(u32 value) : raw(value) { };
    FpuSingle(const FpuExtended &value, ExceptionHandler handler = [](auto&&...) {});
    FpuSingle(const class FPUReg &value, ExceptionHandler handler = [](auto&&...) {});
};


//
// FpuDouble
//

struct FpuDouble {

    u64 raw;

    FpuDouble() : raw(0) { };
    FpuDouble(u64 value) : raw(value) { };
    FpuDouble(const FpuExtended &value, ExceptionHandler handler = [](auto&&...) {});
    FpuDouble(const class FPUReg &value, ExceptionHandler handler = [](auto&&...) {});
};


//
// FpuExtended
//

struct FpuExtended {

    softfloat::floatx80 raw;

    FpuExtended() { raw = { }; }
    FpuExtended(u16 high, u64 low) { raw.high = high; raw.low = low; }
    FpuExtended(const FpuByte &value, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const FpuWord &value, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const FpuLong &value, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const FpuSingle &value, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const FpuDouble &value, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const FpuPacked &value, FpuRoundingMode mode, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(const std::string &s, FpuRoundingMode mode, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(long double value, FpuRoundingMode mode = FPU_RND_NEAREST, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(bool mSign, i16 e, u64 m, ExceptionHandler handler = [](auto&&...) {});

    // Constants
    static FpuExtended nan;
    static FpuExtended posZero;
    static FpuExtended negZero;

    static constexpr i64 bias = 0x3FFF;

    // Operators
    bool operator==(const FpuExtended& rhs) const;
    bool operator!=(const FpuExtended& rhs) const;
    bool operator<=(const FpuExtended& rhs) const;
    bool operator>=(const FpuExtended& rhs) const;
    bool operator<(const FpuExtended& rhs) const;
    bool operator>(const FpuExtended& rhs) const;

    // Analyzers
    i64 sgn() const { return (raw.high & 0x8000) ? -1 : 1; }
    i64 exp() const { return i64(raw.high & 0x7FFF) - bias; }
    u64 man() const { return raw.low; }

    int fpclassify() const;
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isinf() const { return (raw.high & 0x7FFF) == 0x7FFF && raw.low == 0; }
    bool isnan() const { return (raw.high & 0x7FFF) == 0x7FFF && raw.low != 0; }
    bool isSignalingNaN() const { return isnan() && !(raw.low & (1L << 62)); }
    bool isNonsignalingNaN() const { return isnan() && (raw.low & (1L << 62)); }
    bool isnormal() const { return isfinite() && (raw.low & 1L << 63); }
    bool issubnormal() const { return isfinite() && raw.low && !(raw.low & 1L << 63); }
    bool signbit() const { return raw.high & 0x8000; }
    bool isnegative() const { return signbit(); }
    bool iszero() const { return (raw.high & 0x7FFF) == 0 && raw.low == 0; }


    //
    // Converters
    //

    // double asDouble() const;
    long double asLongDouble() const;


    //
    // Arithmetic
    //

    void normalize();
    std::pair<int, long double> frexp10() const;


    //
    // Operators
    //

    /*
    inline bool operator==(const FpuExtended &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }
    */
};


//
// FpuPacked
//

struct FpuPacked {

    u32 data[3] = { };

    FpuPacked() { }
    FpuPacked(u32 dw1, u32 dw2, u32 dw3) { data[0] = dw1; data[1] = dw2; data[2] = dw3; }
    FpuPacked(const FpuExtended &value, int kfactor, FpuRoundingMode mode, ExceptionHandler handler = [](auto&&...) {});
    FpuPacked(const class FPUReg &value, int kfactor, FpuRoundingMode mode, ExceptionHandler handler = [](auto&&...) {});
};

}
