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
#include "xdouble.h"

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
    // Handle some special cases
    if (value == 0.0) {

        raw = { };
        return;
    }

    // Extract the sign bit
    bool mSign = value < 0.0;
    value = std::abs(value);

    // Extract the exponent and the mantissa
    int e; auto m = frexpl(value, &e);

    printf("Float80(%.25Lf): %d %.25Lf\n", value, e, m);

    // Subtract one, because the first digit is left of the comma
    e -= 1;

    u64 mbits2;
    switch (mode) {
        case FPU_RND_NEAREST:   mbits2 = (u64)std::roundl(std::ldexpl(m, 64)); break;
        case FPU_RND_ZERO:      mbits2 = (u64)std::truncl(std::ldexpl(m, 64)); break;
        case FPU_RND_UPWARD:    mbits2 = (u64)std::ceill(std::ldexpl(m, 64)); break;
        default:                mbits2 = (u64)std::floorl(std::ldexpl(m, 64)); break;
    }
    printf("mbits2 = %llu\n", mbits2);

    // Create the bit representation of the mantissa
    u64 mbits = 0;
    for (int i = 63; i >= 0; i--) {
        m *= 2.0;
        if (m >= 1.0) {
            mbits |= (1L << i);
            m -= 1.0;
        } else {
            mbits &= ~(1L << i);
        }
    }

    printf("    mbits = %llx mbits2 = %llx\n", mbits, mbits2);

    *this = Float80(mSign, (i16)e, mbits);
}

Float80::Float80(xdb::XDouble<double> value, FpuRoundingMode mode)
{
    // Handle some special cases
    if (value == 0.0) {

        raw = { };
        return;
    }

    // Extract the sign bit
    bool mSign = value < 0.0;

    // Extract the exponent and the mantissa
    int e; auto m = value.frexp(&e);

    // Subtract one, because the first digit is left of the comma
    e -= 1;

    printf("Float80X(%.25Lf, %d): %d %.25Lf\n", (long double)value, mode, e, (long double)m);

/*
    switch (mode) {
        case FPU_RND_NEAREST:   printf("NEAREST: %s\n", m.ldexp(64).roundEven().ldexp(-64).to_string(30).c_str()); break;
        case FPU_RND_ZERO:      printf("TRUNC: %s\n", m.ldexp(64).trunc().ldexp(-64).to_string(30).c_str()); break;
        case FPU_RND_UPWARD:    printf("CEIL: %s\n", m.ldexp(64).ceil().ldexp(-64).to_string(30).c_str()); break;
        default:                printf("FLOOR: %s\n", m.ldexp(64).floor().ldexp(-64).to_string(30).c_str()); break;
    }
*/
/*
    u64 mbits2;
    switch (mode) {
        case FPU_RND_NEAREST:   mbits2 = (u64)(m.ldexp(64).roundEven().to_ulong_long()); break;
        case FPU_RND_ZERO:      mbits2 = (u64)(m.ldexp(64).trunc().to_ulong_long()); break;
        case FPU_RND_UPWARD:    mbits2 = (u64)(m.ldexp(64).ceil().to_ulong_long()); break;
        default:                mbits2 = (u64)(m.ldexp(64).floor().to_ulong_long()); break;
    }
    printf("mbits2 = %llu\n", mbits2);
*/
    xdb::XDouble<double> m2;

    printf("m ldexp 64: %s\n", m.ldexp(64).to_string(30).c_str());

    // Experimental
    // m = m.ldexp(68).round().ldexp(-68);
    // Round mantissa
    switch (mode) {
        case FPU_RND_NEAREST:   m2 = m.ldexp(64).roundEven(); break;
        case FPU_RND_ZERO:      m2 = m.ldexp(64).trunc(); break;
        case FPU_RND_UPWARD:    m2 = m.ldexp(64).ceil(); break;
        default:                m2 = m.ldexp(64).floor(); break;
    }
    printf("rounded:    %s\n", m2.to_string(30).c_str());


    // Create the bit representation of the mantissa
    /*
    u64 mbits = 0;
    for (int i = 63; i >= 0; i--) {
        m *= 2.0;
        if (m >= 1.0) {
            mbits |= (1L << i);
            m -= 1.0;
        } if (m <= -1.0) {
            mbits |= (1L << i);
            m += 1.0;
        } else {
            mbits &= ~(1L << i);
        }
    }
    */

    m2 = m2.abs();
    u64 mbits2 = 0;
    for (int i = 0; i < 64; i++) {
        m2 /= 2.0;
        auto m3 = m2.trunc();
        if (m2 - m3 > 0.25) {
            mbits2 |= (1L << i);
        } else {
            mbits2 &= ~(1L << i);
        }
        m2 = m3;
    }

    printf("    mbits2 = %llx\n", mbits2);
    printf("\n");

    *this = Float80(mSign, (i16)e, mbits2);
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
    // value = std::stold(s);
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
    auto result = std::ldexp((long double)man(), (int)exp() - 63);
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
    printf("isNormalized = %d\n", isNormalized());
    while (!isNormalized()) {

        raw.high -= 1;
        raw.low <<= 1;
    }
}

}
