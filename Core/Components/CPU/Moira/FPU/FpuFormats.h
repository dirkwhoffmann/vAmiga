// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "FpuEnv.h"

namespace MOIRA_NAMESPACE {

/** The Motorola floating-point unit supports seven data formats:
 *
 *    Byte Integer (B)
 *    Word Integer (W)
 *    Long Word Integer (L)
 *    Single Precision Real (S)
 *    Double Precision Real (D)
 *    Extended Precision Real (X)
 *    Packed Decimal String Real (P)
 *
 * Internally, all values are represented as MFloat80. The functions in this
 * file convert between MFloat80 and the memory representations of the formats
 * listed above. They are pure functions of their arguments and the current
 * floating-point environment (FpuEnv): The rounding mode and precision are
 * read from it, and exceptions are reported by raising the corresponding
 * FloatFlags. Translating these flags into FPSR bits is up to the FPU.
 */
struct FpuFormats {

    /// Number of bytes of a single precision value in memory
    static constexpr int sizeOfSingle   = 4;
    /// Number of bytes of a double precision value in memory
    static constexpr int sizeOfDouble   = 8;
    /// Number of bytes of an extended precision value in memory
    static constexpr int sizeOfExtended = 12;
    /// Number of bytes of a packed decimal value in memory
    static constexpr int sizeOfPacked   = 12;


    //
    // Integers (B, W, L)
    //

    /// Integers convert to MFloat80 without loss of precision
    static MFloat80 fromInteger(i32 value) { return MFloat80::fromInt(value); }

    /** Converts to an integer of 'bits' width (8, 16, or 32). The value is
     * rounded according to the current rounding mode. If it doesn't fit, the
     * invalid flag is raised and the largest value of matching sign is
     * returned (NaNs return the largest positive value).
     */
    static i32 toInteger(FpuEnv &env, const MFloat80 &value, int bits);


    //
    // Single and double precision (S, D)
    //

    /// Converts a single precision value (exact; signaling NaNs raise the invalid flag)
    static MFloat80 fromSingle(FpuEnv &env, u32 raw);
    /// Converts a double precision value (exact; signaling NaNs raise the invalid flag)
    static MFloat80 fromDouble(FpuEnv &env, u64 raw);

    /// Converts to single precision (rounded according to the current rounding mode)
    static u32 toSingle(FpuEnv &env, const MFloat80 &value);
    /// Converts to double precision (rounded according to the current rounding mode)
    static u64 toDouble(FpuEnv &env, const MFloat80 &value);


    //
    // Extended precision (X)
    //

    /** An extended precision value occupies three long words in memory:
     *
     *   w[0]: Sign and exponent in bits 31..16, bits 15..0 are unused
     *   w[1]: Mantissa (high)
     *   w[2]: Mantissa (low)
     */
    static MFloat80 fromExtended(const u32 (&w)[3]);
    /** An extended precision value occupies three long words in memory:
     *
     *   w[0]: Sign and exponent in bits 31..16, bits 15..0 are unused
     *   w[1]: Mantissa (high)
     *   w[2]: Mantissa (low)
     */
    static void toExtended(const MFloat80 &value, u32 (&w)[3]);


    //
    // Packed decimal (P)
    //

    /** A packed decimal value occupies three long words in memory:
     *
     *   w[0]: Bit 31:      Sign of the mantissa (SM)
     *         Bit 30:      Sign of the exponent (SE)
     *         Bits 29..28: Always 0 for numbers, 11 for infinities and NaNs
     *         Bits 27..16: Exponent digits 2, 1, 0 (BCD)
     *         Bits 15..12: Exponent digit 3 (BCD)
     *         Bits 3..0:   Mantissa digit 0 (the integer digit)
     *   w[1]: Mantissa digits 1 to 8 (BCD)
     *   w[2]: Mantissa digits 9 to 16 (BCD)
     *
     * Infinities and NaNs are marked by an exponent field of 0xFFF.
     */
    static MFloat80 fromPacked(FpuEnv &env, const u32 (&w)[3]);

    /** Converts to packed decimal. The parameter 'k' is the k-factor. For
     * positive values, it is the number of significant digits (1...17). For
     * values less than or equal to zero, -k is the number of digits to the
     * right of the decimal point. A k-factor greater than 17 is clamped to 17
     * and raises the invalid flag. The mantissa is rounded according to the
     * current rounding mode. The inexact flag is raised if digits are lost.
     */
    static void toPacked(FpuEnv &env, const MFloat80 &value, int k, u32 (&w)[3]);
};

}
