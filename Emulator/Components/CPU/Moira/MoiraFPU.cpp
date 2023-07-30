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

Float80::Float80(double value)
{
    raw = softfloat::float64_to_floatx80(*((u64 *)&value));
}

Float80::Float80(bool mSign, i16 e, u64 m)
{
    raw.high = (mSign ? 0x8000 : 0x0000) | (u16(e + 0x3FFF) & 0x7FFF);
    raw.low = m;
}

double
Float80::asDouble()
{
    auto value = softfloat::floatx80_to_float64(raw);
    return *((double *)&value);
}

long
Float80::asLong()
{
    auto value = softfloat::floatx80_to_int64(raw);
    return (long)value;
}

bool
Float80::isNormalized()
{
    if ((raw.high & 0x7F) == 0) return true;

    return raw.low == 0 || (raw.low & (1L << 63)) != 0;
}

void
Float80::normalize()
{
    while (!isNormalized()) {

        raw.high -= 1;
        raw.low <<= 1;
    }
}


void
FPU::reset()
{
    for (int i = 0; i < 8; i++) {
        fpr[i] = { };
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

Float80
FPU::getFPR(int n) const
{
    assert(n >= 0 && n <= 7);
    return fpr[n];
}

void
FPU::setFPR(int n, Float80 value)
{
    setFPR(n, value.raw.high, value.raw.low);
}

void
FPU::setFPR(int n, u16 high, u64 low)
{
    assert(n >= 0 && n <= 7);

    fpr[n].raw.high = high;
    fpr[n].raw.low = low;

    setFlags(fpr[n]);
}

void
FPU::setFPCR(u32 value)
{
    fpcr = value;
    softfloat::float_rounding_mode = (value & 0b110000) >> 4;
}

void
FPU::setFPSR(u32 value)
{
    fpsr = value;
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

    fpcr &= ~mask;
}

void
FPU::clearExcStatusBits()
{
    clearExcStatusBit(0xFF00);
}

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
    bool z = (value.raw.high & 0x7fff) == 0 && (value.raw.low << 1) == 0;
    bool i = (value.raw.high & 0x7fff) == 0x7fff && (value.raw.low << 1) == 0;
    bool nan = softfloat::floatx80_is_nan(value.raw);

    REPLACE_BIT(fpsr, 27, n);
    REPLACE_BIT(fpsr, 26, z);
    REPLACE_BIT(fpsr, 25, i);
    REPLACE_BIT(fpsr, 24, nan);
}

void
FPU::musashiPack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3)
{
    static u32 pkmask2[18] =
    {
        0xffffffff, 0, 0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
        0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
        0xffffffff, 0xffffffff, 0xffffffff
    };

    static u32 pkmask3[18] =
    {
        0xffffffff, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0xf0000000, 0xff000000, 0xfff00000, 0xffff0000,
        0xfffff000, 0xffffff00, 0xfffffff0, 0xffffffff,
    };

    char str[128], *ch;
    int i, j, exp;

    dw1 = dw2 = dw3 = 0;
    ch = &str[0];

    snprintf(str, sizeof(str), "%.16e", value.asDouble());

    if (*ch == '-')
    {
        ch++;
        dw1 = 0x80000000;
    }

    if (*ch == '+')
    {
        ch++;
    }

    dw1 |= (*ch++ - '0');

    if (*ch == '.')
    {
        ch++;
    }

    // handle negative k-factor here
    if ((k <= 0) && (k >= -13))
    {
        exp = 0;
        for (i = 0; i < 3; i++)
        {
            if (ch[18+i] >= '0' && ch[18+i] <= '9')
            {
                exp = (exp << 4) | (ch[18+i] - '0');
            }
        }

        if (ch[17] == '-')
        {
            exp = -exp;
        }

        k = -k;
        // last digit is (k + exponent - 1)
        k += (exp - 1);

        // round up the last significant mantissa digit
        if (ch[k+1] >= '5')
        {
            ch[k]++;
        }

        // zero out the rest of the mantissa digits
        for (j = (k+1); j < 16; j++)
        {
            ch[j] = '0';
        }

        // now zero out K to avoid tripping the positive K detection below
        k = 0;
    }

    // crack 8 digits of the mantissa
    for (i = 0; i < 8; i++)
    {
        dw2 <<= 4;
        if (*ch >= '0' && *ch <= '9')
        {
            dw2 |= *ch++ - '0';
        }
    }

    // next 8 digits of the mantissa
    for (i = 0; i < 8; i++)
    {
        dw3 <<= 4;
        if (*ch >= '0' && *ch <= '9')
            dw3 |= *ch++ - '0';
    }

    // handle masking if k is positive
    if (k >= 1)
    {
        if (k <= 17)
        {
            dw2 &= pkmask2[k];
            dw3 &= pkmask3[k];
        }
        else
        {
            dw2 &= pkmask2[17];
            dw3 &= pkmask3[17];
            //            m68ki_cpu.fpcr |=  (need to set OPERR bit)
        }
    }

    // finally, crack the exponent
    if (*ch == 'e' || *ch == 'E')
    {
        ch++;
        if (*ch == '-')
        {
            ch++;
            dw1 |= 0x40000000;
        }

        if (*ch == '+')
        {
            ch++;
        }

        j = 0;
        for (i = 0; i < 3; i++)
        {
            if (*ch >= '0' && *ch <= '9')
            {
                j = (j << 4) | (*ch++ - '0');
            }
        }

        dw1 |= (j << 16);
    }
}

void
FPU::pack(Float80 value, int k, u32 &dw1, u32 &dw2, u32 &dw3)
{
    auto frexp10 = [](double arg, int *exp) {
        *exp = (arg == 0) ? 0 : 1 + (int)std::floor(std::log10(std::fabs(arg) ) );
        return arg * std::pow(10 , -(*exp));
    };

    if (k >= 18) {
        // TODO: Sets the OPERR bit
        k = 17;
    }
    char str[128] = { };
    int e;
    int eSgn = 0;
    int mSgn = 0;

    printf("FPU::pack(%f)\n", value.asDouble());

    // Split value into exponent and mantissa
    auto m = frexp10(value.asDouble(), &e);

    // Lower the exponent by one, because the first digit is left of the comma
    e -= 1;

    // Get sign bits
    if (m < 0.0) { mSgn = 1; m = std::abs(m); }
    if (e < 0.0) { eSgn = 1; e = std::abs(e); }

    printf("e = %d m = %f (%d %d) k = %d\n", e, m, eSgn, mSgn, k);

    // Compute significant digits
    long v;
    if (k <= 0) {
        v = long(std::round(m * pow(10.0, e - k)));
    } else {
        v = long(std::round(m * pow(10.0, k)));
    }

    printf("digits = %ld\n", v);

    // Create a textual represention
    snprintf(str, sizeof(str), "%ld", std::abs(v));

    printf("digits (text) = %s\n", str);

    // Set sign bits
    dw1 = dw2 = dw3 = 0;
    if (mSgn) dw1 |= 0x80000000;
    if (eSgn) dw1 |= 0x40000000;

    // Write exponent
    dw1 |= ((e / 100) % 10) << 24;
    dw1 |= ((e / 10) % 10) << 20;
    dw1 |= (e % 10) << 24;

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
FPU::musashiUnpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result)
{
    double tmp;
    char str[128], *ch;

    ch = &str[0];
    if (dw1 & 0x80000000)    // mantissa sign
    {
        *ch++ = '-';
    }
    *ch++ = (char)((dw1 & 0xf) + '0');
    *ch++ = '.';
    *ch++ = (char)(((dw2 >> 28) & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 24) & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 20) & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 16) & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 12) & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 8)  & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 4)  & 0xf) + '0');
    *ch++ = (char)(((dw2 >> 0)  & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 28) & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 24) & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 20) & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 16) & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 12) & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 8)  & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 4)  & 0xf) + '0');
    *ch++ = (char)(((dw3 >> 0)  & 0xf) + '0');
    *ch++ = 'E';
    if (dw1 & 0x40000000)    // exponent sign
    {
        *ch++ = '-';
    }
    *ch++ = (char)(((dw1 >> 24) & 0xf) + '0');
    *ch++ = (char)(((dw1 >> 20) & 0xf) + '0');
    *ch++ = (char)(((dw1 >> 16) & 0xf) + '0');
    *ch = '\0';

    sscanf(str, "%le", &tmp);
    result = tmp;
}

void
FPU::unpack(u32 dw1, u32 dw2, u32 dw3, Float80 &result)
{
    double mantissa = 0.0;
    double exponent = 0.0;
    u64 m = 0;

    exponent = ((dw1 >> 24) & 0xF) * 100.0;
    exponent += ((dw1 >> 20) & 0xF) * 10.0;
    exponent += ((dw1 >> 16) & 0xF);

    double factor = 1.0;

    mantissa += ((dw1 >> 0) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 28) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 24) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 20) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 16) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 12) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 8) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 4) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw2 >> 0) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 28) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 24) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 20) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 16) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 12) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 8) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 4) & 0xF) * factor; factor /= 10.0;
    mantissa += ((dw3 >> 0) & 0xF) * factor; factor /= 10.0;

    m = 10 * m + ((dw1 >> 0) & 0xF);
    m = 10 * m + ((dw2 >> 28) & 0xF);
    m = 10 * m + ((dw2 >> 24) & 0xF);
    m = 10 * m + ((dw2 >> 20) & 0xF);
    m = 10 * m + ((dw2 >> 16) & 0xF);
    m = 10 * m + ((dw2 >> 12) & 0xF);
    m = 10 * m + ((dw2 >> 8) & 0xF);
    m = 10 * m + ((dw2 >> 4) & 0xF);
    m = 10 * m + ((dw2 >> 0) & 0xF);
    m = 10 * m + ((dw3 >> 28) & 0xF);
    m = 10 * m + ((dw3 >> 24) & 0xF);
    m = 10 * m + ((dw3 >> 20) & 0xF);
    m = 10 * m + ((dw3 >> 16) & 0xF);
    m = 10 * m + ((dw3 >> 12) & 0xF);
    m = 10 * m + ((dw3 >> 8) & 0xF);
    m = 10 * m + ((dw3 >> 4) & 0xF);
    m = 10 * m + ((dw3 >> 0) & 0xF);

    if (dw1 & 0x80000000) exponent *= -1;
    if (dw1 & 0x40000000) mantissa *= -1;

    double val = mantissa * pow(10.0, exponent);
    printf("val = %f  m = %lld sizeof(val) = %zu\n", val, m, sizeof(val));

    int exponent2;

    auto mantissa2 = frexp(val, &exponent2);
    printf("mantissa = %f mantissa2 = %f exponent2 = %d\n", mantissa, mantissa2, exponent2);
    exponent2 -= 1;

    u64 mmm = 0;
    for (isize i = 63; i >= 0; i--) {
        mantissa2 *= 2.0;
        if (mantissa2 >= 1.0) {
            mmm |= (1L << i);
            mantissa2 -= 1.0;
        } else {
            mmm &= ~(1L << i);
        }
    }

    // double combined = std::pow(10.0, exponent) * mantissa;
    // printf("            Exponent = %f Mantissa = %f (m = %lld mm = %llx mmm = %llx)\n", exponent, mantissa, m, mm, mmm);

    result = Float80(dw1 & 0x40000000, (i16)exponent2, mmm);
    result.normalize();
}

}
