// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MFloat80.h"

namespace MOIRA_NAMESPACE {

enum class RoundingMode : int {

    NEAREST = 0,    ////<< Round to nearest, ties to even
    ZERO    = 1,    ////<< Round toward zero
    DOWN    = 2,    ////<< Round toward negative infinity
    UP      = 3     ////<< Round toward positive infinity
};

/** The rounding precision limits the number of mantissa bits of a result. The
 * exponent range is always the one of the extended format.
 */
enum class Precision : int {

    SINGLE   = 32,  ////<< 24 mantissa bits
    DOUBLE   = 64,  ////<< 53 mantissa bits
    EXTENDED = 80   ////<< 64 mantissa bits
};

/// Exception flags. They are sticky: Once raised, a flag stays set until cleared.
struct FloatFlags {

    static constexpr u8 INEXACT     = 0x01;
    static constexpr u8 UNFL        = 0x02;   ////<< Underflow
    static constexpr u8 OVFL        = 0x04;   ////<< Overflow
    static constexpr u8 DIV_BY_ZERO = 0x08;
    static constexpr u8 INVALID     = 0x10;
    static constexpr u8 SIG_NAN        = 0x20;   ////<< Signaling NaN operand
    static constexpr u8 ALL         = 0x3F;
};

/** The floating-point environment of the Motorola FPUs
 *
 * The environment consists of the rounding mode, the rounding precision and the
 * accumulated exception flags. All operations on MFloat80 numbers are carried
 * out by an environment object. They are computed in software and produce
 * bit-exact results that are independent of the host.
 *
 * Operands are interpreted by value, which includes the non-canonical
 * encodings the hardware accepts (denormals, unnormals, pseudo-denormals). A
 * biased exponent of 0 is a regular exponent (the 68k FPUs don't treat it as 1
 * like the x87 does).
 */
class FpuEnv {

    RoundingMode mode = RoundingMode::NEAREST;
    Precision prec = Precision::EXTENDED;

    /* If the precision is SINGLE or DOUBLE, the Motorola FPUs limit the exponent
     * range to the one of the corresponding format. FSGLMUL and FSGLDIV only
     * reduce the mantissa and keep the extended exponent range.
     */
    bool limitExponent = true;

    u8 exceptions = 0;

public:

    //
    // Configuration
    //

    /// Returns the rounding mode (default: NEAREST)
    RoundingMode roundingMode() const { return mode; }
    /// Sets the rounding mode
    void setRoundingMode(RoundingMode value) { mode = value; }

    /// Returns the rounding precision (default: EXTENDED)
    Precision precision() const { return prec; }
    /// Sets the rounding precision
    void setPrecision(Precision value) { prec = value; }

    /// Indicates whether the exponent range of results is limited by the precision
    bool limitsExponent() const { return limitExponent; }
    /// Limits or unlimits the exponent range of results
    void setLimitsExponent(bool value) { limitExponent = value; }

    /// Returns the raised exception flags (see FloatFlags)
    u8 flags() const { return exceptions; }
    /// Clears exception flags
    void clearFlags(u8 mask = FloatFlags::ALL) { exceptions &= u8(~mask); }
    /// Raises exception flags
    void raiseFlags(u8 mask) { exceptions |= u8(mask & FloatFlags::ALL); }

    /** Saves the environment on construction and restores it on destruction.
     * The flags are not restored by default, so exceptions raised inside the
     * scope stay visible afterwards. Pass 'true' to discard them.
     */
    class Scope {

        FpuEnv &env;
        RoundingMode savedMode;
        Precision savedPrecision;
        bool savedLimits;
        u8 savedFlags;
        bool restoreFlags;

    public:

        explicit Scope(FpuEnv &env, bool restoreFlags = false);
        ~Scope();

        Scope(const Scope &) = delete;
        Scope &operator=(const Scope &) = delete;
    };


    //
    // Arithmetic
    //

    /// Rounds to the current precision and exponent range (FMOVE)
    MFloat80 round(const MFloat80 &a);

    MFloat80 add(const MFloat80 &a, const MFloat80 &b);
    MFloat80 sub(const MFloat80 &a, const MFloat80 &b);
    MFloat80 mul(const MFloat80 &a, const MFloat80 &b);
    MFloat80 div(const MFloat80 &a, const MFloat80 &b);
    MFloat80 sqrt(const MFloat80 &a);

    /// Raises the invalid flag and returns the default NaN of the 68k FPUs
    MFloat80 invalidOperation();

    /// Rounds to an integral value according to the current rounding mode
    MFloat80 roundToInt(const MFloat80 &a);

    /// Computes a * 2^n, where n is the integer part of b
    MFloat80 scale(const MFloat80 &a, const MFloat80 &b);

    /** Composes a number from an unrounded significand. The value is
     * sig:extra * 2^(exp - bias - 63), where 'exp' is the biased exponent that
     * applies if 'sig' is normalized (bit 63 set). A 'sig' that is not
     * normalized is normalized first. The bits in 'extra' lie below 'sig' and
     * are rounded away according to the environment. Raises the inexact,
     * underflow and overflow flags as needed.
     */
    MFloat80 roundPack(bool sign, i32 exp, u64 sig, u64 extra = 0);


    //
    // Comparing
    //

    /** The comparisons follow IEEE 754. A NaN is unordered, i.e., it compares
     * unequal to everything (including itself) and every ordering comparison
     * with it is false. Equality is quiet and raises the invalid flag only for
     * signaling NaNs. The ordering comparisons raise the invalid flag for all
     * NaNs. Note that +0 == -0.
     */
    bool eq(const MFloat80 &a, const MFloat80 &b);
    /** The comparisons follow IEEE 754. A NaN is unordered, i.e., it compares
     * unequal to everything (including itself) and every ordering comparison
     * with it is false. Equality is quiet and raises the invalid flag only for
     * signaling NaNs. The ordering comparisons raise the invalid flag for all
     * NaNs. Note that +0 == -0.
     */
    bool lt(const MFloat80 &a, const MFloat80 &b);
    /** The comparisons follow IEEE 754. A NaN is unordered, i.e., it compares
     * unequal to everything (including itself) and every ordering comparison
     * with it is false. Equality is quiet and raises the invalid flag only for
     * signaling NaNs. The ordering comparisons raise the invalid flag for all
     * NaNs. Note that +0 == -0.
     */
    bool le(const MFloat80 &a, const MFloat80 &b);


    //
    // Converting
    //

    /// Converts from single precision (always exact)
    MFloat80 fromFloat(float value);
    /// Converts from double precision (always exact)
    MFloat80 fromDouble(double value);

    /** Conversion to integers. The result is rounded according to the current
     * rounding mode (toInt) or truncated toward zero (toIntTruncated).
     * Out-of-range values and NaNs raise the invalid flag and yield the most
     * negative integer.
     */
    i32 toInt32(const MFloat80 &a);
    /** Conversion to integers. The result is rounded according to the current
     * rounding mode (toInt) or truncated toward zero (toIntTruncated).
     * Out-of-range values and NaNs raise the invalid flag and yield the most
     * negative integer.
     */
    i32 toInt32Truncated(const MFloat80 &a);
    /** Conversion to integers. The result is rounded according to the current
     * rounding mode (toInt) or truncated toward zero (toIntTruncated).
     * Out-of-range values and NaNs raise the invalid flag and yield the most
     * negative integer.
     */
    i64 toInt64(const MFloat80 &a);
    /** Conversion to integers. The result is rounded according to the current
     * rounding mode (toInt) or truncated toward zero (toIntTruncated).
     * Out-of-range values and NaNs raise the invalid flag and yield the most
     * negative integer.
     */
    i64 toInt64Truncated(const MFloat80 &a);

    /// Converts to single precision (rounded according to the current rounding mode)
    float toFloat(const MFloat80 &a);
    /// Converts to double precision (rounded according to the current rounding mode)
    double toDouble(const MFloat80 &a);


    //
    // Transcendental functions (implemented in FpuEnvMath.cpp)
    //

    /* The functions are evaluated with a 128-bit significand and rounded once
     * to the current precision and rounding mode. Hence, results are accurate
     * to far better than one unit in the last place (but, like on the real
     * hardware, not necessarily correctly rounded). Results that are
     * mathematically exact (e.g., 2^n, log2(2^n), sin(0)) are returned exactly.
     *
     * Special operands are handled according to the MC68881 user's manual:
     * Invalid operations (e.g., the logarithm of a negative number) raise the
     * invalid flag and return a quiet NaN, a division by zero (e.g., ln(0))
     * raises the corresponding flag and returns an infinity. NaN operands have
     * to be resolved by the caller.
     */

    /// Sine
    MFloat80 sin(const MFloat80 &x);
    /// Cosine
    MFloat80 cos(const MFloat80 &x);
    /// Tangent
    MFloat80 tan(const MFloat80 &x);
    /// Sine and cosine in one step
    void sincos(const MFloat80 &x, MFloat80 &sine, MFloat80 &cosine);

    /// Arc sine
    MFloat80 asin(const MFloat80 &x);
    /// Arc cosine
    MFloat80 acos(const MFloat80 &x);
    /// Arc tangent
    MFloat80 atan(const MFloat80 &x);

    /// Hyperbolic sine
    MFloat80 sinh(const MFloat80 &x);
    /// Hyperbolic cosine
    MFloat80 cosh(const MFloat80 &x);
    /// Hyperbolic tangent
    MFloat80 tanh(const MFloat80 &x);
    /// Hyperbolic arc tangent
    MFloat80 atanh(const MFloat80 &x);

    /// Exponential functions
    MFloat80 etox(const MFloat80 &x);         // e^x
    MFloat80 etoxm1(const MFloat80 &x);       ///< e^x - 1
    MFloat80 twotox(const MFloat80 &x);       ///< 2^x
    MFloat80 tentox(const MFloat80 &x);       ///< 10^x

    /// Logarithms
    MFloat80 logn(const MFloat80 &x);         // ln(x)
    MFloat80 lognp1(const MFloat80 &x);       ///< ln(x + 1)
    MFloat80 log2(const MFloat80 &x);
    MFloat80 log10(const MFloat80 &x);

private:

    MFloat80 propagateNaN(const MFloat80 &a, const MFloat80 &b);
    MFloat80 addSub(const MFloat80 &a, const MFloat80 &b, bool negate);

    i64 toInteger(const MFloat80 &a, RoundingMode mode, int bits);
    u64 toIeee(const MFloat80 &a, int bits);
};

}
