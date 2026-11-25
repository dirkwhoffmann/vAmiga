// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "../MoiraTypes.h"
#include <bit>
#include <cmath>
#include <concepts>

namespace MOIRA_NAMESPACE {

/* Helpers for 128-bit integer arithmetic. They are shared by the transcendental
 * functions (FpuEnvMath.cpp) and the packed decimal conversion (FpuFormats).
 */
#if defined(__SIZEOF_INT128__) && !defined(MOIRA_SOFT_U128)

using u128 = unsigned __int128;

#else

/* Portable replacement for compilers without a native 128-bit type (MSVC,
 * 32-bit targets). It provides the operators needed by the FPU code. Define
 * MOIRA_SOFT_U128 to use it on all compilers (e.g., for testing).
 */
struct u128 {

    u64 lo = 0;
    u64 hi = 0;

    constexpr u128() = default;
    constexpr u128(u64 h, u64 l) : lo(l), hi(h) { }
    template <std::unsigned_integral T> constexpr u128(T v) : lo(u64(v)), hi(0) { }
    template <std::signed_integral T> constexpr u128(T v) : lo(u64(i64(v))), hi(v < 0 ? ~u64(0) : 0) { }

    explicit constexpr operator u64() const { return lo; }
    explicit constexpr operator u32() const { return u32(lo); }
    explicit constexpr operator i64() const { return i64(lo); }
    explicit constexpr operator i32() const { return i32(lo); }
    explicit constexpr operator bool() const { return lo != 0 || hi != 0; }

    // 64 x 64 -> 128 bit multiplication
    static u128 mul64(u64 a, u64 b)
    {
        u64 a0 = a & 0xFFFFFFFF, a1 = a >> 32, b0 = b & 0xFFFFFFFF, b1 = b >> 32;
        u64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
        u64 mid = (p00 >> 32) + (p01 & 0xFFFFFFFF) + (p10 & 0xFFFFFFFF);
        return u128(p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32), (mid << 32) | (p00 & 0xFFFFFFFF));
    }

    friend constexpr u128 operator~(u128 a) { return u128(~a.hi, ~a.lo); }
    friend constexpr u128 operator&(u128 a, u128 b) { return u128(a.hi & b.hi, a.lo & b.lo); }
    friend constexpr u128 operator|(u128 a, u128 b) { return u128(a.hi | b.hi, a.lo | b.lo); }
    friend constexpr u128 operator^(u128 a, u128 b) { return u128(a.hi ^ b.hi, a.lo ^ b.lo); }

    friend constexpr u128 operator<<(u128 a, int n)
    {
        if (n <= 0) return a;
        if (n >= 128) return u128();
        if (n >= 64) return u128(a.lo << (n - 64), 0);
        return u128((a.hi << n) | (a.lo >> (64 - n)), a.lo << n);
    }
    friend constexpr u128 operator>>(u128 a, int n)
    {
        if (n <= 0) return a;
        if (n >= 128) return u128();
        if (n >= 64) return u128(0, a.hi >> (n - 64));
        return u128(a.hi >> n, (a.lo >> n) | (a.hi << (64 - n)));
    }

    friend constexpr u128 operator+(u128 a, u128 b)
    {
        u64 l = a.lo + b.lo;
        return u128(a.hi + b.hi + (l < a.lo ? 1 : 0), l);
    }
    friend constexpr u128 operator-(u128 a, u128 b)
    {
        return u128(a.hi - b.hi - (a.lo < b.lo ? 1 : 0), a.lo - b.lo);
    }
    friend u128 operator*(u128 a, u128 b)
    {
        u128 r = mul64(a.lo, b.lo);
        r.hi += a.lo * b.hi + a.hi * b.lo;
        return r;
    }

    // Binary long division (quotient and remainder)
    static void divmod(u128 n, u128 d, u128 &q, u128 &r)
    {
        if (n.hi == 0 && d.hi == 0) { q = u128(n.lo / d.lo); r = u128(n.lo % d.lo); return; }
        q = u128(); r = u128();
        for (int i = 127; i >= 0; i--) {
            r = (r << 1) | u128((n >> i).lo & 1);
            if (r >= d) { r = r - d; q = q | (u128(1) << i); }
        }
    }
    friend u128 operator/(u128 a, u128 b) { u128 q, r; divmod(a, b, q, r); return q; }
    friend u128 operator%(u128 a, u128 b) { u128 q, r; divmod(a, b, q, r); return r; }

    friend constexpr bool operator==(u128 a, u128 b) { return a.hi == b.hi && a.lo == b.lo; }
    friend constexpr bool operator!=(u128 a, u128 b) { return !(a == b); }
    friend constexpr bool operator<(u128 a, u128 b) { return a.hi != b.hi ? a.hi < b.hi : a.lo < b.lo; }
    friend constexpr bool operator>(u128 a, u128 b) { return b < a; }
    friend constexpr bool operator<=(u128 a, u128 b) { return !(b < a); }
    friend constexpr bool operator>=(u128 a, u128 b) { return !(a < b); }

    u128 &operator+=(u128 b) { return *this = *this + b; }
    u128 &operator-=(u128 b) { return *this = *this - b; }
    u128 &operator*=(u128 b) { return *this = *this * b; }
    u128 &operator/=(u128 b) { return *this = *this / b; }
    u128 &operator%=(u128 b) { return *this = *this % b; }
    u128 &operator&=(u128 b) { return *this = *this & b; }
    u128 &operator|=(u128 b) { return *this = *this | b; }
    u128 &operator^=(u128 b) { return *this = *this ^ b; }
    u128 &operator<<=(int n) { return *this = *this << n; }
    u128 &operator>>=(int n) { return *this = *this >> n; }
    u128 &operator++() { return *this = *this + 1; }
    u128 &operator--() { return *this = *this - 1; }
};

#endif

inline u128 makeU128(u64 hi, u64 lo) { return (u128(hi) << 64) | u128(lo); }
inline u64 hi64(u128 x) { return u64(x >> 64); }
inline u64 lo64(u128 x) { return u64(x); }

// Counts the leading zero bits (the result is 128 for x = 0)
inline int clz128(u128 x)
{
    return hi64(x) ? std::countl_zero(hi64(x)) : 64 + std::countl_zero(lo64(x));
}

// Multiplies two 128-bit values. The 256-bit product is returned as hi:lo.
inline void mul128To256(u128 a, u128 b, u128 &hi, u128 &lo)
{
    u128 p00 = u128(lo64(a)) * lo64(b), p01 = u128(lo64(a)) * hi64(b);
    u128 p10 = u128(hi64(a)) * lo64(b), p11 = u128(hi64(a)) * hi64(b);

    u128 mid = (p00 >> 64) + lo64(p01) + lo64(p10);
    lo = makeU128(lo64(mid), lo64(p00));
    hi = p11 + (p01 >> 64) + (p10 >> 64) + (mid >> 64);
}

// Returns the upper 128 bits of the 256-bit product
inline u128 mulHigh128(u128 a, u128 b)
{
    u128 hi, lo;
    mul128To256(a, b, hi, lo);
    return hi;
}

// Shifts right and sets the least significant bit if any one-bit was shifted out
inline u128 shiftRightJam128(u128 a, int count)
{
    if (count <= 0) return a;
    if (count >= 128) return a != 0;
    return (a >> count) | ((a << (128 - count)) != 0);
}

// Converts to double (used for error estimates)
inline double toDouble(u128 x) { return std::ldexp(double(hi64(x)), 64) + double(lo64(x)); }

/* Helpers for measuring the distance of two extended precision values in ulps.
 * A finite value is mapped to a signed integer such that adjacent values differ
 * by 1. The position exceeds 64 bits, hence it is stored as magnitude and sign.
 */
inline u128 ulpPosition(u16 biasedExp, u64 man)
{
    u128 e = biasedExp & 0x7FFF;
    return (e << 63) + (e ? u128(man & ~(1ULL << 63)) : u128(man >> 1));
}

inline u128 ulpDistance(bool negA, u128 posA, bool negB, u128 posB)
{
    if (negA == negB) return posA > posB ? posA - posB : posB - posA;
    return posA + posB;
}

}
