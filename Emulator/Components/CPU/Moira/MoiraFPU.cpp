// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "MoiraFPU.h"
#include "Moira.h"
#include "MoiraMacros.h"

namespace vamiga::moira {

Float80
FPUReg::get()
{
    Float80 result = val;

    softfloat::float_exception_flags = 0;

    if ((fpu.fpcr & 0b11000000) == 0b01000000) {
        result.raw = softfloat::float32_to_floatx80(floatx80_to_float32(result.raw));
    } else if ((fpu.fpcr & 0b11000000) == 0b10000000) {
        result.raw = softfloat::float64_to_floatx80(floatx80_to_float64(result.raw));
    }

    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_overflow) {
        fpu.setExcStatusBit(FPEXP_OVFL);
    }
    if (softfloat::float_exception_flags & softfloat::float_flag_underflow) {
        fpu.setExcStatusBit(FPEXP_UNFL);
    }

    // Experimental
    if ((val.raw.high & 0x7FFF) == 0 && val.raw.low != 0 && (val.raw.low & (1L << 63)) == 0) {
        fpu.setExcStatusBit(FPEXP_UNFL);
    }

    // Set flags
    fpu.setFlags(val);

    return result;
}

u8
FPUReg::asByte()
{
    softfloat::float_exception_flags = 0;

    auto result = (u8)softfloat::floatx80_to_int32(get().raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }

    return result;
}

u16
FPUReg::asWord()
{
    softfloat::float_exception_flags = 0;

    auto result = (u16)softfloat::floatx80_to_int32(get().raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }

    return result;
}

u32
FPUReg::asLong()
{
    softfloat::float_exception_flags = 0;

    auto result = (u32)softfloat::floatx80_to_int32(get().raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }

    return result;

}

u32
FPUReg::asSingle()
{
    softfloat::float_exception_flags = 0;

    auto result = softfloat::floatx80_to_float32(get().raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }

    return result;
}

u64
FPUReg::asDouble()
{
    softfloat::float_exception_flags = 0;

    auto result = softfloat::floatx80_to_float64(get().raw);
    if (softfloat::float_exception_flags & softfloat::float_flag_inexact) {
        fpu.setExcStatusBit(FPEXP_INEX2);
    }

    printf("FPUReg::asDouble: %x,%llx -> %llx flags = %x\n", val.raw.high, val.raw.low, result, softfloat::float_exception_flags);
    return result;
}

Float80
FPUReg::asExtended()
{
    return get();
}

Packed
FPUReg::asPacked(int k)
{
    Packed result;
    fpu.pack(get(), k, result.data[0], result.data[1], result.data[2]);
    printf("Packing %x,%llx -> %x, %x, %x\n", val.raw.high, val.raw.low, result.data[0], result.data[1], result.data[2]);
    return result;
}

void
FPUReg::set(const Float80 other)
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


    // Set flags
    fpu.setFlags(val);
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

/*
void
FPU::clearExcStatusBits()
{
    clearExcStatusBit(0xFF00);
}
*/

void
FPU::setFlags(int reg)
{
    assert(reg >= 0 && reg <= 7);
    setFlags(fpr[reg]);
}

void
FPU::setFlags(const Float80 &value)
{
    bool n = value.raw.high & 0x8000;
    // bool z = (value.raw.high & 0x7fff) == 0 && (value.raw.low << 1) == 0;
    bool z = (value.raw.high & 0x7fff) == 0 && value.raw.low == 0;
    bool i = (value.raw.high & 0x7fff) == 0x7fff && (value.raw.low << 1) == 0;
    bool nan = softfloat::floatx80_is_nan(value.raw);

    REPLACE_BIT(fpsr, 27, n);
    REPLACE_BIT(fpsr, 26, z);
    REPLACE_BIT(fpsr, 25, i);
    REPLACE_BIT(fpsr, 24, nan);
}

Float80
FPU::readCR(unsigned nr)
{
    Float80 result;

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

        auto result = Float80(entry.hi, entry.lo);

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

long
FPU::roundmantissa(long double mantissa, int digits)
{
    auto shifted = mantissa * powl(10.0L, digits);
    long double rounded;
    switch (fpcr & 0x30) {
        case 0x00: rounded = std::roundl(shifted); printf("    round %.20Lf %.20Lf\n", mantissa, rounded); break;
        case 0x10: rounded = std::truncl(shifted); printf("    trunc %.20Lf %.20Lf\n", mantissa, rounded); break;
        case 0x20: rounded = std::floorl(shifted); printf("    floor %.20Lf %.20Lf\n", mantissa, rounded); break;
        default:   rounded = std::ceill(shifted);  printf("    ceil %.20Lf %.20Lf\n", mantissa, rounded); break;
    }
    if (std::abs(mantissa - rounded / powl(10.0L, digits)) > 1e-10) {
        setExcStatusBit(FPEXP_INEX2);
    }
    return long(rounded);
}

void
FPU::pack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3)
{
    /*
    auto rounded = [this](double m, int digits) {
        auto shifted = m * pow(10.0, digits);
        double rounded;
        switch (fpcr & 0x30) {
            case 0x00: rounded = std::round(shifted); printf("    round %f %f\n", m, rounded); break;
            case 0x10: rounded = std::trunc(shifted); printf("    trunc %f %f\n", m, rounded); break;
            case 0x20: rounded = std::floor(shifted); printf("    floor %f %f\n", m, rounded); break;
            default:   rounded = std::ceil(shifted);  printf("    ceil %f %f\n", m, rounded); break;
        }
        if (std::abs(m - rounded / pow(10.0, digits)) > 1e-10) {
            setExcStatusBit(FPEXP_INEX2);
        }
        return long(rounded);
    };
    */

    printf("FPU::pack %x,%llx k = %d\n", value.raw.high, value.raw.low, k);

    if (k > 17) {
        setExcStatusBit(FPEXP_OPERR);
        setExcStatusBit(FPEXP_INEX2);
        k = 17;
    }
    char str[128] = { };
    int eSgn = 0;
    int mSgn = 0;

    // Split value into exponent and mantissa
    auto split = value.frexp10();
    auto e = split.first;
    auto m = split.second;
    printf("    frexp10: e = %d m = %Lf\n", e, m);

    // Lower the exponent by one, because the first digit is left of the comma
    e -= 1;

    // Get sign bits
    if (m < 0.0) { mSgn = 1; m = std::abs(m); }
    if (e < 0.0) { eSgn = 1; e = std::abs(e); }

    // Determine the number of digits
    auto numDigits = std::min(17, k <= 0 ? e + 1 - k : k);

    printf("    e = %d m = %.20Lf (%d %d) k = %d numDigits = %d\n", e, m, eSgn, mSgn, k, numDigits);

    // Compute the digits
    long digits = roundmantissa(m, numDigits);
    printf("    digits = %ld\n", digits);

    // Create a textual represention
    snprintf(str, sizeof(str), "%ld", std::abs(digits));
    printf("    digits (text) = %s\n", str);

    // Set sign bits
    dw1 = dw2 = dw3 = 0;
    if (mSgn) dw1 |= 0x80000000;
    if (eSgn) dw1 |= 0x40000000;

    // Write exponent
    dw1 |= ((e / 100) % 10) << 24;
    dw1 |= ((e / 10) % 10) << 20;
    dw1 |= (e % 10) << 16;

    // Write mantissa
    dw1 |= ((str[0] - '0') & 0xF) << 0;
    dw2 |= ((str[1] - '0') & 0xF) << 28;
    dw2 |= ((str[2] - '0') & 0xF) << 24;
    dw2 |= ((str[3] - '0') & 0xF) << 20;
    dw2 |= ((str[4] - '0') & 0xF) << 16;
    dw2 |= ((str[5] - '0') & 0xF) << 12;
    dw2 |= ((str[6] - '0') & 0xF) << 8;
    dw2 |= ((str[7] - '0') & 0xF) << 4;
    dw2 |= ((str[8] - '0') & 0xF) << 0;
    dw3 |= ((str[9] - '0') & 0xF) << 28;
    dw3 |= ((str[10] - '0') & 0xF) << 24;
    dw3 |= ((str[11] - '0') & 0xF) << 20;
    dw3 |= ((str[12] - '0') & 0xF) << 16;
    dw3 |= ((str[13] - '0') & 0xF) << 12;
    dw3 |= ((str[14] - '0') & 0xF) << 8;
    dw3 |= ((str[15] - '0') & 0xF) << 4;
    dw3 |= ((str[16] - '0') & 0xF) << 0;
}

void
FPU::unpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result)
{
    long double factor = 10.0;
    long double exponent = 0.0;
    long double mantissa = 0.0;

    auto digit = [&](u32 d) {
        d &= 0xF; factor /= 10.0; return d * factor;
    };

    printf("FPU::unpack %x,%x,%x\n", dw1, dw2, dw3);

    // Extract exponent
    exponent += ((dw1 >> 24) & 0xF) * 100.0;
    exponent += ((dw1 >> 20) & 0xF) * 10.0;
    exponent += ((dw1 >> 16) & 0xF);

    // Extract mantissa
    mantissa += digit(dw1 >> 0);
    mantissa += digit(dw2 >> 28);
    mantissa += digit(dw2 >> 24);
    mantissa += digit(dw2 >> 20);
    mantissa += digit(dw2 >> 16);
    mantissa += digit(dw2 >> 12);
    mantissa += digit(dw2 >> 8);
    mantissa += digit(dw2 >> 4);
    mantissa += digit(dw2 >> 0);
    mantissa += digit(dw3 >> 28);
    mantissa += digit(dw3 >> 24);
    mantissa += digit(dw3 >> 20);
    mantissa += digit(dw3 >> 16);
    mantissa += digit(dw3 >> 12);
    mantissa += digit(dw3 >> 8);
    mantissa += digit(dw3 >> 4);
    mantissa += digit(dw3 >> 0);

    // Evaluate exponent sign bit
    if (dw1 & 0x80000000) mantissa *= -1;
    if (dw1 & 0x40000000) exponent *= -1;

    printf("    exponent = %Lf mantissa = %.20Lf\n", exponent, mantissa);

    auto combined = mantissa * powl(10.0, exponent);

    int e;
    auto m = std::frexpl(combined, &e);

    // Round
    m = std::ldexpl(m, 32);
    long double rounded;
    switch (fpcr & 0x30) {
        case 0x00: rounded = std::roundl(m); printf("    round %.20Lf %.20Lf\n", m, rounded); break;
        case 0x10: rounded = std::truncl(m); printf("    trunc %.20Lf %.20Lf\n", m, rounded); break;
        case 0x20: rounded = std::floorl(m); printf("    floor %.20Lf %.20Lf\n", m, rounded); break;
        default:   rounded = std::ceill(m);  printf("    ceil %.20Lf %.20Lf\n", m, rounded); break;
    }
    m = std::ldexpl(rounded, -32);


    // Round

    result = Float80(std::ldexpl(m, e));
}

}
