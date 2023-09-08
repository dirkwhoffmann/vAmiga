// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "config.h"
#include "MoiraFPU.h"
#include "Moira.h"
#include "MoiraMacros.h"
#include <cfenv>
#include <sstream>

namespace vamiga::moira {

FpuExtended
FPUReg::asExtended()
{
    FpuExtended result = val;
    
    if (val.isfinite()) {
        
        softfloat::float_exception_flags = 0;
        
        if (fpu.getPrecision() != FPU_PREC_EXTENDED) {
            
            if (fpu.getPrecision() == FPU_PREC_SINGLE) {
                
                auto val = FpuSingle(result, [this](int flags) { fpu.setExcStatusBit(flags); } );
                
                if (val.isposinf()) { result = FpuExtended::posInf; }
                else if (val.isneginf()) { result = FpuExtended::negInf; }
                else result = val;
            }
            if (fpu.getPrecision() == FPU_PREC_DOUBLE) {
                
                auto val = FpuDouble(result, [this](int flags) { fpu.setExcStatusBit(flags); } );
                
                if (val.isposinf()) { result = FpuExtended::posInf; }
                else if (val.isneginf()) { result = FpuExtended::negInf; }
                else result = val;
                
            }
        }
        if (issubnormal()) {
            
            fpu.setExcStatusBit(FPEXP_UNFL);
        }
    }
    return result;
}

void
FPUReg::set(const FpuExtended other)
{
    val = other;
    
    printf("FPUReg (1): %x,%llx\n", val.raw.high, val.raw.low);
    
    // Round to the correct precision
    val = asExtended();
    
    printf("FPUReg (2): %x,%llx\n", val.raw.high, val.raw.low);
    
    // Experimental
    val.normalize();
    
    // REMOVE THIS!
    if (val.isSignalingNaN()) {
        val.raw.low |= (1LL << 62); // Make nonsignaling
        fpu.setExcStatusBit(FPEXP_SNAN);
    }
    
    printf("FPUReg::set %x,%llx (%Lf) flags = %x\n", val.raw.high, val.raw.low, val.asLongDouble(), softfloat::float_exception_flags);
}

FPU::FPU(Moira& ref) : moira(ref)
{
    static_assert(!REQUIRE_PRECISE_FPU || sizeof(long double) > 8,
                  "No long double support. FPU inaccuracies may occur.");
}

void
FPU::reset()
{
    for (int i = 0; i < 8; i++) {
        fpr[i].reset();
    }
    
    fpiar = 0;
    fpsr = 0;
    fpcr = 0;
}

void
FPU::setModel(FPUModel model)
{
    // Only proceed if the model changes
    if (this->model == model) return;
    
    this->model = model;
}

FpuPrecision
FPU::getPrecision() const
{
    switch (fpcr & 0xC0) {
            
        case 0x00:  return FPU_PREC_EXTENDED;
        case 0x40:  return FPU_PREC_SINGLE;
        case 0x80:  return FPU_PREC_DOUBLE;
        default:    return FPU_PREC_UNDEFINED;
    }
}

FpuRoundingMode
FPU::getRoundingMode() const
{
    switch (fpcr & 0x30) {
            
        case 0x00:  return FPU_RND_NEAREST;
        case 0x10:  return FPU_RND_ZERO;
        case 0x20:  return FPU_RND_DOWNWARD;
        default:    return FPU_RND_UPWARD;
    }
}

FpuRoundingMode
FPU::fesetround(FpuRoundingMode mode)
{
    auto oldMode = fegetround();
    
    switch (mode) {
            
        case FPU_RND_NEAREST:   ::fesetround(FE_TONEAREST); break;
        case FPU_RND_ZERO:      ::fesetround(FE_TOWARDZERO); break;
        case FPU_RND_DOWNWARD:  ::fesetround(FE_DOWNWARD); break;
        default:                ::fesetround(FE_UPWARD); break;
    }
    
    switch (oldMode) {
            
        case FE_TONEAREST:      return FPU_RND_NEAREST;
        case FE_TOWARDZERO:     return FPU_RND_ZERO;
        case FE_DOWNWARD:       return FPU_RND_DOWNWARD;
        default:                return FPU_RND_UPWARD;
    }
}

bool
FPU::isValidExt(Instr I, Mode M, u16 op, u32 ext)
{
    auto cod  = xxx_____________ (ext);
    auto mode = ___xx___________ (ext);
    auto fmt  = ___xxx__________ (ext);
    auto lst  = ___xxx__________ (ext);
    auto cmd  = _________xxxxxxx (ext);
    
    switch (I) {
            
        case FDBcc:
        case FScc:
        case FTRAPcc:
            
            return (ext & 0xFFE0) == 0;
            
        case FMOVECR:
            
            return (op & 0x3F) == 0;
            
        case FMOVE:
            
            switch (cod) {
                    
                case 0b010:
                    
                    if (M == MODE_IP) break;
                    return true;
                    
                case 0b000:
                    
                    if (cmd == 0 && cod == 0 && (op & 0x3F)) break;
                    return true;
                    
                case 0b011:
                    
                    if (fmt != 0b011 && fmt != 0b111 && (ext & 0x7F)) break;
                    
                    if (M == MODE_DN) {
                        if (fmt == 0b010 || fmt == 0b011 || fmt == 0b101 || fmt == 0b111) break;
                    }
                    if (M == MODE_AN) {
                        if (fmt == 0b011 || fmt == 0b111) break;
                    }
                    if (M == MODE_DIPC || M == MODE_IXPC || M == MODE_IM || M == MODE_IP) {
                        break;
                    } else {
                        if (fmt == 0b111 && (ext & 0xF)) break;
                    }
                    
                    return true;
            }
            
        case FMOVEM:
            
            switch (cod) {
                    
                case 0b101:
                {
                    
                    if (ext & 0x3FF) break;
                    
                    if (M == MODE_DN || M == MODE_AN) {
                        if (lst != 0b000 && lst != 0b001 && lst != 0b010 && lst != 0b100) break;
                    }
                    if (M == MODE_DIPC || M == MODE_IXPC || M == MODE_IM || M == MODE_IP) {
                        break;
                    }
                    return true;
                }
                case 0b100:
                    
                    if (ext & 0x3FF) break;
                    if (M == MODE_IP) break;
                    return true;
                    
                case 0b110:
                case 0b111:
                    
                    if (ext & 0x0700) break;
                    if (mode == 3 && (ext & 0x8F)) break;
                    
                    if (M == MODE_DN || M == MODE_AN) {
                        break;
                    }
                    if (M == MODE_DIPC || M == MODE_IXPC || M == MODE_IM || M == MODE_IP) {
                        break;
                    }
                    if (M == MODE_AI) {
                        if (mode == 0 || mode == 1) break;
                    }
                    if (M == MODE_PI) {
                        if (mode == 0 || mode == 1 || cod == 0b111) break;
                    }
                    if (M == MODE_PD) {
                        if (cod == 0b110) break;
                        if (cod == 0b111 && (mode == 1) && (ext & 0x8F)) break;
                        if (cod == 0b111 && (mode == 2 || mode == 3)) break;
                    }
                    if (M == MODE_DI || M == MODE_IX || M == MODE_AW || M == MODE_AL) {
                        if (mode == 0 || mode == 1) break;
                    }
                    return true;
            }
            return false;
            
        default:
            fatalError;
    }
}

void
FPU::setFPCR(u32 value)
{
    fpcr = value & 0x0000FFF0;
    
    softfloat::float_rounding_mode = (value & 0b110000) >> 4;
}

void
FPU::setFPSR(u32 value)
{
    fpsr = value & 0x0FFFFFF8;
}

void
FPU::setExcStatusBit(u32 mask)
{
    assert((mask & ~0xFF00) == 0);
    
    fpsr |= mask;
    
    // Set sticky bits (accrued exception byte)
    if (fpsr & (FPEXP_SNAN | FPEXP_OPERR))                  SET_BIT(fpsr, 7);
    if (fpsr & FPEXP_OVFL)                                  SET_BIT(fpsr, 6);
    if ((fpsr & FPEXP_UNFL) && (fpsr & FPEXP_INEX2))        SET_BIT(fpsr, 5);
    if (fpsr & FPEXP_DZ)                                    SET_BIT(fpsr, 4);
    if (fpsr & (FPEXP_INEX1 | FPEXP_INEX2 | FPEXP_OVFL))    SET_BIT(fpsr, 3);
}

void
FPU::clearExcStatusBit(u32 mask)
{
    assert((mask & ~0xFF00) == 0);
    
    fpsr &= ~mask;
}

void
FPU::setConditionCodes(int reg)
{
    assert(reg >= 0 && reg <= 7);
    setConditionCodes(fpr[reg].val);
}

void
FPU::setConditionCodes(const FpuExtended &value)
{
    bool n = value.raw.high & 0x8000;
    bool z = (value.raw.high & 0x7fff) == 0 && value.raw.low == 0;
    bool i = (value.raw.high & 0x7fff) == 0x7fff && (value.raw.low << 1) == 0;
    bool nan = softfloat::floatx80_is_nan(value.raw);
    
    REPLACE_BIT(fpsr, 27, n);
    REPLACE_BIT(fpsr, 26, z);
    REPLACE_BIT(fpsr, 25, i);
    REPLACE_BIT(fpsr, 24, nan);
}

void
FPU::setFPIAR(u32 value)
{
    fpiar = value;
}

FpuExtended
FPU::readCR(unsigned nr)
{
    FpuExtended result;
    
    typedef struct { u16 hi; u64 lo; i64 r1; i64 r2; bool inex; } RomEntry;
    
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
        
        { 0x3ffe, 0xb17217f7d1cf79ac, -1,  0,  1  }, // 0x00: Ln(2)
        { 0x4000, 0x935d8dddaaa8ac17, -1,  0,  1  }, // 0x01: Ln(10)
        { 0x3FFF, 0x8000000000000000,  0,  0,  0  }, // 0x02: 10^0
        { 0x4002, 0xA000000000000000,  0,  0,  0  }, // 0x03: 10^1
        { 0x4005, 0xC800000000000000,  0,  0,  0  }, // 0x04: 10^2
        { 0x400C, 0x9C40000000000000,  0,  0,  0  }, // 0x05: 10^4
        { 0x4019, 0xBEBC200000000000,  0,  0,  0  }, // 0x06: 10^8
        { 0x4034, 0x8E1BC9BF04000000,  0,  0,  0  }, // 0x07: 10^16
        { 0x4069, 0x9DC5ADA82B70B59E, -1,  0,  1  }, // 0x08: 10^32
        { 0x40D3, 0xC2781F49FFCFA6D5,  0,  1,  1  }, // 0x09: 10^64
        { 0x41A8, 0x93BA47C980E98CE0, -1,  0,  1  }, // 0x0A: 10^128
        { 0x4351, 0xAA7EEBFB9DF9DE8E, -1,  0,  1  }, // 0x0B: 10^256
        { 0x46A3, 0xE319A0AEA60E91C7, -1,  0,  1  }, // 0x0C: 10^512
        { 0x4D48, 0xC976758681750C17,  0,  1,  1  }, // 0x0D: 10^1024
        { 0x5A92, 0x9E8B3B5DC53D5DE5, -1,  0,  1  }, // 0x0E: 10^2048
        { 0x7525, 0xC46052028A20979B, -1,  0,  1  }  // 0x0F: 10^4096
    };
    
    auto readRom = [&](const RomEntry &entry) {
        
        auto result = FpuExtended(entry.hi, entry.lo);
        
        // Round if necessary
        if ((fpcr & 0b110000) == 0b010000) result.raw.low += entry.r1;
        if ((fpcr & 0b110000) == 0b100000) result.raw.low += entry.r1;
        if ((fpcr & 0b110000) == 0b110000) result.raw.low += entry.r2;
        
        // Mark value as inexact if necessary
        if (entry.inex) setExcStatusBit(FPEXP_INEX2);
        
        return result;
    };
    
    if (nr >= 0x40) {
        // Values in this range seem to produce a Guru on the real machine
    }
    
    if (nr >= 0x00 && nr < 0x10) result = readRom(rom1[nr]);
    if (nr >= 0x30 && nr < 0x40) result = readRom(rom2[nr - 0x30]);
    
    return result;
}

void
FPU::clearHostFpuFlags()
{
    feclearexcept(FE_ALL_EXCEPT);
}

void
FPU::copyHostFpuFlags()
{
    if (fetestexcept(FE_INEXACT))   { setExcStatusBit(FPEXP_INEX2); }
    if (fetestexcept(FE_UNDERFLOW)) { setExcStatusBit(FPEXP_UNFL); }
    if (fetestexcept(FE_OVERFLOW))  { setExcStatusBit(FPEXP_OVFL); }
    if (fetestexcept(FE_DIVBYZERO)) { setExcStatusBit(FPEXP_DZ); }
    if (fetestexcept(FE_INVALID))   { setExcStatusBit(FPEXP_OPERR); }
}

FpuExtended
FPU::makeNonsignalingNan(const FpuExtended &value)
{
    return FpuExtended(value.raw.high, value.raw.low | (1LL << 62));
}

std::optional<FpuExtended>
FPU::resolveNan(const FpuExtended &op1, const FpuExtended &op2)
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
     * preceding section for non-signaling NANs.
     */
    
    if (!op1.isnan() && !op2.isnan()) return {};
    
    auto o1 = op1;
    auto o2 = op2;
    
    if (o1.isSignalingNaN()) {
        
        o1.raw.low |= (1LL << 62);
        setExcStatusBit(FPEXP_SNAN);
    }
    if (o2.isSignalingNaN()) {
        
        o2.raw.low |= (1LL << 62);
        setExcStatusBit(FPEXP_SNAN);
    }
    
    return o2.isnan() ? o2 : o1;
}

std::optional<FpuExtended>
FPU::resolveNan(const FpuExtended &op)
{
    if (!op.isnan()) return {};
    
    auto o = op;
    
    if (o.isSignalingNaN()) {
        
        o.raw.low |= (1LL << 62);
        setExcStatusBit(FPEXP_SNAN);
    }
    
    return o;
}

FpuExtended
FPU::monadic(const FpuExtended &value, std::function<long double(long double)> func)
{
    switch (value.fpclassify()) {
            
            /*
             case FP_INFINITE:
             
             setExcStatusBit(FPEXP_OPERR);
             return FpuExtended(0x7FFF, 0xFFFFFFFFFFFFFFFF);
             */
            
        case FP_NAN:
            
            printf("monadic FP_NAN\n");
            return makeNonsignalingNan(value);
            
        default:
            
            clearHostFpuFlags();
            auto result = func(value.asLongDouble());
            copyHostFpuFlags();
            
            printf("Monadic(%Lf) = %Lf\n", value.asLongDouble(), result);
            // EXPERIMENTAL
            if (fetestexcept(FE_INVALID)) {
                return FpuExtended(0x7FFF, 0xFFFFFFFFFFFFFFFF);
            }
            if (std::isnan(result)) {
                return FpuExtended(0x7FFF, 0xFFFFFFFFFFFFFFFF);
            }
            
            return FpuExtended(result, getRoundingMode(), exceptionHandler);
    }
}

FpuExtended
FPU::fabs(const FpuExtended &value)
{
    printf("fabs(%Lf)\n", value.asLongDouble());
    
    if (value.isnan()) { return makeNonsignalingNan(value); }
    
    auto result = value;
    result.raw.high &= 0x7FFF;
    
    return result;
}

FpuExtended
FPU::facos(const FpuExtended &value)
{
    printf("facos %x %llx\n", value.raw.high, value.raw.low);
    
    return monadic(value, [&](long double x) { return std::acos(x); });
}

FpuExtended
FPU::fasin(const FpuExtended &value)
{
    printf("fasin %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::asin(x); });
}

FpuExtended
FPU::fatan(const FpuExtended &value)
{
    printf("fatan %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::atan(x); });
}

FpuExtended
FPU::fatanh(const FpuExtended &value)
{
    printf("fatanh %x %llx\n", value.raw.high, value.raw.low);
    if (value == -1.0) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::negInf;
    }
    if (value == 1.0) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::posInf;
    }
    
    return monadic(value, [&](long double x) { return std::atanh(x); });
}

FpuExtended
FPU::fcos(const FpuExtended &value)
{
    printf("fcos %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::cos(x); });
}

FpuExtended
FPU::fcosh(const FpuExtended &value)
{
    printf("fcosh %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::cosh(x); });
}

FpuExtended
FPU::fetox(const FpuExtended &value)
{
    printf("fetox %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::expl(x); });
}

FpuExtended
FPU::fetoxm1(const FpuExtended &value)
{
    printf("fetoxm1 %x %llx\n", value.raw.high, value.raw.low);
    
    if (value.iszero()) {
        
        return value;
    }
    
    return monadic(value, [&](long double x) { return std::expl(x) - 1.0L; });
}

FpuExtended
FPU::fgetexp(const FpuExtended &value)
{
    printf("fgetexp %x %llx\n", value.raw.high, value.raw.low);
    
    if (value.iszero()) {
        
        return value;
    }
    if (value.isinf()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    
    /*
     long exp = (value.raw.high & 0x7FFF) - FpuExtended::bias;
     return FpuExtended(exp);
     */
    return FpuExtended((value.raw.high & 0x7FFF) - FpuExtended::bias);
}

FpuExtended
FPU::fgetman(const FpuExtended &value)
{
    printf("fgetexp %x %llx\n", value.raw.high, value.raw.low);
    
    if (value.iszero()) {
        
        return value;
    }
    if (value.isinf()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    
    /*
     auto result = value;
     result.raw.high = (result.raw.high & 0x8000) | 0x3FFF;
     return result;
     */
    return FpuExtended((value.raw.high & 0x8000) | 0x3FFF, value.raw.low);
}

FpuExtended
FPU::fint(const FpuExtended &value)
{
    if (value.iszero()) {
        
        return value;
    }
    if (value.isinf()) {
        
        return value;
    }
    
    softfloat::float_exception_flags = 0;
    auto rounded = softfloat::floatx80_round_to_int(value.raw);
    
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        setExcStatusBit(FPEXP_INEX2);
    }
    /*
     if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
     flags |= FPEXP_OVFL;
     }
     if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
     flags |= FPEXP_UNFL;
     }
     */
    
    return FpuExtended(rounded.high, rounded.low);
}

FpuExtended
FPU::fintrz(const FpuExtended &value)
{
    if (value.iszero()) {
        
        return value;
    }
    if (value.isinf()) {
        
        return value;
    }
    
    softfloat::float_exception_flags = 0;
    auto oldmode = softfloat::float_rounding_mode;
    softfloat::float_rounding_mode = 1;
    auto rounded = softfloat::floatx80_round_to_int(value.raw);
    softfloat::float_rounding_mode = oldmode;
    
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        setExcStatusBit(FPEXP_INEX2);
    }
    /*
     if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
     flags |= FPEXP_OVFL;
     }
     if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
     flags |= FPEXP_UNFL;
     }
     */
    
    return FpuExtended(rounded.high, rounded.low);
}

FpuExtended
FPU::flog10(const FpuExtended &value)
{
    if (value.iszero()) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::negInf;
    }
    if (value.isnegative()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (value.isinf()) {
        
        if (value.ispositive()) {
            
            return FpuExtended::posInf;
            
        } else {
            
            setExcStatusBit(FPEXP_OPERR);
            return FpuExtended::nan;
        }
    }
    
    return monadic(value, [&](long double x) { return std::log10(x); });
}

FpuExtended
FPU::flog2(const FpuExtended &value)
{
    if (value.iszero()) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::negInf;
    }
    if (value.isnegative()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (value.isinf()) {
        
        if (value.ispositive()) {
            
            return FpuExtended::posInf;
            
        } else {
            
            setExcStatusBit(FPEXP_OPERR);
            return FpuExtended::nan;
        }
    }
    
    return monadic(value, [&](long double x) { return std::log2(x); });
}

FpuExtended
FPU::flogn(const FpuExtended &value)
{
    if (value.iszero()) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::negInf;
    }
    if (value.isnegative()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (value.isinf()) {
        
        if (value.ispositive()) {
            
            return FpuExtended::posInf;
            
        } else {
            
            setExcStatusBit(FPEXP_OPERR);
            return FpuExtended::nan;
        }
    }
    
    return monadic(value, [&](long double x) { return std::log(x); });
}

FpuExtended
FPU::flognp1(const FpuExtended &value)
{
    if (value.iszero()) {
        
        return value;
    }
    if (value == -1.0) {
        
        setExcStatusBit(FPEXP_DZ);
        return FpuExtended::negInf;
    }
    if (value < -1.0) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (value.isinf()) {
        
        if (value.ispositive()) {
            
            return FpuExtended::posInf;
            
        } else {
            
            setExcStatusBit(FPEXP_OPERR);
            return FpuExtended::nan;
        }
    }
    
    return monadic(value, [&](long double x) { return std::log(x + 1); });
}

FpuExtended
FPU::fneg(const FpuExtended &value)
{
    printf("fneg(%Lf)\n", value.asLongDouble());

    auto result = value;
    result.raw.high ^= 0x8000;

    return result;
}

FpuExtended
FPU::fsin(const FpuExtended &value)
{
    printf("fsin %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::sin(x); });
}

FpuExtended
FPU::fsinh(const FpuExtended &value)
{
    printf("fsinh %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::sinh(x); });
}

FpuExtended
FPU::fsqrt(const FpuExtended &value)
{
    printf("fsqrt %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::sqrt(x); });
}

FpuExtended
FPU::ftan(const FpuExtended &value)
{
    printf("ftan %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::tan(x); });
}

FpuExtended
FPU::ftanh(const FpuExtended &value)
{
    printf("ftanh %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::tanh(x); });
}

FpuExtended
FPU::ftentox(const FpuExtended &value)
{
    printf("ftentox %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::powl(10.0L, x); });
}

FpuExtended
FPU::ftwotox(const FpuExtended &value)
{
    printf("ftwotox %x %llx\n", value.raw.high, value.raw.low);
    return monadic(value, [&](long double x) { return std::powl(2.0L, x); });
}

FpuExtended
FPU::fadd(const FpuExtended &op1, const FpuExtended &op2)
{
    if (op1.iszero() && op2.iszero()) {
        
        if (op1.signbit() == op2.signbit()) {
            return op1;
        } else if (getRoundingMode() == FPU_RND_DOWNWARD) {
            return FpuExtended::negZero;
        } else {
            return FpuExtended::posZero;
        }
    }
    if (op1.isinf() && op2.isinf() && op1.signbit() != op2.signbit()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (op1.isinf()) {
        
        return op1;
    }
    if (op2.isinf()) {
        
        return op2;
    }
    
    softfloat::float_exception_flags = 0;
    
    auto result = softfloat::floatx80_add(op1.raw, op2.raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        setExcStatusBit(FPEXP_INEX2);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        setExcStatusBit(FPEXP_OVFL);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        setExcStatusBit(FPEXP_UNFL);
    }
    
    return FpuExtended(result.high, result.low);
}

FpuExtended
FPU::fcmp(const FpuExtended &op1, const FpuExtended &op2)
{
    if (op1.iszero() && op2.iszero()) {
        
        return op2;
    }
    if (op1.iszero()) {
        
        return op2.signbit() ? -1 : 1;
    }
    if (op2.iszero()) {
        
        return op1.signbit() ? 1 : -1;
    }
    if (op1.isinf() && op2.isinf()) {
        
        if (op1.signbit() == 0 && op2.signbit() == 0) return FpuExtended::posZero;
        if (op1.signbit() == 0 && op2.signbit() == 1) return -1;
        if (op1.signbit() == 1 && op2.signbit() == 0) return 1;
        if (op1.signbit() == 1 && op2.signbit() == 1) return FpuExtended::negZero;
    }
    if (op1.isinf()) {
        
        return op1.signbit() ? 1 : -1;
    }
    if (op2.isinf()) {
        
        return op2.signbit() ? -1 : 1;
    }
    if (op1 == op2) {
        
        return op1.signbit() ? FpuExtended::negZero : FpuExtended::posZero;
    }
    if (op2.ispositive()) {

        return op1 > op2 ? -1.0 : 1.0;
    }
    return op1 < op2 ? 1.0 : -1.0;
}

FpuExtended
FPU::fdiv(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::fmod(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("fmod op1: (%x, %llx) (%x, %llx)\n", op1.raw.high, op1.raw.low, op2.raw.high, op2.raw.low);
    
    long double modulus;
    
    if (op1.iszero() || op2.isinf()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (op2.iszero()) {
        
        return op2;
    }
    if (op1.isinf()) {

        modulus = op2.asLongDouble();

    } else {
        
        clearHostFpuFlags();
        modulus = std::fmodl(op2.asLongDouble(), op1.asLongDouble());
        copyHostFpuFlags();
    }
    
    auto result = FpuExtended(modulus, getRoundingMode(), exceptionHandler);
    u32 quotientByte = (result.raw.low & 0xF);
    if (op1.signbit() ^ op2.signbit()) quotientByte |= 0x80;
    fpsr = (fpsr & 0xFF00FFFF) | quotientByte << 16;
    
    return result;
}

FpuExtended
FPU::fmul(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::frem(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::fscal(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::fsgldiv(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::fsglmul(const FpuExtended &op1, const FpuExtended &op2)
{
    printf("TODO\n");
    return 0;
}

FpuExtended
FPU::fsub(const FpuExtended &op1, const FpuExtended &op2)
{
    if (op1.iszero() && op2.iszero()) {
        
        if (op1.signbit() != op2.signbit()) {
            return FpuExtended::negZero;
        } else if (getRoundingMode() == FPU_RND_DOWNWARD) {
            return FpuExtended::negZero;
        } else {
            return FpuExtended::posZero;
        }
    }
    if (op1.isinf() && op2.isinf() && op1.signbit() == op2.signbit()) {
        
        setExcStatusBit(FPEXP_OPERR);
        return FpuExtended::nan;
    }
    if (op1.isinf()) {
        
        return -op1;
    }
    if (op2.isinf()) {
        
        return op2;
    }
    
    softfloat::float_exception_flags = 0;
    
    auto result = softfloat::floatx80_sub(op2.raw, op1.raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        setExcStatusBit(FPEXP_INEX2);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        setExcStatusBit(FPEXP_OVFL);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        setExcStatusBit(FPEXP_UNFL);
    }
    
    return FpuExtended(result.high, result.low);
}

}
