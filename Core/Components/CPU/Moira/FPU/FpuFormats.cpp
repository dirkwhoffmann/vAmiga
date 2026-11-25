// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "FpuFormats.h"
#include "Wide.h"
#include <algorithm>
#include <bit>
#include <vector>

namespace MOIRA_NAMESPACE {

namespace {

int bcd(u32 w, int shift) { return int((w >> shift) & 0xF); }

}


//
// Integers
//

i32
FpuFormats::toInteger(FpuEnv &env, const MFloat80 &value, int bits)
{
    i64 hi = (1LL << (bits - 1)) - 1;
    i64 lo = -hi - 1;

    if (value.isNaN()) {

        // The result consists of the most significant bits of the (quieted) mantissa
        u64 man = value.mantissa();
        if (value.isSignalingNaN()) { env.raiseFlags(FloatFlags::SIG_NAN); man |= 1ULL << 62; }
        env.raiseFlags(FloatFlags::INVALID);
        return (i32)(u32)(man >> (64 - bits));
    }
    if (value.isInfinite()) {

        env.raiseFlags(FloatFlags::INVALID);
        return (i32)(value.sign() ? lo : hi);
    }

    auto saved = env.flags();
    auto rounded = env.roundToInt(value);

    if (rounded.exponent() < 63) {

        auto i = env.toInt64Truncated(rounded);
        if (i >= lo && i <= hi) return (i32)i;
    }

    // An overflow is reported as an operand error without the inexact exception
    env.clearFlags();
    env.raiseFlags(saved);
    env.raiseFlags(FloatFlags::INVALID);
    return (i32)(value.sign() ? lo : hi);
}


//
// Single and double precision
//

MFloat80
FpuFormats::fromSingle(FpuEnv &env, u32 raw)
{
    // NaNs are converted bit by bit to keep their payload and their signaling state
    if ((raw & 0x7F800000) == 0x7F800000 && (raw & 0x007FFFFF)) {
        return MFloat80::fromBits(u16((raw >> 16) & 0x8000) | 0x7FFF,
                                 u64(raw & 0x007FFFFF) << 40);
    }

    // Denormals keep their unnormalized significand and the smallest normal exponent
    if ((raw & 0x7F800000) == 0 && (raw & 0x007FFFFF)) {
        return MFloat80::fromBits(u16((raw >> 16) & 0x8000) | 0x3F81, u64(raw & 0x007FFFFF) << 40);
    }
    return env.fromFloat(std::bit_cast<float>(raw));
}

MFloat80
FpuFormats::fromDouble(FpuEnv &env, u64 raw)
{
    if ((raw & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL && (raw & 0x000FFFFFFFFFFFFFULL)) {
        return MFloat80::fromBits(u16((raw >> 48) & 0x8000) | 0x7FFF,
                                 (raw & 0x000FFFFFFFFFFFFFULL) << 11);
    }

    if ((raw & 0x7FF0000000000000ULL) == 0 && (raw & 0x000FFFFFFFFFFFFFULL)) {
        return MFloat80::fromBits(u16((raw >> 48) & 0x8000) | 0x3C01, (raw & 0x000FFFFFFFFFFFFFULL) << 11);
    }
    return env.fromDouble(std::bit_cast<double>(raw));
}

u32
FpuFormats::toSingle(FpuEnv &env, const MFloat80 &value)
{
    if (value.isNaN()) {

        // Truncate the payload, but make sure the result remains a NaN
        u32 frac = u32((value.mantissa() >> 40) & 0x007FFFFF);
        if (value.isSignalingNaN()) { env.raiseFlags(FloatFlags::SIG_NAN); frac |= 0x00400000; }
        if (frac == 0) frac = 0x00400000;
        return u32(value.sign()) << 31 | 0x7F800000 | frac;
    }
    return std::bit_cast<u32>(env.toFloat(value));
}

u64
FpuFormats::toDouble(FpuEnv &env, const MFloat80 &value)
{
    if (value.isNaN()) {

        u64 frac = (value.mantissa() >> 11) & 0x000FFFFFFFFFFFFFULL;
        if (value.isSignalingNaN()) { env.raiseFlags(FloatFlags::SIG_NAN); frac |= 0x0008000000000000ULL; }
        if (frac == 0) frac = 0x0008000000000000ULL;
        return u64(value.sign()) << 63 | 0x7FF0000000000000ULL | frac;
    }
    return std::bit_cast<u64>(env.toDouble(value));
}


//
// Extended precision
//

MFloat80
FpuFormats::fromExtended(const u32 (&w)[3])
{
    return MFloat80::fromBits(u16(w[0] >> 16), u64(w[1]) << 32 | w[2]);
}

void
FpuFormats::toExtended(const MFloat80 &value, u32 (&w)[3])
{
    w[0] = u32(value.signExp()) << 16;
    w[1] = u32(value.mantissa() >> 32);
    w[2] = u32(value.mantissa());
}


namespace {

/* Packed-decimal conversion needs more precision than an extended-precision
 * value offers, because a power of ten has to be applied to a 64-bit mantissa.
 * The conversion therefore works on binary numbers with a 128-bit significand
 * (BinFloat). The significand is normalized, i.e., the value of a BinFloat is
 *
 *     sig / 2^127 * 2^(exp - 0x3FFF)
 *
 * Multiplication and division truncate to 128 bits and merge all discarded bits
 * into the least significant bit (sticky bit). A power of ten is built by
 * repeated squaring in which intermediate results are rounded to 64 bits, which
 * is the source of the (reproducible) deviations from a correctly rounded
 * conversion in rare cases.
 */

struct BinFloat { i32 exp; u128 sig; };

BinFloat mul(const BinFloat &a, const BinFloat &b)
{
    u128 hi, lo;
    mul128To256(a.sig, b.sig, hi, lo);

    BinFloat z { a.exp + b.exp - 0x3FFE, hi | (lo != 0) };
    if (!(z.sig >> 127)) { z.sig <<= 1; z.exp--; }
    return z;
}

BinFloat div(BinFloat a, const BinFloat &b)
{
    // Make sure that the quotient is less than one
    i32 exp = a.exp - b.exp + 0x3FFE;
    if (a.sig >= b.sig) { a.sig >>= 1; exp++; }

    // Compute the 128 fraction bits of a / b, one bit at a time
    u128 q = 0, r = a.sig;
    for (int i = 0; i < 128; i++) {
        bool carry = (r >> 127) != 0;
        r <<= 1;
        q <<= 1;
        if (carry || r >= b.sig) { r -= b.sig; q |= 1; }
    }
    return { exp, q | (r != 0) };
}

// Tells whether discarding the fraction 'frac' increments the magnitude
bool incrementsMagnitude(bool negative, RoundingMode mode, u64 frac)
{
    switch (mode) {
        case RoundingMode::NEAREST: return (frac >> 63) != 0;
        case RoundingMode::DOWN:    return negative && frac;
        case RoundingMode::UP:      return !negative && frac;
        default:                    return false;
    }
}

// Rounds the 64.64 fixed-point number 'ip.frac' to an integer
u64 roundInteger(bool negative, RoundingMode mode, u64 ip, u64 frac)
{
    if (!incrementsMagnitude(negative, mode, frac)) return ip;

    ip++;
    if (mode == RoundingMode::NEAREST && frac == 1ULL << 63) ip &= ~1ULL;
    return ip;
}

// Rounds the significand to 64 bits (the lower half is cleared)
BinFloat round64(bool negative, BinFloat a, RoundingMode mode)
{
    u64 ip = hi64(a.sig);
    u64 r = roundInteger(negative, mode, ip, lo64(a.sig));

    if (r == 0) {
        if (ip) { a.exp++; r = 1ULL << 63; }    // Carry out of the significand
        else    { a.exp = 0; }                  // Zero
    }
    a.sig = makeU128(r, 0);
    return a;
}

// Rounds the scaled value to an integer
u64 roundMagnitude(bool negative, const BinFloat &z, RoundingMode mode, bool &inexact)
{
    u128 fixed = shiftRightJam128(z.sig, 0x403E - z.exp);

    inexact = lo64(fixed) != 0;
    return roundInteger(negative, mode, hi64(fixed), lo64(fixed));
}

/* Selects the rounding mode for a power of ten. The factor is rounded such
 * that the final result moves in the direction of the requested mode. If the
 * factor is a divisor (inverse), the direction is reversed.
 */
RoundingMode factorMode(RoundingMode mode, bool negative, bool inverse)
{
    switch (mode) {
        case RoundingMode::DOWN: return negative != inverse ? RoundingMode::UP : RoundingMode::DOWN;
        case RoundingMode::UP:   return negative != inverse ? RoundingMode::DOWN : RoundingMode::UP;
        case RoundingMode::ZERO: return inverse ? RoundingMode::UP : RoundingMode::DOWN;
        default:                 return mode;
    }
}

// Computes 10^n
BinFloat powerOfTen(int n, RoundingMode mode)
{
    BinFloat result { 0x3FFF, makeU128(1ULL << 63, 0) };
    BinFloat square { 0x4002, makeU128(0xA000000000000000ULL, 0) };

    for (; n; n >>= 1) {
        if (n & 1) result = round64(false, mul(result, round64(false, square, mode)), mode);
        square = mul(square, square);
    }
    return result;
}

// Computes 10^n as an integer
u64 integerPowerOfTen(int n)
{
    u64 result = 1;
    while (n-- > 0) result *= 10;
    return result;
}

/* Estimates the decimal exponent of a value. The binary logarithm is
 * approximated by the sum of the binary exponent and the fraction of the
 * significand. The estimate can be off by one in either direction.
 */
i32 estimateDecimalExponent(i32 exp, u64 sig)
{
    if (sig == 0 || exp == 0x3FFF) return 0;
    if (exp < 0) return -4932;

    // Compute |e + f| as a number with 64 fractional bits
    i32 e = exp - 0x3FFF;
    u128 f = u128(sig ^ 1ULL << 63) << 1;
    u128 v = u128(e < 0 ? -e : e) << 64;
    v = e < 0 ? v - f : v + f;

    // Multiply by log10(2) (as a 0.128 fixed-point number) and round to an integer
    u128 hi, lo;
    mul128To256(v, makeU128(0x4D104D427DE7FBCCULL, 0x47C4ACD605BE48BCULL), hi, lo);
    u64 n = hi64(hi) + (lo64(hi) >> 63);

    return e < 0 ? -i32(n) : i32(n);
}

// Number of digits to generate for a given k-factor
int digitCount(int &k, i32 &exp10, FpuEnv &env)
{
    // A positive k-factor requests k significant digits
    if (k > 0) {
        if (k > 17) { k = 17; env.raiseFlags(FloatFlags::INVALID); }
        return k;
    }

    // Otherwise, -k digits are requested right of the decimal point
    int len = std::clamp(exp10 + 1 - k, 1, 17);
    if (k > exp10) exp10 = k;
    return len;
}

}

//
// Packed decimal
//

MFloat80
FpuFormats::fromPacked(FpuEnv &env, const u32 (&w)[3])
{
    // The conversion is carried out in extended precision. A reduced precision
    // is applied by the instruction that uses the value.
    FpuEnv::Scope scope(env);
    env.setPrecision(Precision::EXTENDED);

    bool negative = w[0] >> 31;

    // Infinities and NaNs
    if (((w[0] >> 16) & 0x7FFF) == 0x7FFF) {

        u64 man = u64(w[1]) << 32 | w[2];
        return MFloat80::fromBits(negative ? 0xFFFF : 0x7FFF, man);
    }

    // Zero (the exponent is irrelevant if the mantissa is zero)
    if (!(w[0] & 0xF) && !w[1] && !w[2]) return MFloat80::zero(negative);

    // Read the 17 mantissa digits as an integer
    u64 mant = w[0] & 0xF;
    for (int i = 28; i >= 0; i -= 4) mant = mant * 10 + ((w[1] >> i) & 0xF);
    for (int i = 28; i >= 0; i -= 4) mant = mant * 10 + ((w[2] >> i) & 0xF);

    // Read the three exponent digits. The integer mantissa is 10^16 times
    // larger than the decimal mantissa, which is compensated in the exponent.
    i32 exp10 = bcd(w[0], 24) * 100 + bcd(w[0], 20) * 10 + bcd(w[0], 16);
    if ((w[0] >> 30) & 1) exp10 = -exp10;
    exp10 -= 16;

    // Scale the mantissa
    RoundingMode mode = env.roundingMode();
    bool inverse = exp10 < 0;
    BinFloat factor = powerOfTen(inverse ? -exp10 : exp10, factorMode(mode, negative, inverse));

    int shift = std::countl_zero(mant);
    BinFloat z { 0x403E - shift, makeU128(mant << shift, 0) };
    z = inverse ? div(z, factor) : mul(z, factor);

    if (lo64(z.sig)) env.raiseFlags(FloatFlags::INEXACT);
    z = round64(negative, z, mode);

    return MFloat80::fromBits(u16(negative << 15 | z.exp), hi64(z.sig));
}

void
FpuFormats::toPacked(FpuEnv &env, const MFloat80 &value, int k, u32 (&w)[3])
{
    w[0] = w[1] = w[2] = 0;
    u32 sign = value.sign() ? 0x80000000 : 0;

    // Infinities and NaNs
    if (!value.isFinite()) {

        w[0] = sign | 0x7FFF0000;
        if (value.isNaN()) {

            // Signaling NaNs are converted to quiet NaNs
            u64 man = value.mantissa();
            if (value.isSignalingNaN()) { env.raiseFlags(FloatFlags::SIG_NAN); man |= 1ULL << 62; }
            w[1] = u32(man >> 32);
            w[2] = u32(man);
        }
        return;
    }

    // Zero. A k-factor above +17 is an operand error for a zero too
    // (MC68881UM, Table 6-2: "k-Factor > +17" for FMOVE to P). UAE omits it.
    if (value.isZero()) {
        if (k > 17 && compat != Compatibility::UAE) env.raiseFlags(FloatFlags::INVALID);
        w[0] = sign;
        return;
    }

    u64 sig = value.mantissa();
    i32 exp = (i32)value.biasedExponent();
    bool negative = value.sign();
    RoundingMode mode = env.roundingMode();

    // Normalize denormals. The exponent of a denormal is regarded as 0 (not 1).
    if (exp == 0) {
        int shift = std::countl_zero(sig);
        exp = -shift;
        sig <<= shift;
    }

    /* The value is scaled by a power of ten such that the requested digits
     * form an integer. Because the exponent is only estimated, the result
     * might have too many or too few digits. In this case, the estimate is
     * corrected and the scaling is repeated once.
     */
    i32 exp10 = estimateDecimalExponent(exp, sig);
    int len;
    u64 digits;

    for (int pass = 0; ; pass++) {

        len = digitCount(k, exp10, env);

        i32 scale = exp10 + 1 - len;
        bool up = scale < 0;
        BinFloat factor = powerOfTen(up ? -scale : scale, factorMode(mode, up, false));
        BinFloat z { exp, makeU128(sig, 0) };
        z = up ? mul(z, factor) : div(z, factor);

        bool inexact;
        digits = roundMagnitude(negative, z, mode, inexact);
        if (inexact) env.raiseFlags(FloatFlags::INEXACT);

        u64 smallest = integerPowerOfTen(len - 1);
        if (pass == 1 || (digits >= smallest && digits <= smallest * 10)) break;

        exp10 += digits < smallest ? -1 : 1;
    }

    // Assemble the exponent (four digits)
    u32 e = u32(exp10 < 0 ? -exp10 : exp10);
    if (e > 999) env.raiseFlags(FloatFlags::INVALID);

    u32 expWord = (e / 100 % 10) << 24 | (e / 10 % 10) << 20 | (e % 10) << 16 | (e / 1000) << 12;

    // Assemble the mantissa (the first digit goes into w[0], the rest into w[1] and w[2])
    u64 fraction = 0;
    for (int i = len - 1; i > 0; i--) {
        fraction |= (digits % 10) << (64 - i * 4);
        digits /= 10;
    }

    w[0] = sign | (exp10 < 0 ? 0x40000000 : 0) | expWord | u32(digits % 10);
    w[1] = u32(fraction >> 32);
    w[2] = u32(fraction);
}

}
