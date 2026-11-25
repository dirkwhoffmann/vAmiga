// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "../MoiraConfig.h"
#include "FPU.h"
#include "Wide.h"
#include <bit>

namespace MOIRA_NAMESPACE {

void
FPU::reset()
{
    // After reset, all data registers contain a non-signaling NaN
    for (auto &reg : fpr) reg = MFloat80::quietNaN();

    fpiar = 0;
    fpsr = 0;
    fpcr = 0;

    resetState = true;
    pendingTrap = 0;
}

u16
FPU::trapVector() const
{
    /* The exceptions are prioritized as follows (MC68881/MC68882UM, 6.1):
     * BSUN (48), SNAN (54), OPERR (52), OVFL (53), UNFL (51), DZ (50), INEX1 and INEX2 (49)
     */
    static constexpr u16 vectors[8] = { 49, 49, 50, 51, 53, 52, 54, 48 };

    u32 bits = (fpsr & fpcr & FPSR::EXC_MASK) >> 8;
    for (int i = 7; i >= 0; i--) if (bits & (1 << i)) return vectors[i];
    return 0;
}

bool
FPU::isSupported(Instr I) const
{
    switch (I) {

        // Instructions that are only available on the 6888x. The 68040 traps them.
        case Instr::FACOS:     case Instr::FASIN:     case Instr::FATANH:    case Instr::FCOS:
        case Instr::FCOSH:     case Instr::FETOX:     case Instr::FETOXM1:   case Instr::FGETEXP:
        case Instr::FGETMAN:   case Instr::FLOG10:    case Instr::FLOG2:     case Instr::FLOGN:
        case Instr::FLOGNP1:   case Instr::FMOD:      case Instr::FREM:      case Instr::FSCAL:
        case Instr::FSIN:      case Instr::FSINCOS:   case Instr::FSINH:     case Instr::FTAN:
        case Instr::FTANH:     case Instr::FTENTOX:   case Instr::FTWOTOX:   case Instr::FATAN:
        case Instr::FINT:      case Instr::FINTRZ:    case Instr::FMOVECR:
            return model != FPUModel::M68040;

        // Instructions that are only available on the 68040
        case Instr::FSABS:     case Instr::FDABS:     case Instr::FSADD:     case Instr::FDADD:
        case Instr::FSDIV:     case Instr::FDDIV:     case Instr::FSMOVE:    case Instr::FDMOVE:
        case Instr::FSMUL:     case Instr::FDMUL:     case Instr::FSNEG:     case Instr::FDNEG:
        case Instr::FSSQRT:    case Instr::FDSQRT:    case Instr::FSSUB:     case Instr::FDSUB:
            return model == FPUModel::M68040;

        default:
            return true;
    }
}

Precision
FPU::getPrecision() const
{
    switch (fpcr & FPCR::PREC_MASK) {

        case FPCR::PREC_EXTENDED:   return Precision::EXTENDED;
        case FPCR::PREC_SINGLE:     return Precision::SINGLE;
        default:                    return Precision::DOUBLE;
    }
}

Precision
FPU::getPrecision(Instr I) const
{
    switch (I) {

        case Instr::FSABS: case Instr::FSADD: case Instr::FSDIV: case Instr::FSMUL:
        case Instr::FSNEG: case Instr::FSSQRT: case Instr::FSSUB:

            return Precision::SINGLE;

        case Instr::FDABS: case Instr::FDADD: case Instr::FDDIV: case Instr::FDMUL:
        case Instr::FDNEG: case Instr::FDSQRT: case Instr::FDSUB:

            return Precision::DOUBLE;

        default:
            return getPrecision();
    }
}

RoundingMode
FPU::getRoundingMode() const
{
    switch (fpcr & FPCR::RND_MASK) {

        case FPCR::RND_NEAREST:     return RoundingMode::NEAREST;
        case FPCR::RND_ZERO:        return RoundingMode::ZERO;
        case FPCR::RND_DOWN:        return RoundingMode::DOWN;
        default:                    return RoundingMode::UP;
    }
}

void
FPU::setExcStatusBit(u32 mask)
{
    fpsr |= mask & FPSR::EXC_MASK;

    // Update the accrued exception byte (sticky bits)
    if (fpsr & (FPEXP::BSUN | FPEXP::SIG_NAN | FPEXP::OPERR))      fpsr |= 1 << 7;
    if (fpsr & FPEXP::OVFL)                                     fpsr |= 1 << 6;
    if ((fpsr & FPEXP::UNFL) && (fpsr & FPEXP::INEX2))          fpsr |= 1 << 5;
    if (fpsr & FPEXP::DZ)                                       fpsr |= 1 << 4;
    if (fpsr & (FPEXP::INEX1 | FPEXP::INEX2 | FPEXP::OVFL))     fpsr |= 1 << 3;
}

void
FPU::setConditionCodes(const MFloat80 &value)
{
    fpsr &= ~FPSR::CC_MASK;

    if (value.isNegative()) fpsr |= FPSR::CC_N;
    if (value.isZero())     fpsr |= FPSR::CC_Z;
    if (value.isInfinite()) fpsr |= FPSR::CC_I;
    if (value.isNaN())      fpsr |= FPSR::CC_NAN;
}

bool
FPU::test(unsigned condition)
{
    condition &= 0x1F;

    bool n = fpsr & FPSR::CC_N;
    bool z = fpsr & FPSR::CC_Z;
    bool nan = fpsr & FPSR::CC_NAN;

    // The signaling variants raise BSUN if the operands were unordered. Only BSUN
    // and the accrued IOP bit are set. No other bit is affected (MC68881/MC68882UM,
    // FBcc, FDBcc, FScc, FTRAPcc). setExcStatusBit() would also turn stale exception
    // bits into accrued bits.
    if ((condition & 0x10) && nan) fpsr |= FPEXP::BSUN | (1 << 7);

    switch (condition & 0xF) {

        case 0x0:   return false;                           // F, SF
        case 0x1:   return z;                               // EQ, SEQ
        case 0x2:   return !(nan || z || n);                // OGT, GT
        case 0x3:   return z || !(nan || n);                // OGE, GE
        case 0x4:   return n && !(nan || z);                // OLT, LT
        case 0x5:   return z || (n && !nan);                // OLE, LE
        case 0x6:   return !(nan || z);                     // OGL, GL
        case 0x7:   return !nan;                            // OR, GLE
        case 0x8:   return nan;                             // UN, NGLE
        case 0x9:   return nan || z;                        // UEQ, NGL
        case 0xA:   return nan || !(n || z);                // UGT, NLE
        case 0xB:   return nan || z || !n;                  // UGE, NLT
        case 0xC:   return nan || (n && !z);                // ULT, NGE
        case 0xD:   return nan || z || n;                   // ULE, NGT
        case 0xE:   return !z;                              // NE, SNE
        default:    return true;                            // T, ST
    }
}

void
FPU::setQuotientByte(u8 byte)
{
    fpsr = (fpsr & ~FPSR::QUOTIENT_MASK) | u32(byte) << 16;
}

void
FPU::setQuotientByte(u8 byte, bool sign)
{
    setQuotientByte((byte & 0x7F) | (sign ? 0x80 : 0x00));
}

FPU::Scope::Scope(FPU &fpu) : fpu(fpu), saved(fpu.env)
{
    fpu.env.setRoundingMode(fpu.getRoundingMode());
    fpu.env.setPrecision(fpu.getPrecision());
    fpu.env.clearFlags();
}

FPU::Scope::Scope(FPU &fpu, Precision precision) : Scope(fpu)
{
    fpu.env.setPrecision(precision);
}

FPU::Scope::~Scope()
{
    fpu.collectFlags();
}

void
FPU::collectFlags()
{
    auto flags = env.flags();
    u32 mask = 0;

    if (flags & FloatFlags::INEXACT)        mask |= FPEXP::INEX2;
    if (flags & FloatFlags::UNFL)           mask |= FPEXP::UNFL;
    if (flags & FloatFlags::OVFL)           mask |= FPEXP::OVFL;
    if (flags & FloatFlags::DIV_BY_ZERO)    mask |= FPEXP::DZ;
    if (flags & FloatFlags::INVALID)        mask |= FPEXP::OPERR;
    if (flags & FloatFlags::SIG_NAN)           mask |= FPEXP::SIG_NAN;

    if (mask) setExcStatusBit(mask);
    env.clearFlags();
}

//
// Arithmetic
//

namespace {

/* FSGLMUL and FSGLDIV round the significand to single precision, but keep the
 * extended exponent range. If an overflow yields the largest finite number,
 * all significand bits are set.
 */
MFloat80 singleOverflow(FpuEnv &env, const MFloat80 &result)
{
    if ((env.flags() & FloatFlags::OVFL) && result.isFinite() && result.biasedExponent() == 0x7FFE) {
        return MFloat80::fromBits(result.signExp(), ~0ULL);
    }
    return result;
}

/* The result of dividing the magnitudes of two finite nonzero numbers a and b.
 * The remainder has the value 'rem' * 2^(unit - bias - 63) and the quotient
 * is given by its least significant bits.
 */
struct Division {

    u64 quotient = 0;
    u128 rem = 0;
    u128 divisor = 0;
    i32 unit = 0;
    bool small = false;     // The magnitude of a is less than half the magnitude of b
};

Division
divide(const MFloat80 &a, const MFloat80 &b)
{
    auto normalize = [](const MFloat80 &x, i32 &e) {
        int shift = std::countl_zero(x.mantissa());
        e = x.biasedExponent() - shift;
        return x.mantissa() << shift;
    };

    Division d;
    i32 ea, eb;
    u64 ma = normalize(a, ea), mb = normalize(b, eb);
    i32 diff = ea - eb;

    // Magnitudes that differ by more than a factor of 2 have a zero quotient
    if (diff < -1) {
        d.rem = ma; d.divisor = u128(mb) << 2; d.unit = ea; d.small = true;
        return d;
    }
    if (diff == -1) {
        d.rem = ma; d.divisor = u128(mb) << 1; d.unit = ea;
        return d;
    }

    // Binary long division of ma * 2^diff by mb. Only the last bits of the quotient are of interest.
    u128 rem = 0;
    u64 q = 0;
    for (i32 i = 63; i >= -diff; i--) {

        rem = rem << 1 | (i >= 0 ? (ma >> i) & 1 : 0);
        bool bit = rem >= mb;
        if (bit) rem -= mb;
        q = q << 1 | u64(bit);
    }
    d.quotient = q;
    d.rem = rem;
    d.divisor = mb;
    d.unit = eb;
    return d;
}

// Converts the remainder of a division into a number
MFloat80
remainder(FpuEnv &env, const Division &d, bool sign)
{
    if (d.rem == 0) return MFloat80::zero(sign);

    int length = 128 - (d.rem >> 64 ? std::countl_zero(u64(d.rem >> 64)) : 64 + std::countl_zero(u64(d.rem)));
    u64 sig = length > 64 ? u64(d.rem >> (length - 64)) : u64(d.rem << (64 - length));
    return env.roundPack(sign, d.unit + length - 64, sig, 0);
}

}

MFloat80
FPU::round(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.round(value);
}

std::optional<MFloat80>
FPU::resolveNan(const MFloat80 &op1, const MFloat80 &op2)
{
    /* MC68881/MC68882 Floating-Point Coprocessor User's Manual:
     *
     * 4.5.4.1 NON-SIGNALING NANS
     *
     * "If either, but not both, operand of an operation is a NAN, and it is a
     *  non-signaling NAN, then that NAN is returned as the result. If both
     *  operands are non-signaling NANs, then the destination operand
     *  non-signaling NAN is returned as the result.
     *
     * 4.5.4.2 SIGNALING NANS
     *
     * If either operand to an operation is a signaling NAN (SNAN), then the
     * SNAN bit is set in the FPSR EXC byte. If the SNAN trap enable bit is
     * set in the FPCR ENABLE byte, then the trap is taken and the destination
     * is not modified. If the SNAN trap enable bit is not set, then the SNAN
     * is converted to a non-signaling NAN (by setting the SNAN bit in the
     * operand to a one), and the operation continues as described in the
     * preceding section for non-signaling NANs."
     */
    if (!op1.isNaN() && !op2.isNaN()) return std::nullopt;

    auto quiet = [&](const MFloat80 &op) {

        if (op.isSignalingNaN()) setExcStatusBit(FPEXP::SIG_NAN);
        return MFloat80::fromBits(op.signExp(), op.mantissa() | (1ULL << 62));
    };

    // Both operands are processed, because both can raise the SNAN exception
    auto r1 = quiet(op1);
    auto r2 = quiet(op2);

    return op1.isNaN() ? r1 : r2;
}

std::optional<MFloat80>
FPU::resolveNan(const MFloat80 &op)
{
    return resolveNan(op, MFloat80::zero());
}

MFloat80
FPU::moveOutExtended(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    if (value.biasedExponent() != 0 || value.mantissa() == 0) return value;

    // The value is a denormal number
    env.raiseFlags(FloatFlags::UNFL);
    bool sign = value.sign();
    u16 signBit = sign ? 0x8000 : 0;

    auto precision = env.precision();
    if (precision == Precision::EXTENDED) {

        // No bits get lost (a pseudo denormal number becomes a normalized number)
        return MFloat80::fromBits(u16(signBit | (value.mantissa() >> 63)), value.mantissa());
    }

    /* The exponent of a denormal number is far below the range of the selected
     * precision. Only a sticky bit remains, which is rounded according to the
     * rounding mode. The result carries the smallest exponent of the format.
     */
    bool single = precision == Precision::SINGLE;
    u64 mask = single ? 0xFFFFFFFFFFULL : 0x7FFULL;
    u16 exponent = single ? 0x3F81 : 0x3C01;

    u64 increment;
    switch (env.roundingMode()) {

        case RoundingMode::NEAREST: increment = (mask + 1) >> 1; break;
        case RoundingMode::ZERO:    increment = 0; break;
        case RoundingMode::DOWN:    increment = sign ? mask : 0; break;
        default:                    increment = sign ? 0 : mask; break;
    }
    env.raiseFlags(FloatFlags::INEXACT);
    return MFloat80::fromBits(u16(signBit | exponent), (1 + increment) & ~mask);
}

MFloat80
FPU::fabs(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return round(value.abs());
}

MFloat80
FPU::fneg(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return round(-value);
}

MFloat80
FPU::fsqrt(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.sqrt(value);
}

MFloat80
FPU::fsin(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.sin(value);
}

MFloat80
FPU::fcos(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.cos(value);
}

MFloat80
FPU::ftan(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.tan(value);
}

MFloat80
FPU::fasin(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.asin(value);
}

MFloat80
FPU::facos(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.acos(value);
}

MFloat80
FPU::fatan(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.atan(value);
}

MFloat80
FPU::fsinh(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.sinh(value);
}

MFloat80
FPU::fcosh(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.cosh(value);
}

MFloat80
FPU::ftanh(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.tanh(value);
}

MFloat80
FPU::fatanh(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.atanh(value);
}

MFloat80
FPU::fetox(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.etox(value);
}

MFloat80
FPU::fetoxm1(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.etoxm1(value);
}

MFloat80
FPU::ftwotox(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.twotox(value);
}

MFloat80
FPU::ftentox(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.tentox(value);
}

MFloat80
FPU::flogn(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.logn(value);
}

MFloat80
FPU::flognp1(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.lognp1(value);
}

MFloat80
FPU::flog2(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.log2(value);
}

MFloat80
FPU::flog10(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.log10(value);
}

void
FPU::fsincos(const MFloat80 &value, MFloat80 &sine, MFloat80 &cosine)
{
    if (auto nan = resolveNan(value)) { sine = cosine = *nan; return; }
    env.sincos(value, sine, cosine);
}

MFloat80
FPU::fint(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;
    return env.roundToInt(value);
}

MFloat80
FPU::fintrz(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;

    FpuEnv::Scope scope(env);
    env.setRoundingMode(RoundingMode::ZERO);
    return env.roundToInt(value);
}

MFloat80
FPU::fgetexp(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;

    if (value.isInfinite()) { setExcStatusBit(FPEXP::OPERR); return defaultNaN(); }
    if (value.isZero()) return value;

    // The exponent of a denormalized number is determined after normalization
    i32 e = i32(value.biasedExponent()) - MFloat80::bias - std::countl_zero(value.mantissa());
    return MFloat80::fromInt(e);
}

MFloat80
FPU::fgetman(const MFloat80 &value)
{
    if (auto nan = resolveNan(value)) return *nan;

    if (value.isInfinite()) { setExcStatusBit(FPEXP::OPERR); return defaultNaN(); }
    if (value.isZero()) return value;

    u64 man = value.mantissa() << std::countl_zero(value.mantissa());
    return MFloat80::fromBits(u16((value.sign() ? 0x8000 : 0) | MFloat80::bias), man);
}

MFloat80
FPU::ftst(const MFloat80 &value)
{
    auto result = value;
    if (auto nan = resolveNan(value)) result = *nan;

    setConditionCodes(result);
    return result;
}

MFloat80
FPU::fadd(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;
    return env.add(dst, src);
}

MFloat80
FPU::fsub(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;
    return env.sub(dst, src);
}

MFloat80
FPU::fmul(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;
    return env.mul(dst, src);
}

MFloat80
FPU::fdiv(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;
    return env.div(dst, src);
}

MFloat80
FPU::fscal(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;

    // Scaling by an infinite value is invalid
    if (src.isInfinite()) { setExcStatusBit(FPEXP::OPERR); return defaultNaN(); }

    // Infinities and zeroes are unaffected by scaling
    if (dst.isInfinite() || dst.isZero()) return dst;

    return env.scale(dst, src);
}

MFloat80
FPU::fsglmul(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;

    // The operands are truncated to single precision, the product is rounded to
    // single precision
    auto truncate = [&](const MFloat80 &x) {

        // The 24 most significant bits after normalization are kept
        if (!x.isFinite() || x.mantissa() == 0) return x;
        int lead = 63 - std::countl_zero(x.mantissa());
        u64 mask = lead > 23 ? ~((1ULL << (lead - 23)) - 1) : ~0ULL;
        return MFloat80::fromBits(x.signExp(), x.mantissa() & mask);
    };

    FpuEnv::Scope scope(env);
    env.setPrecision(Precision::SINGLE);
    env.setLimitsExponent(false);
    return singleOverflow(env, env.mul(truncate(dst), truncate(src)));
}

MFloat80
FPU::fsgldiv(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;

    // The quotient of the unmodified operands is rounded to single precision
    FpuEnv::Scope scope(env);
    env.setPrecision(Precision::SINGLE);
    env.setLimitsExponent(false);
    return singleOverflow(env, env.div(dst, src));
}

/* A zero remainder is built by normalizing a zero significand, which drags the
 * exponent down by 128. Depending on the rounding precision, this can leave the
 * exponent range, in which case an underflow or overflow is reported.
 */
static MFloat80 zeroRemainder(FpuEnv &env, const MFloat80 &dst, const MFloat80 &src, bool sign)
{
    // Exponents are normalized if the operand is a denormal number
    auto exponent = [](const MFloat80 &v) {
        return v.biasedExponent() ? int(v.biasedExponent()) : 1 - std::countl_zero(v.mantissa());
    };
    int bExp = exponent(src);
    int diff = std::max(exponent(dst) - bExp, 0);

    // Mimic the quotient loops of the reference
    diff -= 64;
    while (diff > 0) diff -= 62;
    diff += 64;
    int zExp = bExp + diff - 128;

    // Exponent offset and mask of the discarded significand bits
    auto precision = env.precision();
    int offset = precision == Precision::SINGLE ? 0x3F80 : precision == Precision::DOUBLE ? 0x3C00 : 0;
    u64 discarded = precision == Precision::SINGLE ? 0xFFFFFFFFFFULL : precision == Precision::DOUBLE ? 0x7FFULL : 0;

    if (precision != Precision::EXTENDED && zExp > 0x7FFE - offset) {

        // Overflow
        env.raiseFlags(FloatFlags::OVFL);
        auto mode = env.roundingMode();
        bool largest = mode == RoundingMode::ZERO ||
                       (sign && mode == RoundingMode::UP) || (!sign && mode == RoundingMode::DOWN);
        return largest ? MFloat80::fromBits(u16((sign ? 0x8000 : 0) | (0x7FFE - offset)), ~discarded)
                       : MFloat80::infinity(sign);
    }
    if (zExp < offset + (precision == Precision::EXTENDED ? 0 : 1)) env.raiseFlags(FloatFlags::UNFL);

    return MFloat80::zero(sign);
}

MFloat80
FPU::fmod(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;

    // Invalid operations (dst infinite, src zero) have no quotient
    if (dst.isInfinite() || src.isZero()) return env.invalidOperation();

    bool negative = dst.sign() != src.sign();
    if (src.isInfinite() || dst.isZero()) {

        setQuotientByte(0, negative);

        // A zero destination is normalized for an infinite source, which raises UNFL
        if (src.isInfinite() && dst.isZero()) env.raiseFlags(FloatFlags::UNFL);
        return env.round(dst);
    }

    // The remainder is computed exactly and then rounded like any other result
    auto d = divide(dst, src);
    setQuotientByte(u8(d.quotient & 0x7F), negative);

    return d.rem == 0 ? zeroRemainder(env, dst, src, dst.sign()) : remainder(env, d, dst.sign());
}

MFloat80
FPU::frem(const MFloat80 &dst, const MFloat80 &src)
{
    if (auto nan = resolveNan(dst, src)) return *nan;

    if (dst.isInfinite() || src.isZero()) return env.invalidOperation();

    bool negative = dst.sign() != src.sign();
    if (src.isInfinite() || dst.isZero()) {

        setQuotientByte(0, negative);

        // A zero destination is normalized for an infinite source, which raises UNFL
        if (src.isInfinite() && dst.isZero()) env.raiseFlags(FloatFlags::UNFL);
        return env.round(dst);
    }

    auto d = divide(dst, src);
    bool sign = dst.sign();

    // Round the quotient to the nearest integer (ties to even)
    u128 twice = d.rem << 1;
    if (twice > d.divisor || (twice == d.divisor && (d.quotient & 1))) {
        d.quotient++;
        d.rem = d.divisor - d.rem;
        sign = !sign;
    }
    setQuotientByte(u8(d.quotient & 0x7F), negative);

    // A destination smaller than half the source is returned unrounded
    if (d.small) return dst;

    return d.rem == 0 ? zeroRemainder(env, dst, src, sign) : remainder(env, d, sign);
}

void
FPU::fcmp(const MFloat80 &dst_, const MFloat80 &src_)
{
    MFloat80 dst = dst_, src = src_;

    // Only the 68040 keeps the sign of a NaN (the N flag is set for a negative NaN)
    if (model != FPUModel::M68040) {
        if (dst.isNaN()) dst = dst.abs();
        if (src.isNaN()) src = src.abs();
    }
    if (auto nan = resolveNan(dst, src)) { setConditionCodes(*nan); return; }

    // Unnormal operands are normalized first (as far as the exponent allows)
    auto normalize = [](const MFloat80 &v) {

        u16 exp = v.biasedExponent();
        u64 man = v.mantissa();
        if (exp == 0 || exp == 0x7FFF) return v;
        if (man == 0) return MFloat80::zero(v.sign());

        int shift = std::min<int>(std::countl_zero(man), exp);
        return MFloat80::fromBits(u16((v.sign() ? 0x8000 : 0) | (exp - shift)), man << shift);
    };
    dst = normalize(dst);
    src = normalize(src);

    /* The difference isn't computed. The operands' exponents and significands are
     * compared instead, which produces +-1 or +-0. Hence, the I flag is never set
     * (not even for infinite operands).
     */
    constexpr u64 one = 0x8000000000000000ULL;
    u16 aExp = dst.biasedExponent(), bExp = src.biasedExponent();
    u64 aSig = dst.mantissa(), bSig = src.mantissa();
    bool aSign = dst.sign(), bSign = src.sign();
    auto pack = [](bool sign, u16 exp, u64 man) { return MFloat80::fromBits(u16((sign ? 0x8000 : 0) | exp), man); };

    MFloat80 r;
    if      (bExp < aExp)                r = pack(aSign, 0x3FFF, one);
    else if (aExp < bExp)                r = pack(!bSign, 0x3FFF, one);
    else if (aExp == 0x7FFF)             r = aSign == bSign ? pack(aSign, 0, 0) : pack(aSign, 0x3FFF, one);
    else if (bSig < aSig)                r = pack(aSign, 0x3FFF, one);
    else if (aSig < bSig)                r = pack(!bSign, 0x3FFF, one);
    else if (aSig == 0)                  r = pack(aSign, 0, 0);
    else if (aSign == bSign)             r = pack(false, 0, 0);
    else                                 r = pack(aSign, 0x3FFF, one);
    setConditionCodes(r);
}

MFloat80
FPU::readCR(unsigned nr)
{
    struct RomEntry { u16 hi; u64 lo; i64 r1; i64 r2; bool inex; };

    static constexpr RomEntry rom1[] = {

        { 0x4000, 0xc90fdaa22168c235, -1,  0,  1 }, // 0x00: Pi
        { 0x4001, 0xfe00068200000000,  0,  0,  0 }, // 0x01: Undocumented
        { 0x4001, 0xffc0050380000000,  0,  0,  0 }, // 0x02: Undocumented
        { 0x2000, 0x7FFFFFFF00000000,  0,  0,  0 }, // 0x03: Undocumented
        { 0x0000, 0xFFFFFFFFFFFFFFFF,  0,  0,  0 }, // 0x04: Undocumented
        { 0x3C00, 0xFFFFFFFFFFFFF800,  0,  0,  0 }, // 0x05: Undocumented
        { 0x3F80, 0xFFFFFF0000000000,  0,  0,  0 }, // 0x06: Undocumented
        { 0x0001, 0xF65D8D9C00000000,  0,  0,  0 }, // 0x07: Undocumented
        { 0x7FFF, 0x401E000000000000,  0,  0,  0 }, // 0x08: Undocumented
        { 0x43F3, 0xE000000000000000,  0,  0,  0 }, // 0x09: Undocumented
        { 0x4072, 0xC000000000000000,  0,  0,  0 }, // 0x0A: Undocumented
        { 0x3ffd, 0x9a209a84fbcff798,  0,  1,  1 }, // 0x0B: Log10(2)
        { 0x4000, 0xadf85458a2bb4a9a,  0,  1,  1 }, // 0x0C: E
        { 0x3fff, 0xb8aa3b295c17f0bc, -1,  0,  1 }, // 0x0D: Log2(e)
        { 0x3ffd, 0xde5bd8a937287195,  0,  0,  0 }, // 0x0E: Log10(e)
        { 0x0000, 0x0000000000000000,  0,  0,  0 }  // 0x0F: 0.0
    };

    static constexpr RomEntry rom2[] = {

        { 0x3ffe, 0xb17217f7d1cf79ac, -1,  0,  1  }, // 0x30: Ln(2)
        { 0x4000, 0x935d8dddaaa8ac17, -1,  0,  1  }, // 0x31: Ln(10)
        { 0x3FFF, 0x8000000000000000,  0,  0,  0  }, // 0x32: 10^0
        { 0x4002, 0xA000000000000000,  0,  0,  0  }, // 0x33: 10^1
        { 0x4005, 0xC800000000000000,  0,  0,  0  }, // 0x34: 10^2
        { 0x400C, 0x9C40000000000000,  0,  0,  0  }, // 0x35: 10^4
        { 0x4019, 0xBEBC200000000000,  0,  0,  0  }, // 0x36: 10^8
        { 0x4034, 0x8E1BC9BF04000000,  0,  0,  0  }, // 0x37: 10^16
        { 0x4069, 0x9DC5ADA82B70B59E, -1,  0,  1  }, // 0x38: 10^32
        { 0x40D3, 0xC2781F49FFCFA6D5,  0,  1,  1  }, // 0x39: 10^64
        { 0x41A8, 0x93BA47C980E98CE0, -1,  0,  1  }, // 0x3A: 10^128
        { 0x4351, 0xAA7EEBFB9DF9DE8E, -1,  0,  1  }, // 0x3B: 10^256
        { 0x46A3, 0xE319A0AEA60E91C7, -1,  0,  1  }, // 0x3C: 10^512
        { 0x4D48, 0xC976758681750C17,  0,  1,  1  }, // 0x3D: 10^1024
        { 0x5A92, 0x9E8B3B5DC53D5DE5, -1,  0,  1  }, // 0x3E: 10^2048
        { 0x7525, 0xC46052028A20979B, -1,  0,  1  }  // 0x3F: 10^4096
    };

    auto readRom = [&](const RomEntry &entry) {

        u64 lo = entry.lo;

        // Adjust the last digit if the rounding mode requires it
        switch (fpcr & FPCR::RND_MASK) {

            case FPCR::RND_ZERO:    lo += entry.r1; break;
            case FPCR::RND_DOWN:    lo += entry.r1; break;
            case FPCR::RND_UP:      lo += entry.r2; break;
        }

        // Mark the value as inexact if necessary
        if (entry.inex) setExcStatusBit(FPEXP::INEX2);

        return MFloat80::fromBits(entry.hi, lo);
    };

    // Values outside the two ranges seem to produce a Guru on the real machine
    if (nr < 0x10) return readRom(rom1[nr]);
    if (nr >= 0x30 && nr < 0x40) return readRom(rom2[nr - 0x30]);

    return MFloat80::zero();
}

int
FPU::stateFrameSize(FpuFrameType type) const
{
    switch (type) {

        case FpuFrameType::IDLE:

            switch (model) {
                case FPUModel::M68040:     return 0;
                case FPUModel::M68881:     return 24;
                case FPUModel::M68882:     return 56;
            }
            break;

        case FpuFrameType::UNIMP:

            return model == FPUModel::M68040 ? 44 : 0;

        case FpuFrameType::BUSY:

            switch (model) {
                case FPUModel::M68040:     return 92;
                case FPUModel::M68881:     return 180;
                case FPUModel::M68882:     return 212;
            }
            break;

        default:
            break;
    }

    return 0;
}

int
FPU::stateFrameSize(u32 fmtWord) const
{
    return (fmtWord >> 16) & 0xFF;
}

FpuFrameType
FPU::typeOfFrame(u32 fmtWord) const
{
    int size = stateFrameSize(fmtWord);

    if (size == stateFrameSize(FpuFrameType::NULL_FRAME)) return FpuFrameType::NULL_FRAME;
    if (size == stateFrameSize(FpuFrameType::IDLE)) return FpuFrameType::IDLE;
    if (size == stateFrameSize(FpuFrameType::UNIMP)) return FpuFrameType::UNIMP;
    if (size == stateFrameSize(FpuFrameType::BUSY)) return FpuFrameType::BUSY;

    return FpuFrameType::INVALID;
}

u32
FPU::computeFormatWord(FpuFrameType type) const
{
    switch (type) {

        case FpuFrameType::IDLE:

            // The 68040 uses a different version number than the 6888x
            if (model == FPUModel::M68040) return 0x41000000;
            return 0x1F000000 | u32(stateFrameSize(type)) << 16;

        case FpuFrameType::UNIMP:
        case FpuFrameType::BUSY:

            return 0x1F000000 | u32(stateFrameSize(type)) << 16;

        default:

            return 0;
    }
}

}
