// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "softfloat.h"

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

    u8 raw = 0;

    FpuByte(u8 value) : raw(value) { };
    FpuByte(const FpuExtended &value, ExceptionHandler handler); 
};


//
// FpuWord
//

struct FpuWord {

    u16 raw;

    FpuWord() : raw(0) { };
    FpuWord(u16 value) : raw(value) { };
    FpuWord(const FpuExtended &value, ExceptionHandler handler);
};


//
// FpuLong
//

struct FpuLong {

    u32 raw;

    FpuLong() : raw(0) { };
    FpuLong(u32 value) : raw(value) { };
    FpuLong(const FpuExtended &value, ExceptionHandler handler);
};


//
// FpuSingle
//

struct FpuSingle {

    u32 raw;

    FpuSingle() : raw(0) { };
    FpuSingle(u32 value) : raw(value) { };
    FpuSingle(const class FpuExtended &value, ExceptionHandler handler);
};


//
// FpuDouble
//

struct FpuDouble {

    u64 raw;

    FpuDouble() : raw(0) { };
    FpuDouble(u64 value) : raw(value) { };
    FpuDouble(const class FpuExtended &value, ExceptionHandler handler);
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
    FpuExtended(long double value, FpuRoundingMode mode, ExceptionHandler handler = [](auto&&...) {});
    FpuExtended(bool mSign, i16 e, u64 m, ExceptionHandler handler = [](auto&&...) {});


    //
    // Constants
    //

    static FpuExtended NaN() { return FpuExtended(0x7FFF, 0xFFFFFFFFFFFFFFFF); }
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

    // DEPRECATED
    u8 asByte(ExceptionHandler handler) const;
    u16 asWord(ExceptionHandler handler) const;
    u32 asLong(ExceptionHandler handler) const;

    double asDouble() const;
    long double asLongDouble() const;
    long asLong() const;
    struct FpuPacked asPacked(int kfactor, FpuRoundingMode mode, u32 *statusbits) const;


    //
    // Arithmetic
    //

    void normalize();


    //
    // Operators
    //

    inline bool operator==(const FpuExtended &other) const {

        return raw.high == other.raw.high && raw.low == other.raw.low;
    }
};


//
// FpuPacked
//

struct FpuPacked {

    u32 data[3] = { };

    // FpuPacked(const FpuExtended &value, int kfactor, FpuRoundingMode mode, ExceptionHandler handler);
};

}
