// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* The rules for propagating NaNs (propagateNaN) and the conventions for
 * out-of-range integer conversions follow Berkeley SoftFloat, Release 3e, by
 * John R. Hauser. Its license is reproduced below. Everything else in this file
 * operates on unpacked numbers and is independent of that library.
 *
 * Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2018 The Regents of the
 * University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions, and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the University nor the names of its contributors may
 *     be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "FpuEnv.h"
#include "Wide.h"
#include <algorithm>
#include <bit>
#include <climits>

namespace MOIRA_NAMESPACE {

namespace {

constexpr i32 bias = MFloat80::bias;

/* An unpacked number. Finite non-zero numbers are normalized: The exponent is the
 * biased exponent that applies to the normalized significand (bit 63 is set).
 * It is not limited to the range of the format. The value is
 * sig * 2^(exp - bias - 63).
 */
struct Num {

    enum Kind { ZERO, FINITE, INF, NaN } kind;
    bool sign;
    i32 exp = 0;
    u64 sig = 0;

    explicit Num(const MFloat80 &x) : sign(x.sign())
    {
        if (x.isNaN()) { kind = NaN; return; }
        if (x.isInfinite()) { kind = INF; return; }
        if (x.mantissa() == 0) { kind = ZERO; return; }

        kind = FINITE;
        int shift = std::countl_zero(x.mantissa());
        sig = x.mantissa() << shift;
        exp = x.biasedExponent() - shift;
    }

    bool isZero() const { return kind == ZERO; }
    bool isInf() const { return kind == INF; }
    bool isNaN() const { return kind == NaN; }
    bool isFinite() const { return kind == FINITE; }
};

// Packs a normalized number that is exactly representable
MFloat80 pack(bool sign, i32 exp, u64 sig)
{
    u16 s = sign ? 0x8000 : 0;
    if (exp >= 0) return MFloat80::fromBits(u16(exp) | s, sig);

    // Denormal
    return MFloat80::fromBits(s, -exp >= 64 ? 0 : sig >> -exp);
}

// Decides whether the magnitude of a rounded result has to be incremented
bool increment(RoundingMode mode, bool sign, bool roundBit, bool rest, bool odd)
{
    switch (mode) {

        case RoundingMode::NEAREST: return roundBit && (rest || odd);
        case RoundingMode::ZERO:    return false;
        case RoundingMode::DOWN:    return sign && (roundBit || rest);
        case RoundingMode::UP:      return !sign && (roundBit || rest);
    }
    return false;
}

// Returns a normalized sign:extra pair for a 128-bit magnitude
void split(u128 value, u64 &sig, u64 &extra) { sig = hi64(value); extra = lo64(value); }

// Computes the integer square root of a 128-bit number
u64 isqrt(u128 n, u128 &remainder)
{
    u128 result = 0;
    u128 bit = u128(1) << 126;

    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    remainder = n;
    return u64(result);
}

}


//
// Environment
//

FpuEnv::Scope::Scope(FpuEnv &env, bool restoreFlags) : env(env), restoreFlags(restoreFlags)
{
    savedMode = env.roundingMode();
    savedPrecision = env.precision();
    savedLimits = env.limitsExponent();
    savedFlags = env.flags();
}

FpuEnv::Scope::~Scope()
{
    env.setRoundingMode(savedMode);
    env.setPrecision(savedPrecision);
    env.setLimitsExponent(savedLimits);

    if (restoreFlags) {
        env.clearFlags();
        env.raiseFlags(savedFlags);
    }
}


//
// Rounding
//

MFloat80
FpuEnv::roundPack(bool sign, i32 exp, u64 sig, u64 extra)
{
    /* The value is given as sign, biased exponent and a normalized significand
     * (integer bit set) plus up to 64 additional bits below the significand.
     * This is the rounding step of the Motorola FPUs:
     *
     *  - A result beyond the largest number of the format raises OVFL and INEX2
     *    and becomes infinity, or the largest number if the rounding mode points
     *    towards zero.
     *  - A result below the smallest normalized number raises UNFL (tininess is
     *    detected before rounding, even if the result is exact) and is
     *    denormalized. Denormals are stored with an unnormalized significand and
     *    the exponent of the smallest normalized number. The extended format is
     *    special, because a denormal has the biased exponent 0 whereas the
     *    smallest normalized number has the biased exponent 1. A number with the
     *    exponent 0 and the integer bit set is thus a regular result that is not
     *    tiny.
     */
    if (sig == 0 && extra == 0) return MFloat80::zero(sign);
    if (sig == 0) { sig = extra; extra = 0; exp -= 64; }

    int shift = std::countl_zero(sig);
    sig = shift ? sig << shift | extra >> (64 - shift) : sig;
    extra <<= shift;
    exp -= shift;

    // Select the format. With a reduced precision, the exponent range of the
    // respective format applies (single: 0x3F80, double: 0x3C00).
    int bits = prec == Precision::SINGLE ? 24 : prec == Precision::DOUBLE ? 53 : 64;
    i32 offset = 0;
    if (limitExponent) offset = prec == Precision::SINGLE ? 0x3F80 : prec == Precision::DOUBLE ? 0x3C00 : 0;
    i32 lowest = offset ? offset + 1 : 0;
    i32 maxExp = 0x7FFE - offset;

    u128 t = makeU128(sig, extra);
    bool sticky = false;

    // Denormalize
    bool tiny = exp < lowest;
    if (tiny) {
        i32 dist = lowest - exp;
        if (dist >= 128) {
            sticky = t != 0;
            t = 0;
        } else {
            sticky = (t & ((u128(1) << dist) - 1)) != 0;
            t >>= dist;
        }
    }

    // Round
    int drop = 128 - bits;
    u128 unit = u128(1) << drop;
    u128 half = unit >> 1;
    u128 frac = t & (unit - 1);
    u128 kept = t >> drop;
    bool inexact = frac != 0 || sticky;
    bool roundBit = (frac & half) != 0;
    bool rest = (frac & (half - 1)) != 0 || sticky;

    kept += increment(mode, sign, roundBit, rest, (kept & 1) != 0);
    if (!tiny && (kept >> bits)) { kept >>= 1; exp++; }

    // Overflow
    if (exp > maxExp) {

        raiseFlags(FloatFlags::OVFL | FloatFlags::INEXACT);
        bool towardsZero = mode == RoundingMode::ZERO ||
                           (sign && mode == RoundingMode::UP) || (!sign && mode == RoundingMode::DOWN);
        u64 largest = bits == 64 ? ~0ULL : (~0ULL << (64 - bits));
        return towardsZero ? MFloat80::fromBits(u16(maxExp | (sign ? 0x8000 : 0)), largest)
                           : MFloat80::infinity(sign);
    }

    if (inexact) raiseFlags(FloatFlags::INEXACT);
    if (tiny) raiseFlags(FloatFlags::UNFL);

    // A result that rounds to zero is a regular zero
    if (kept == 0) return MFloat80::zero(sign);

    u64 result = u64(kept) << (64 - bits);
    return MFloat80::fromBits(u16((tiny ? lowest : exp) | (sign ? 0x8000 : 0)), result);
}

MFloat80
FpuEnv::round(const MFloat80 &a)
{
    if (!a.isFinite() || a.mantissa() == 0) return a;

    // Regular extended precision values are not changed
    if (a.isNormal() && prec == Precision::EXTENDED) return a;

    Num x(a);
    return roundPack(x.sign, x.exp, x.sig);
}


//
// NaNs
//

MFloat80
FpuEnv::propagateNaN(const MFloat80 &a, const MFloat80 &b)
{
    // Infinities are looked at in the encoding of the x87 (integer bit set)
    auto mantissa = [](const MFloat80 &x) { return x.isInfinite() ? 1ULL << 63 : x.mantissa(); };

    u16 a64 = a.signExp(), b64 = b.signExp();
    u64 a0 = mantissa(a), b0 = mantissa(b);
    bool sigA = a.isSignalingNaN(), sigB = b.isSignalingNaN();

    auto resultA = [&] { return MFloat80::fromBits(a64, a0 | 0xC000000000000000ULL); };
    auto resultB = [&] { return MFloat80::fromBits(b64, b0 | 0xC000000000000000ULL); };

    if (sigA || sigB) {

        raiseFlags(FloatFlags::INVALID);

        if (!sigA) return a.isNaN() ? resultA() : resultB();
        if (!sigB) return b.isNaN() ? resultB() : resultA();
    }

    // Return the NaN with the larger magnitude
    u16 magA = a64 & 0x7FFF, magB = b64 & 0x7FFF;
    if (magA < magB) return resultB();
    if (magB < magA) return resultA();
    if (a0 < b0) return resultB();
    if (b0 < a0) return resultA();
    if (a64 < b64) return resultA();
    return resultB();
}

MFloat80
FpuEnv::invalidOperation()
{
    raiseFlags(FloatFlags::INVALID);
    return MFloat80::defaultNaN();
}


//
// Arithmetic
//

MFloat80
FpuEnv::addSub(const MFloat80 &a, const MFloat80 &b, bool negate)
{
    Num x(a), y(b);

    if (x.isNaN() || y.isNaN()) return propagateNaN(a, b);
    if (negate) y.sign = !y.sign;

    // Infinities
    if (x.isInf()) return y.isInf() && x.sign != y.sign ? invalidOperation() : MFloat80::infinity(x.sign);
    if (y.isInf()) return MFloat80::infinity(y.sign);

    // Zeros
    if (x.isZero() && y.isZero())
        return MFloat80::zero(x.sign == y.sign ? x.sign : mode == RoundingMode::DOWN);
    if (y.isZero()) return roundPack(x.sign, x.exp, x.sig);
    if (x.isZero()) return roundPack(y.sign, y.exp, y.sig);

    // Make x the operand with the larger magnitude
    if (x.exp < y.exp || (x.exp == y.exp && x.sig < y.sig)) std::swap(x, y);

    // Align the operands. One bit of headroom is left for a carry.
    u128 big = u128(x.sig) << 63;
    u128 small = shiftRightJam128(u128(y.sig) << 63, x.exp - y.exp);

    u128 result = x.sign == y.sign ? big + small : big - small;
    if (result == 0) return MFloat80::zero(mode == RoundingMode::DOWN);

    int lz = clz128(result);
    u64 sig, extra;
    split(result << lz, sig, extra);

    return roundPack(x.sign, x.exp - lz + 1, sig, extra);
}

MFloat80 FpuEnv::add(const MFloat80 &a, const MFloat80 &b) { return addSub(a, b, false); }
MFloat80 FpuEnv::sub(const MFloat80 &a, const MFloat80 &b) { return addSub(a, b, true); }

MFloat80
FpuEnv::mul(const MFloat80 &a, const MFloat80 &b)
{
    Num x(a), y(b);
    bool sign = x.sign != y.sign;

    if (x.isNaN() || y.isNaN()) return propagateNaN(a, b);

    // Infinities and zeros
    if (x.isInf() || y.isInf()) return x.isZero() || y.isZero() ? invalidOperation() : MFloat80::infinity(sign);
    if (x.isZero() || y.isZero()) return MFloat80::zero(sign);

    // The product has 127 or 128 bits
    u128 p = u128(x.sig) * y.sig;
    i32 exp = x.exp + y.exp - bias + 1;
    if (!(p >> 127)) { p <<= 1; exp--; }

    return roundPack(sign, exp, hi64(p), lo64(p));
}

MFloat80
FpuEnv::div(const MFloat80 &a, const MFloat80 &b)
{
    Num x(a), y(b);
    bool sign = x.sign != y.sign;

    if (x.isNaN() || y.isNaN()) return propagateNaN(a, b);

    // Infinities
    if (x.isInf()) return y.isInf() ? invalidOperation() : MFloat80::infinity(sign);
    if (y.isInf()) return MFloat80::zero(sign);

    // Zeros
    if (y.isZero()) {
        if (x.isZero()) return invalidOperation();
        raiseFlags(FloatFlags::DIV_BY_ZERO);
        return MFloat80::infinity(sign);
    }
    if (x.isZero()) return MFloat80::zero(sign);

    // The quotient has 64 or 65 bits
    u128 n = u128(x.sig) << 64;
    u128 q = n / y.sig;
    u64 r = u64(n % y.sig);
    u64 extra;
    i32 exp;

    if (q >> 64) {

        // The least significant bit becomes the round bit
        exp = x.exp - y.exp + bias;
        extra = u64(q & 1) << 63 | (r != 0);
        q >>= 1;

    } else {

        // Compute 64 more bits
        exp = x.exp - y.exp + bias - 1;
        u128 m = u128(r) << 64;
        extra = u64(m / y.sig);
        extra |= u64((m % y.sig) != 0);
    }
    return roundPack(sign, exp, u64(q), extra);
}

MFloat80
FpuEnv::sqrt(const MFloat80 &a)
{
    Num x(a);

    if (x.isNaN()) return propagateNaN(a, MFloat80());
    if (x.isZero()) return a.isZero() ? a : MFloat80::zero(x.sign);
    if (x.sign) return invalidOperation();
    if (x.isInf()) return MFloat80::infinity();

    // Write the value as X * 2^(2k), where X is a 128-bit integer
    i32 e = x.exp - bias - 63;
    int shift = (e & 1) ? 63 : 64;
    u128 n = u128(x.sig) << shift;
    i32 exp = (e - shift) / 2 + bias + 63;

    u128 remainder;
    u64 root = isqrt(n, remainder);

    // The remainder tells if the exact root is above the round bit or inexact
    u64 extra = (remainder > root ? 1ULL << 63 : 0) | u64(remainder != 0);

    return roundPack(false, exp, root, extra);
}

MFloat80
FpuEnv::roundToInt(const MFloat80 &a)
{
    Num x(a);

    if (x.isNaN()) return propagateNaN(a, MFloat80());
    if (x.isInf()) return MFloat80::infinity(x.sign);
    if (x.isZero()) return MFloat80::zero(x.sign);

    i32 e = x.exp - bias;
    if (e >= 63) return pack(x.sign, x.exp, x.sig);

    // Split into integer part and fraction
    u64 ip;
    bool roundBit, rest;
    int s = 63 - e;
    if (s >= 65) {
        ip = 0; roundBit = false; rest = true;
    } else if (s == 64) {
        ip = 0; roundBit = true; rest = (x.sig << 1) != 0;
    } else {
        ip = x.sig >> s;
        roundBit = (x.sig >> (s - 1)) & 1;
        rest = (x.sig & ((1ULL << (s - 1)) - 1)) != 0;
    }
    if (!roundBit && !rest) return pack(x.sign, x.exp, x.sig);

    raiseFlags(FloatFlags::INEXACT);
    u64 n = ip + increment(mode, x.sign, roundBit, rest, ip & 1);
    if (n == 0) return MFloat80::zero(x.sign);

    int lz = std::countl_zero(n);
    return pack(x.sign, bias + 63 - lz, n << lz);
}

MFloat80
FpuEnv::scale(const MFloat80 &a, const MFloat80 &b)
{
    // NaNs propagate (and signaling NaNs raise the invalid flag)
    if (a.isNaN() || b.isNaN()) return add(a, b);

    // Scaling by an infinite factor is undefined
    if (b.isInfinite()) {
        raiseFlags(FloatFlags::INVALID);
        return MFloat80::quietNaN();
    }

    // Zeros and infinities are not changed
    if (a.isZero() || a.isInfinite()) return a;

    // Determine the scale factor. Factors beyond the exponent range of the
    // format are clamped, because they overflow or underflow anyway.
    i32 n;
    if (b.exponent() >= 20) {
        n = b.sign() ? -0x100000 : 0x100000;
    } else {
        // Truncating discards the fraction without being inexact
        auto saved = flags();
        n = toInt32Truncated(b);
        clearFlags();
        raiseFlags(saved);
    }

    Num x(a);
    if (x.isZero()) return MFloat80::zero(x.sign);

    // The rounding step takes care of overflows and underflows
    return roundPack(x.sign, x.exp + std::clamp(n, -0x100000, 0x100000), x.sig);
}


//
// Comparing
//

namespace {

// Compares two numbers that are not NaNs
int compare(const Num &a, const Num &b)
{
    if (a.isZero() && b.isZero()) return 0;
    if (a.isZero()) return b.sign ? 1 : -1;
    if (b.isZero()) return a.sign ? -1 : 1;
    if (a.sign != b.sign) return a.sign ? -1 : 1;

    i32 ea = a.isInf() ? INT_MAX : a.exp, eb = b.isInf() ? INT_MAX : b.exp;
    int magnitude = ea != eb ? (ea < eb ? -1 : 1) : (a.sig == b.sig ? 0 : (a.sig < b.sig ? -1 : 1));

    return a.sign ? -magnitude : magnitude;
}

}

bool
FpuEnv::eq(const MFloat80 &a, const MFloat80 &b)
{
    if (a.isNaN() || b.isNaN()) {
        if (a.isSignalingNaN() || b.isSignalingNaN()) raiseFlags(FloatFlags::INVALID);
        return false;
    }
    return compare(Num(a), Num(b)) == 0;
}

bool
FpuEnv::lt(const MFloat80 &a, const MFloat80 &b)
{
    if (a.isNaN() || b.isNaN()) { raiseFlags(FloatFlags::INVALID); return false; }
    return compare(Num(a), Num(b)) < 0;
}

bool
FpuEnv::le(const MFloat80 &a, const MFloat80 &b)
{
    if (a.isNaN() || b.isNaN()) { raiseFlags(FloatFlags::INVALID); return false; }
    return compare(Num(a), Num(b)) <= 0;
}


//
// Converting
//

MFloat80
FpuEnv::fromFloat(float value)
{
    u32 ui = std::bit_cast<u32>(value);
    bool sign = ui >> 31;
    u32 ex = (ui >> 23) & 0xFF, frac = ui & 0x7FFFFF;

    if (ex == 0xFF) {

        if (!frac) return MFloat80::infinity(sign);
        if (!(ui & 0x400000)) raiseFlags(FloatFlags::INVALID);
        return MFloat80::fromBits(u16(sign << 15 | 0x7FFF), 0xC000000000000000ULL | (u64(ui) << 41) >> 1);
    }
    if (ex == 0) {

        if (!frac) return MFloat80::zero(sign);
        int lz = std::countl_zero(u64(frac));
        return MFloat80::fromBits(u16((bias + 63 - lz - 149) | (sign << 15)), u64(frac) << lz);
    }
    return MFloat80::fromBits(u16((ex - 127 + bias) | (sign << 15)), u64(frac | 0x800000) << 40);
}

MFloat80
FpuEnv::fromDouble(double value)
{
    u64 ui = std::bit_cast<u64>(value);
    bool sign = ui >> 63;
    u64 ex = (ui >> 52) & 0x7FF, frac = ui & 0xFFFFFFFFFFFFFULL;

    if (ex == 0x7FF) {

        if (!frac) return MFloat80::infinity(sign);
        if (!(ui & 0x8000000000000ULL)) raiseFlags(FloatFlags::INVALID);
        return MFloat80::fromBits(u16(sign << 15 | 0x7FFF), 0xC000000000000000ULL | (ui << 12) >> 1);
    }
    if (ex == 0) {

        if (!frac) return MFloat80::zero(sign);
        int lz = std::countl_zero(frac);
        return MFloat80::fromBits(u16((bias + 63 - lz - 1074) | (sign << 15)), frac << lz);
    }
    return MFloat80::fromBits(u16((i32(ex) - 1023 + bias) | (sign << 15)), (frac | 0x10000000000000ULL) << 11);
}

i64
FpuEnv::toInteger(const MFloat80 &a, RoundingMode m, int bits)
{
    i64 overflow = bits == 32 ? i64(INT32_MIN) : INT64_MIN;
    Num x(a);

    if (x.isNaN() || x.isInf()) { raiseFlags(FloatFlags::INVALID); return overflow; }
    if (x.isZero()) return 0;

    i32 e = x.exp - bias;
    if (e >= 64) { raiseFlags(FloatFlags::INVALID); return overflow; }

    // Split into integer part and fraction
    u64 ip;
    bool roundBit = false, rest = false;
    if (e == 63) {
        ip = x.sig;
    } else {
        int s = 63 - e;
        if (s >= 65) {
            ip = 0; rest = true;
        } else if (s == 64) {
            ip = 0; roundBit = true; rest = (x.sig << 1) != 0;
        } else {
            ip = x.sig >> s;
            roundBit = (x.sig >> (s - 1)) & 1;
            rest = (x.sig & ((1ULL << (s - 1)) - 1)) != 0;
        }
    }
    u128 magnitude = u128(ip) + increment(m, x.sign, roundBit, rest, ip & 1);
    u128 limit = (u128(1) << (bits - 1)) - (x.sign ? 0 : 1);

    if (magnitude > limit) { raiseFlags(FloatFlags::INVALID); return overflow; }
    if (roundBit || rest) raiseFlags(FloatFlags::INEXACT);

    return x.sign ? i64(u64(0) - u64(magnitude)) : i64(magnitude);
}

i32 FpuEnv::toInt32(const MFloat80 &a) { return (i32)toInteger(a, mode, 32); }
i32 FpuEnv::toInt32Truncated(const MFloat80 &a) { return (i32)toInteger(a, RoundingMode::ZERO, 32); }
i64 FpuEnv::toInt64(const MFloat80 &a) { return toInteger(a, mode, 64); }
i64 FpuEnv::toInt64Truncated(const MFloat80 &a) { return toInteger(a, RoundingMode::ZERO, 64); }

u64
FpuEnv::toIeee(const MFloat80 &a, int bits)
{
    const bool single = bits == 32;
    const int p = single ? 24 : 53;                 // Significand bits
    const i32 emin = single ? -126 : -1022;
    const i32 emax = single ? 127 : 1023;
    const u64 fieldMax = single ? 0xFF : 0x7FF;
    const u64 hidden = 1ULL << (p - 1);

    Num x(a);
    u64 signBit = u64(x.sign) << (bits - 1);

    if (x.isNaN()) {

        if (a.isSignalingNaN()) raiseFlags(FloatFlags::INVALID);
        u64 payload = (a.mantissa() << 1) >> (64 - (p - 1));
        return signBit | fieldMax << (p - 1) | hidden >> 1 | payload;
    }
    if (x.isInf()) return signBit | fieldMax << (p - 1);
    if (x.isZero()) return signBit;

    i32 e = x.exp - bias;
    u128 t = makeU128(x.sig, 0);
    int drop = 128 - p;
    u128 unit = u128(1) << drop;
    u128 half = unit >> 1;

    // Round with an unbounded exponent
    u128 kept = t >> drop;
    u128 frac = t & (unit - 1);
    bool roundBit = (frac & half) != 0, rest = (frac & (half - 1)) != 0;
    kept += increment(mode, x.sign, roundBit, rest, (kept & 1) != 0);
    bool carry = (kept >> p) != 0;

    if (e >= emin) {

        if (carry) { kept >>= 1; e++; }

        if (e > emax) {

            raiseFlags(FloatFlags::OVFL | FloatFlags::INEXACT);
            bool towardsZero = mode == RoundingMode::ZERO ||
                               (x.sign && mode == RoundingMode::UP) || (!x.sign && mode == RoundingMode::DOWN);
            return towardsZero ? signBit | (fieldMax - 1) << (p - 1) | (hidden - 1)
                               : signBit | fieldMax << (p - 1);
        }
        if (frac != 0) raiseFlags(FloatFlags::INEXACT);
        return signBit | u64(e + emax) << (p - 1) | (u64(kept) & (hidden - 1));
    }

    // Tininess is detected after rounding
    bool tiny = e < emin - 1 || !carry;

    t = shiftRightJam128(t, emin - e);
    kept = t >> drop;
    frac = t & (unit - 1);
    roundBit = (frac & half) != 0;
    rest = (frac & (half - 1)) != 0;
    kept += increment(mode, x.sign, roundBit, rest, (kept & 1) != 0);

    if (frac != 0) {
        raiseFlags(FloatFlags::INEXACT);
        if (tiny) raiseFlags(FloatFlags::UNFL);
    }
    return signBit | u64(kept);
}

float FpuEnv::toFloat(const MFloat80 &a) { return std::bit_cast<float>(u32(toIeee(a, 32))); }
double FpuEnv::toDouble(const MFloat80 &a) { return std::bit_cast<double>(toIeee(a, 64)); }

}
