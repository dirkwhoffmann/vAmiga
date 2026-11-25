// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "../MoiraTypes.h"
#include <string>

namespace MOIRA_NAMESPACE {

/** MFloat80 is the 80-bit extended precision format of the Motorola FPUs:
 *
 *   Bit 79      : Sign bit
 *   Bits 64..78 : Biased exponent (bias 0x3FFF)
 *   Bits 0..63  : Mantissa, including an explicit integer bit (bit 63)
 *
 * The class is a plain value type. It stores the raw bits of the number and
 * makes no attempt to normalize them. In particular, it can represent the
 * non-canonical encodings (unnormals, pseudo-denormals) that the hardware
 * accepts as operands.
 *
 * MFloat80 carries no floating-point environment. Arithmetic, comparisons and
 * conversions that depend on the rounding mode or raise exceptions are provided
 * by FpuEnv.
 */
class MFloat80 {

    // Sign and biased exponent
    u16 se = 0;

    // Mantissa (including the integer bit)
    u64 man = 0;

public:

    /// Exponent bias
    static constexpr i32 bias = 0x3FFF;


    //
    // Creating numbers
    //

    /// Creates +0
    constexpr MFloat80() = default;

    /// Creates a number from its raw bits
    constexpr MFloat80(u16 signExp, u64 mantissa) : se(signExp), man(mantissa) { }
    /// Creates a number from its raw bits
    static constexpr MFloat80 fromBits(u16 signExp, u64 mantissa) { return { signExp, mantissa }; }

    /// Converts from a 64-bit integer (always exact)
    static MFloat80 fromInt(i64 value);

    /// Converts from 'long double' (exact if the host's long double is an 80-bit extended type)
    static MFloat80 fromLongDouble(long double value);

    /// Returns +0 or -0
    static constexpr MFloat80 zero(bool negative = false) { return { u16(negative ? 0x8000 : 0), 0 }; }
    /// Returns +1 or -1
    static constexpr MFloat80 one(bool negative = false) { return { u16(negative ? 0xBFFF : 0x3FFF), 1ULL << 63 }; }
    /// Returns +infinity or -infinity
    static constexpr MFloat80 infinity(bool negative = false) { return { u16(negative ? 0xFFFF : 0x7FFF), 0 }; }
    /// Returns a quiet NaN
    static constexpr MFloat80 quietNaN(bool negative = false) { return { u16(negative ? 0xFFFF : 0x7FFF), 0xC000000000000000ULL }; }
    /// Returns the result of an invalid operation (positive, all mantissa bits set)
    static constexpr MFloat80 defaultNaN() { return { 0x7FFF, 0xFFFFFFFFFFFFFFFFULL }; }
    /// Returns a signaling NaN
    static constexpr MFloat80 signalingNaN(bool negative = false) { return { u16(negative ? 0xFFFF : 0x7FFF), 0xA000000000000000ULL }; }


    //
    // Accessing raw bits
    //

    constexpr u16 signExp() const { return se; }
    constexpr u64 mantissa() const { return man; }

    constexpr bool sign() const { return se & 0x8000; }
    constexpr u16 biasedExponent() const { return se & 0x7FFF; }
    constexpr i32 exponent() const { return i32(biasedExponent()) - bias; }
    constexpr bool integerBit() const { return man >> 63; }


    //
    // Classifying numbers
    //

    constexpr bool isNegative() const { return sign(); }
    constexpr bool isZero() const { return biasedExponent() == 0 && man == 0; }
    constexpr bool isInfinite() const { return biasedExponent() == 0x7FFF && (man << 1) == 0; }
    constexpr bool isNaN() const { return biasedExponent() == 0x7FFF && (man << 1) != 0; }
    constexpr bool isSignalingNaN() const { return isNaN() && !(man & (1ULL << 62)); }
    constexpr bool isQuietNaN() const { return isNaN() && (man & (1ULL << 62)); }
    constexpr bool isFinite() const { return biasedExponent() != 0x7FFF; }

    /// A number with a minimal exponent and a cleared integer bit
    constexpr bool isDenormal() const { return biasedExponent() == 0 && man != 0 && !integerBit(); }

    /// A number with a regular exponent and a set integer bit
    constexpr bool isNormal() const { return biasedExponent() != 0 && biasedExponent() != 0x7FFF && integerBit(); }

    /// Checks whether at least one operand is a NaN
    constexpr bool unordered(const MFloat80 &rhs) const { return isNaN() || rhs.isNaN(); }

    /// Checks for identical bit patterns (0 and -0 differ)
    constexpr bool identical(const MFloat80 &rhs) const { return se == rhs.se && man == rhs.man; }


    //
    // Operations that are exact and raise no exceptions
    //

    constexpr MFloat80 operator-() const { return { u16(se ^ 0x8000), man }; }
    constexpr MFloat80 abs() const { return { u16(se & 0x7FFF), man }; }


    //
    // Converting
    //

    /// Converts to 'long double' (exact if the host's long double is an 80-bit extended type)
    long double toLongDouble() const;

    /// Returns a string of the form "0x3FFF:8000000000000000"
    std::string str() const;
};

}
