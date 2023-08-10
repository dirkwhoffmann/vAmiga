//// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "Float80.h"
#include "MoiraFPU.h"
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

Float80::Float80(long double value)
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

    // Subtract one, because the first digit is left of the comma
    e -= 1;

    // Create the bit representation of the mantissa
    u64 mbits = 0;
    for (isize i = 63; i >= 0; i--) {
        m *= 2.0;
        if (m >= 1.0) {
            mbits |= (1L << i);
            m -= 1.0;
        } else {
            mbits &= ~(1L << i);
        }
    }

    *this = Float80(mSign, (i16)e, mbits);
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
    auto result = std::pow((long double)man(), -(long double)exp() - 1);
    return sgn() ? -result : result;
}

long
Float80::asLong() const
{
    auto value = softfloat::floatx80_to_int64(raw);
    return (long)value;
}

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
