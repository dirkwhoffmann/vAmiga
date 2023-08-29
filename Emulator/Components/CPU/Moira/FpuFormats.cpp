// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "config.h"
#include "FpuFormats.h"
#include "MoiraFPU.h"
#include <sstream>
#include <cmath>

namespace vamiga::moira {

//
// FpuByte
//

FpuByte::FpuByte(const FpuExtended &value, ExceptionHandler handler)
{
    u32 flags = 0;

    softfloat::float_exception_flags = 0;
    raw = u8(softfloat::floatx80_to_int32(value.raw));

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuByte::FpuByte(const FPUReg &reg, ExceptionHandler handler) : FpuByte(reg.val, handler) { }


//
// FpuWord
//

FpuWord::FpuWord(const FpuExtended &value, ExceptionHandler handler)
{
    u32 flags = 0;

    softfloat::float_exception_flags = 0;
    raw = u16(softfloat::floatx80_to_int32(value.raw));

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuWord::FpuWord(const FPUReg &reg, ExceptionHandler handler) : FpuWord(reg.val, handler) { }


//
// FpuLong
//

FpuLong::FpuLong(const FpuExtended &value, ExceptionHandler handler)
{
    u32 flags = 0;

    softfloat::float_exception_flags = 0;
    raw = u16(softfloat::floatx80_to_int32(value.raw));

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuLong::FpuLong(const FPUReg &reg, ExceptionHandler handler) : FpuLong(reg.val, handler) { }


//
// FpuSingle
//

FpuSingle::FpuSingle(const class FpuExtended &value, ExceptionHandler handler)
{
    u32 flags = 0;

    softfloat::float_exception_flags = 0;
    raw = floatx80_to_float32(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuSingle::FpuSingle(const FPUReg &reg, ExceptionHandler handler) : FpuSingle(reg.val, handler) { }


//
// FpuDouble
//

FpuDouble::FpuDouble(const class FpuExtended &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = floatx80_to_float64(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuDouble::FpuDouble(const FPUReg &reg, ExceptionHandler handler) : FpuDouble(reg.val, handler) { }


//
// FpuExtended
//

FpuExtended::FpuExtended(const FpuByte &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = softfloat::int64_to_floatx80(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuExtended::FpuExtended(const FpuWord &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = softfloat::int64_to_floatx80(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuExtended::FpuExtended(const FpuLong &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = softfloat::int64_to_floatx80(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuExtended::FpuExtended(const FpuSingle &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = softfloat::float32_to_floatx80(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuExtended::FpuExtended(const FpuDouble &value, ExceptionHandler handler)
{
    u32 flags = 0;
    softfloat::float_exception_flags = 0;

    raw = softfloat::float64_to_floatx80(value.raw);

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        flags |= FPEXP_INEX2;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        flags |= FPEXP_OVFL;
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        flags |= FPEXP_UNFL;
    }

    handler(flags);
}

FpuExtended::FpuExtended(const FpuPacked &packed, FpuRoundingMode mode, ExceptionHandler handler)
{
    char str[128], *ch = str;
    i32 ex = 0; u64 mal = 0, mar = 0;

    auto dw1 = packed.data[0];
    auto dw2 = packed.data[1];
    auto dw3 = packed.data[2];

    // Extract the sign bits
    auto msign = bool(dw1 & 0x80000000);
    auto esign = bool(dw1 & 0x40000000);

    // Compose the exponent
    ex = (char)((dw1 >> 24) & 0xF);
    ex = ex * 10 + (char)((dw1 >> 20) & 0xF);
    ex = ex * 10 + (char)((dw1 >> 16) & 0xF);

    // Compose the fractional part of the mantissa
    mar = (char)((dw2 >> 28) & 0xF);
    mar = mar * 10 + (char)((dw2 >> 24) & 0xF);
    mar = mar * 10 + (char)((dw2 >> 20) & 0xF);
    mar = mar * 10 + (char)((dw2 >> 16) & 0xF);
    mar = mar * 10 + (char)((dw2 >> 12) & 0xF);
    mar = mar * 10 + (char)((dw2 >> 8)  & 0xF);
    mar = mar * 10 + (char)((dw2 >> 4)  & 0xF);
    mar = mar * 10 + (char)((dw2 >> 0)  & 0xF);
    mar = mar * 10 + (char)((dw3 >> 28) & 0xF);
    mar = mar * 10 + (char)((dw3 >> 24) & 0xF);
    mar = mar * 10 + (char)((dw3 >> 20) & 0xF);
    mar = mar * 10 + (char)((dw3 >> 16) & 0xF);
    mar = mar * 10 + (char)((dw3 >> 12) & 0xF);
    mar = mar * 10 + (char)((dw3 >> 8)  & 0xF);
    mar = mar * 10 + (char)((dw3 >> 4)  & 0xF);
    mar = mar * 10 + (char)((dw3 >> 0)  & 0xF);

    // Compose the integer part of the mantissa
    mal = (char)((dw1 >> 0) & 0xF);
    mal += mar / 10000000000000000;
    mar %= 10000000000000000;

    // Check for special cases
    if (ex == 1665) {

        if (mar == 0) {

            if (((dw1 >> 28) & 0x7) == 0x7) {
                *this = FpuExtended(msign ? 0xFFFF : 0x7FFF, 0); // Infinity
                return;
            } else {
                *this = FpuExtended(msign ? 0x8000 : 0, 0); // ?
                return;
            }

        } else {

            if (((dw1 >> 28) & 0x7) == 0x7) {
                *this = FpuExtended(msign ? 0xFFFF : 0x7FFF, u64(dw2) << 32 | dw3); // NaN
                return;
            } else {
                // *this = Float80(msign ? 0x8000 : 0, 0); // ?
                // return;
            }
        }
    }

    // Write the integer part of the mantissa
    if (msign) *ch++ = '-';
    for (int i = 1; i >= 0; i--) { ch[i] = (mal % 10) + '0'; mal /= 10; }
    ch += 2;

    // Write the fractional part of the mantissa
    *ch++ = '.';
    for (int i = 15; i >= 0; i--) { ch[i] = (mar % 10) + '0'; mar /= 10; }
    ch += 16;

    // Write the exponent
    *ch++ = 'E';
    if (esign) *ch++ = '-';
    for (int i = 3; i >= 0; i--) { ch[i] = (ex % 10) + '0'; ex /= 10; }
    ch += 4;

    // Terminate the string
    *ch = 0;

    *this = FpuExtended(str, mode, handler);
}

FpuExtended::FpuExtended(const std::string &s, FpuRoundingMode mode, ExceptionHandler handler)
{
    long double value;

    auto old = FPU::fesetround(mode);
    sscanf(s.c_str(), "%Le", &value);
    FPU::fesetround(old);

    *this = FpuExtended(value, mode);
    normalize();
}

FpuExtended::FpuExtended(long double value, FpuRoundingMode mode, ExceptionHandler handler)
{
    // Handle special cases
    if (value == 0.0) { *this = std::signbit(value) ? negZero : posZero; return; }

    // Extract the exponent and the mantissa
    int e; auto m = frexpl(value, &e);

    // Subtract one, because the first digit is left of the comma
    e -= 1;

    // Round the mantissa
    switch (mode) {
        case FPU_RND_NEAREST:   m = std::round(std::ldexpl(m, 64)); break;
        case FPU_RND_ZERO:      m = std::truncl(std::ldexpl(m, 64)); break;
        case FPU_RND_UPWARD:    m = std::ceill(std::ldexpl(m, 64)); break;
        default:                m = std::floorl(std::ldexpl(m, 64)); break;
    }

    // Compose the result
    *this = FpuExtended(value < 0.0, (i16)e, (u64)std::abs(m));
}

FpuExtended::FpuExtended(bool mSign, i16 e, u64 m, ExceptionHandler handler)
{
    raw.high = (mSign ? 0x8000 : 0x0000) | (u16(e + 0x3FFF) & 0x7FFF);
    raw.low = m;

    handler(0);
}

FpuExtended FpuExtended::nan     = FpuExtended(0x7FFF, 0xFFFFFFFFFFFFFFFF);
FpuExtended FpuExtended::posZero = FpuExtended(0x0000, 0x0000000000000000);
FpuExtended FpuExtended::negZero = FpuExtended(0x8000, 0x0000000000000000);

double
FpuExtended::asDouble() const
{
    auto value = softfloat::floatx80_to_float64(raw);
    return *((double *)&value);
}

long double
FpuExtended::asLongDouble() const
{
    auto result = std::ldexpl((long double)man(), (int)exp() - 63);
    return result * sgn();
}

bool
FpuExtended::isNegative() const
{
    return (raw.high & 0x8000);
}

bool
FpuExtended::isZero() const
{
    return (raw.high & 0x7FFF) == 0 && raw.low == 0;
}

bool
FpuExtended::isfinite() const
{
    return !isNaN() && !isInfinity();
}

bool
FpuExtended::isInfinity() const
{
    return (raw.high & 0x7FFF) == 0x7FFF && raw.low == 0;
}

bool
FpuExtended::isNaN() const
{
    return (raw.high & 0x7FFF) == 0x7FFF && raw.low != 0;
}

bool
FpuExtended::isSignalingNaN() const
{
    return isNaN() && (raw.low & (1L << 62)) == 0;
}

bool
FpuExtended::isNonsignalingNaN() const
{
    return isNaN() && (raw.low & (1L << 62)) != 0;
}

bool
FpuExtended::isNormalized() const
{
    if ((raw.high & 0x7FFF) == 0) return true;
    if (isNaN()) return true;

    return raw.low == 0 || (raw.low & (1L << 63)) != 0;
}

void
FpuExtended::normalize()
{
    while (!isNormalized()) {

        raw.high -= 1;
        raw.low <<= 1;
    }
}

std::pair<int, long double>
FpuExtended::frexp10() const
{
    long double val = asLongDouble();
    int e = isZero() ? 0 : 1 + (int)std::floor(std::log10(std::fabs(val)));
    long double m = val * std::powl(10L, -e);

    return { e, m };
};


//
// FpuPacked
//

FpuPacked::FpuPacked(const FpuExtended &value, int k, FpuRoundingMode mode, ExceptionHandler handler)
{
    u32 statusbits = 0;

    // Get exponent
    auto e = value.frexp10().first - 1;

    // Check k-factor
    if (k > 17) {
        statusbits = FPEXP_OPERR | FPEXP_INEX2;
        k = 17;
    }
    if (k < -17) {
        k = -17;
    }

    // Setup stringstream
    std::stringstream ss;
    long double test;
    ss.setf(std::ios::scientific, std::ios::floatfield);
    ss.precision(k > 0 ? k - 1 : e - k);

    // Create string representation
    auto ldval = value.asLongDouble();
    auto old = FPU::fesetround(mode);
    ss << ldval;
    std::stringstream ss2(ss.str());
    ss2 >> test;
    FPU::fesetround(old);

    if (ldval != test) {
        statusbits |= FPEXP_INEX2;
    }
    
    // Assemble exponent
    data[0] = e < 0 ? 0x40000000 : 0;
    data[0] |= (e % 10) << 16; e /= 10;
    data[0] |= (e % 10) << 20; e /= 10;
    data[0] |= (e % 10) << 24;

    // Assemble mantisse
    char c;
    int shift = 64;

    while (ss.get(c)) {

        if (c == '+') continue;
        if (c == '-') data[0] |= 0x80000000;
        if (c >= '0' && c <= '9') {
            if (shift == 64) data[0] |= u32(c - '0');
            else if (shift >= 32) data[1] |= u32(c - '0') << (shift - 32);
            else if (shift >= 0) data[2] |= u32(c - '0') << shift;
            shift -= 4;
        }
        if (c == 'e' || c ==  'E') break;
    }

    handler(statusbits);
}

FpuPacked::FpuPacked(const FPUReg &reg, int k, FpuRoundingMode mode,
                     ExceptionHandler handler) : FpuPacked(reg.val, k, mode, handler) { }

}
