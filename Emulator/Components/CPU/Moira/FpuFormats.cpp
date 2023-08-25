//// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "FpuFormats.h"
#include "MoiraFPU.h"
#include <sstream>
#include <cmath>

namespace vamiga::moira {

Float80::Float80(u32 value)
{
    raw = softfloat::int64_to_floatx80(i64(value));
}

Float80::Float80(double value)
{
    raw = softfloat::float64_to_floatx80(*((u64 *)&value));
}

Float80::Float80(long double value, FpuRoundingMode mode)
{
    // Handle special cases
    if (value == 0.0) { raw = { }; return; }

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
    *this = Float80(value < 0.0, (i16)e, (u64)std::abs(m));
}

Float80::Float80(u16 high, u64 low)
{
    raw.high = high;
    raw.low = low;
}

Float80::Float80(bool mSign, i16 e, u64 m)
{
    raw.high = (mSign ? 0x8000 : 0x0000) | (u16(e + 0x3FFF) & 0x7FFF);
    raw.low = m;
}

Float80::Float80(const std::string &s, FpuRoundingMode mode)
{
    long double value;
    sscanf(s.c_str(), "%Le", &value);
    printf("Float80::Float80(%s) -> %.20Lf\n", s.c_str(), value);
    *this = Float80(value, mode);
    printf("    %x, %llx\n", raw.high, raw.low);
    normalize();
}

Float80::Float80(const struct FPUReg &reg)
{
    *this = reg.val;
}

double
Float80::asDouble() const
{
    auto value = softfloat::floatx80_to_float64(raw);
    return *((double *)&value);
}

long double
Float80::asLongDouble() const
{
    auto result = std::ldexpl((long double)man(), (int)exp() - 63);
    return result * sgn();
}

long
Float80::asLong() const
{
    auto value = softfloat::floatx80_to_int64(raw);
    return (long)value;
}

std::pair<int, long double>
Float80::frexp10() const
{
    long double val = asLongDouble();
    int e = isZero() ? 0 : 1 + (int)std::floor(std::log10(std::fabs(val)));
    long double m = val * std::powl(10L, -e);

    printf("    frexp10: val = %.20Lf e = %d m = %.20Lf\n", val, e, m);

    return { e, m };
};


bool
Float80::isNegative() const
{
    return (raw.high & 0x8000);
}

bool
Float80::isZero() const
{
    return (raw.high & 0x7FFF) == 0 && raw.low == 0;
}

bool
Float80::isInfinity() const
{
    return (raw.high & 0x7FFF) == 0x7FFF && raw.low == 0;
}

bool
Float80::isNaN() const
{
    return (raw.high & 0x7FFF) == 0x7FFF && raw.low != 0;
}

bool
Float80::isSignalingNaN() const
{
    return isNaN() && (raw.low & (1L << 62)) == 0;
}

bool
Float80::isNonsignalingNaN() const
{
    return isNaN() && (raw.low & (1L << 62)) != 0;
}

bool
Float80::isNormalized() const
{
    if ((raw.high & 0x7FFF) == 0) return true;
    if (isNaN()) return true;

    return raw.low == 0 || (raw.low & (1L << 63)) != 0;
}

void
Float80::normalize()
{
    while (!isNormalized()) {

        raw.high -= 1;
        raw.low <<= 1;
    }
}

}
