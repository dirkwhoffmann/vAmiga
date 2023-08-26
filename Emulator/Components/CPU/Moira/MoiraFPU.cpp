// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "MoiraFPU.h"
#include "Moira.h"
#include "MoiraMacros.h"
#include <cfenv>
#include <sstream>

namespace vamiga::moira {

FpuExtended
FPUReg::get()
{
    FpuExtended result = val;

    softfloat::float_exception_flags = 0;

    if (fpu.getPrecision() != FPU_PREC_EXTENDED) {

        if (fpu.getPrecision() == FPU_PREC_SINGLE) {
            result = FpuSingle(result, [this](int flags) { fpu.setExcStatusBit(flags); } );
        }
        if (fpu.getPrecision() == FPU_PREC_DOUBLE) {
            result = FpuDouble(result, [this](int flags) { fpu.setExcStatusBit(flags); } );
        }
    }
    if (!isNormalized()) {

        fpu.setExcStatusBit(FPEXP_UNFL);
    }

    return result;
}

void
FPUReg::set(const FpuExtended other)
{
    val = other;

    // Round to the correct precision
    val = get();

    // Experimental
    val.normalize();

    // Experimental
    if (val.isSignalingNaN()) {
        val.raw.low |= (1L << 62); // Make nonsignaling
        fpu.setExcStatusBit(FPEXP_SNAN);
    }

    printf("FPUReg::set %x,%llx (%f) flags = %x\n", val.raw.high, val.raw.low, val.asDouble(), softfloat::float_exception_flags);
}

void
FPUReg::move(FPUReg &dest)
{
    dest.set(val);
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

void
FPU::pushRoundingMode(int mode)
{
    oldRoundingMode = fegetround();

    switch (mode) {

        case FPU_RND_NEAREST:   fesetround(FE_TONEAREST); break;
        case FPU_RND_ZERO:      fesetround(FE_TOWARDZERO); break;
        case FPU_RND_DOWNWARD:  fesetround(FE_DOWNWARD); break;
        case FPU_RND_UPWARD:    fesetround(FE_UPWARD); break;
    }
}

void
FPU::popRoundingMode()
{
    fesetround(oldRoundingMode);
}

int
FPU::setRoundingMode(int mode)
{
    auto oldMode = fegetround();

    switch (mode) {

        case FPU_RND_NEAREST:   fesetround(FE_TONEAREST); break;
        case FPU_RND_ZERO:      fesetround(FE_TOWARDZERO); break;
        case FPU_RND_DOWNWARD:  fesetround(FE_DOWNWARD); break;
        case FPU_RND_UPWARD:    fesetround(FE_UPWARD); break;
    }

    return oldMode;
}

bool
FPU::isValidExt(Instr I, Mode M, u16 op, u32 ext) const
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
FPU::setFPIAR(u32 value)
{
    fpiar = value;
}

void
FPU::setExcStatusBit(u32 mask)
{
    printf("setExcStatusBit\n");
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

}
