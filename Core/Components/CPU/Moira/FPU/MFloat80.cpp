// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "MFloat80.h"
#include <bit>
#include <cfloat>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <limits>

namespace MOIRA_NAMESPACE {

MFloat80
MFloat80::fromInt(i64 value)
{
    if (value == 0) return zero();

    bool negative = value < 0;
    u64 magnitude = negative ? u64(0) - u64(value) : u64(value);
    int shift = std::countl_zero(magnitude);

    return { u16((bias + 63 - shift) | (negative ? 0x8000 : 0)), magnitude << shift };
}

MFloat80
MFloat80::fromLongDouble(long double value)
{
    if constexpr (LDBL_MANT_DIG == 64 && std::endian::native == std::endian::little) {

        // x87 extended format: 8 bytes mantissa followed by 2 bytes sign and exponent
        u64 m;
        u16 e;
        std::memcpy(&m, &value, sizeof(m));
        std::memcpy(&e, reinterpret_cast<const char *>(&value) + sizeof(m), sizeof(e));
        return fromBits(e, m);

    } else {

        // Convert via double
        double d = (double)value;
        if (d == 0) return zero(std::signbit(d));
        if (d != d) return quietNaN(std::signbit(d));
        if (std::isinf(d)) return infinity(d < 0);

        int e;
        double f = std::frexp(std::fabs(d), &e);
        return fromBits(u16((e - 1 + bias) | (d < 0 ? 0x8000 : 0)), u64(std::ldexp(f, 64)));
    }
}

long double
MFloat80::toLongDouble() const
{
    if constexpr (LDBL_MANT_DIG == 64 && std::endian::native == std::endian::little) {

        long double result = 0;
        std::memcpy(&result, &man, sizeof(man));
        std::memcpy(reinterpret_cast<char *>(&result) + sizeof(man), &se, sizeof(se));
        return result;

    } else {

        if (isNaN()) return sign() ? -std::numeric_limits<long double>::quiet_NaN() : std::numeric_limits<long double>::quiet_NaN();
        if (isInfinite()) return sign() ? -INFINITY : INFINITY;
        long double r = std::ldexp((long double)man, exponent() - 63);
        return sign() ? -r : r;
    }
}

std::string
MFloat80::str() const
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "0x%04X:%016llX", (unsigned)se, (unsigned long long)man);
    return buf;
}

}
