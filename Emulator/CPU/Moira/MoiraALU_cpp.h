// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Sanitizer friendly macros for adding signed offsets to u32 values
#define U32_ADD(x,y) (u32)((i64)(x) + (i64)(y))
#define U32_SUB(x,y) (u32)((i64)(x) - (i64)(y))
#define U32_ADD3(x,y,z) (u32)((i64)(x) + (i64)(y) + (i64)(z))
#define U32_SUB3(x,y,z) (u32)((i64)(x) - (i64)(y) - (i64)(z))

// Sanitizer friendly macros for adding signed offsets to u64 values
#define U64_ADD(x,y) (u64)((i64)(x) + (i64)(y))
#define U64_SUB(x,y) (u64)((i64)(x) - (i64)(y))
#define U64_ADD3(x,y,z) (u64)((i64)(x) + (i64)(y) + (i64)(z))
#define U64_SUB3(x,y,z) (u64)((i64)(x) - (i64)(y) - (i64)(z))


template<Size S> u32 MSBIT() {
    if constexpr (S == Byte) return 0x00000080;
    if constexpr (S == Word) return 0x00008000;
    if constexpr (S == Long) return 0x80000000;
}

template<Size S> u32 CLIP(u64 data) {
    if constexpr (S == Byte) return data & 0x000000FF;
    if constexpr (S == Word) return data & 0x0000FFFF;
    if constexpr (S == Long) return data & 0xFFFFFFFF;
}

template<Size S> u32 CLEAR(u64 data) {
    if constexpr (S == Byte) return data & 0xFFFFFF00;
    if constexpr (S == Word) return data & 0xFFFF0000;
    if constexpr (S == Long) return data & 0x00000000;
}

template<Size S> i32 SEXT(u64 data) {
    if constexpr (S == Byte) return (i8)data;
    if constexpr (S == Word) return (i16)data;
    if constexpr (S == Long) return (i32)data;
}

template<Size S> bool NBIT(u64 data) {
    if constexpr (S == Byte) return (data & 0x00000080) != 0;
    if constexpr (S == Word) return (data & 0x00008000) != 0;
    if constexpr (S == Long) return (data & 0x80000000) != 0;
}

template<Size S> bool CARRY(u64 data) {
    if constexpr (S == Byte) return data & 0x000000100;
    if constexpr (S == Word) return data & 0x000010000;
    if constexpr (S == Long) return data & 0x100000000;
}

template<Size S> bool ZERO(u64 data) {
    if constexpr (S == Byte) return !(data & 0x000000FF);
    if constexpr (S == Word) return !(data & 0x0000FFFF);
    if constexpr (S == Long) return !(data & 0xFFFFFFFF);
}

template<Size S> u32 WRITE(u32 d1, u32 d2) {
    if constexpr (S == Byte) return (d1 & 0xFFFFFF00) | (d2 & 0x000000FF);
    if constexpr (S == Word) return (d1 & 0xFFFF0000) | (d2 & 0x0000FFFF);
    if constexpr (S == Long) return d2;
}

template<Instr I, Size S> u32
Moira::shift(int cnt, u64 data) {

    switch(I) {

        case ASL:
        case ASL_LOOP:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                u64 shifted = data << 1;
                changed |= (u32)(data ^ shifted);
                data = shifted;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = NBIT<S>(changed);
            break;
        }
        case ASR:
        case ASR_LOOP:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                u64 shifted = SEXT<S>(data) >> 1;
                changed |= (u32)(data ^ shifted);
                data = shifted;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = NBIT<S>(changed);
            break;
        }
        case LSL:
        case LSL_LOOP:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case LSR:
        case LSR_LOOP:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data = data >> 1;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROL:
        case ROL_LOOP:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1 | (carry ? 1 : 0);
            }
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROR:
        case ROR_LOOP:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data >>= 1;
                if (carry) data |= MSBIT<S>();
            }
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROXL:
        case ROXL_LOOP:
        {
            bool carry = reg.sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = NBIT<S>(data);
                data = data << 1 | (extend ? 1 : 0);
            }

            reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROXR:
        case ROXR_LOOP:
        {
            bool carry = reg.sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = data & 1;
                data >>= 1;
                if (extend) data |= MSBIT<S>();
            }

            reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        default:
            fatalError;
    }
    
    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    return CLIP<S>(data);
}

template<Instr I, Size S> u32
Moira::addsub(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ADD:
        case ADD_LOOP:
        case ADDI:
        case ADDQ:
        {
            result = U64_ADD(op1, op2);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case ADDX:
        case ADDX_LOOP:
        {
            result = U64_ADD3(op1, op2, reg.sr.x);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            if (CLIP<S>(result)) reg.sr.z = 0;
            break;
        }
        case SUB:
        case SUB_LOOP:
        case SUBI:
        case SUBQ:
        {
            result = U64_SUB(op2, op1);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case SUBX:
        case SUBX_LOOP:
        {
            result = U64_SUB3(op2, op1, reg.sr.x);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            if (CLIP<S>(result)) reg.sr.z = 0;
            break;
        }
        default:
        {
            fatalError;
        }
    }
    reg.sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Instr I> u32
Moira::mul(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

         case MULS:
         {
             result = (i16)op1 * (i16)op2;
             break;
         }
         case MULU:
         {
             result = op1 * op2;
             break;
         }
        default:
            fatalError;
     }

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    sync(cyclesMul<I>((u16)op1));
    return result;
}

template <Instr I> u32
Moira::div(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

    reg.sr.n = reg.sr.z = reg.sr.v = reg.sr.c = 0;

    switch (I) {

        case DIVS: // Signed division
        {
            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            result = (u32)((quotient & 0xffff) | remainder << 16);
            overflow = ((quotient & 0xffff8000) != 0 &&
                        (quotient & 0xffff8000) != 0xffff8000);
            overflow |= op1 == 0x80000000 && (i16)op2 == -1;
            break;
        }
        case DIVU: // Unsigned division
        {
            i64 quotient  = op1 / op2;
            u16 remainder = (u16)(op1 % op2);

            result = (u32)((quotient & 0xffff) | remainder << 16);
            overflow = quotient > 0xFFFF;
            break;
        }
        default:
        {
            fatalError;
        }
    }
    reg.sr.v = overflow ? 1        : reg.sr.v;
    reg.sr.n = overflow ? 1        : NBIT<Word>(result);
    reg.sr.z = overflow ? reg.sr.z : ZERO<Word>(result);

    sync(cyclesDiv<I>(op1, (u16)op2) - 4);
    return overflow ? op1 : result;
}

template<Instr I, Size S> u32
Moira::bcd(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ABCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op1_lo + op2_lo + reg.sr.x;
            u16 resHi = op1_hi + op2_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            if (resLo > 9) result += 6;
            reg.sr.x = reg.sr.c = (result & 0x3F0) > 0x90;
            if (reg.sr.c) result += 0x60;
            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmp_result & 0x80) == 0) && ((result & 0x80) == 0x80);
            break;
        }
        case SBCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op2_lo - op1_lo - reg.sr.x;
            u16 resHi = op2_hi - op1_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            int bcd = 0;
            if (resLo & 0xf0) {
                bcd = 6;
                result -= 6;
            }
            if (((op2 - op1 - reg.sr.x) & 0x100) > 0xff) result -= 0x60;
            reg.sr.c = reg.sr.x = ((op2 - op1 - bcd - reg.sr.x) & 0x300) > 0xff;

            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmp_result & 0x80) == 0x80) && ((result & 0x80) == 0);
            break;
        }
        default:
        {
            fatalError;
        }
    }
    reg.sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Size S> void
Moira::cmp(u32 op1, u32 op2)
{
    u64 result = U64_SUB(op2, op1);
    
    reg.sr.c = NBIT<S>(result >> 1);
    reg.sr.v = NBIT<S>((op2 ^ op1) & (op2 ^ result));
    reg.sr.z = ZERO<S>(result);
    reg.sr.n = NBIT<S>(result);
}

template<Instr I, Size S> u32
Moira::logic(u32 op)
{
    u32 result;

    switch(I) {

        case NOT:
        case NOT_LOOP:
        {
            result = ~op;
            reg.sr.n = NBIT<S>(result);
            reg.sr.z = ZERO<S>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;
        }
        case NEG:
        case NEG_LOOP:
        {
            result = addsub<SUB,S>(op, 0);
            break;
        }
        case NEGX:
        case NEGX_LOOP:
        {
            result = addsub<SUBX,S>(op, 0);
            break;
        }
        default:
            fatalError;
    }
    return result;
}

template<Instr I, Size S> u32
Moira::logic(u32 op1, u32 op2)
{
    u32 result;

    switch(I) {

        case AND: case ANDI: case ANDICCR: case ANDISR:
        {
            result = op1 & op2;
            break;
        }
        case OR: case ORI: case ORICCR: case ORISR:
        {
            result = op1 | op2;
            break;
        }
        case EOR: case EORI: case EORICCR: case EORISR:
        {
            result = op1 ^ op2;
            break;
        }
        default:
        {
            fatalError;
        }
    }

    reg.sr.n = NBIT<S>(result);
    reg.sr.z = ZERO<S>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;
    return result;
}

template <Instr I> u32
Moira::bit(u32 op, u8 bit)
{
    switch (I) {
        case BCHG:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op ^= (1 << bit);
            break;
        }
        case BSET:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op |= (1 << bit);
            break;
        }
        case BCLR:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op &= ~(1 << bit);
            break;
        }
        case BTST:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            break;
        }
        default:
        {
            fatalError;
        }
    }
    return op;
}

template <Instr I> bool
Moira::cond() {

    switch(I) {

        case BRA: case ST:  case DBT:  case DBT_LOOP:  return true;
        case SF:            case DBF:  case DBF_LOOP:  return false;
        case BHI: case SHI: case DBHI: case DBHI_LOOP: return !reg.sr.c && !reg.sr.z;
        case BLS: case SLS: case DBLS: case DBLS_LOOP: return reg.sr.c || reg.sr.z;
        case BCC: case SCC: case DBCC: case DBCC_LOOP: return !reg.sr.c;
        case BCS: case SCS: case DBCS: case DBCS_LOOP: return reg.sr.c;
        case BNE: case SNE: case DBNE: case DBNE_LOOP: return !reg.sr.z;
        case BEQ: case SEQ: case DBEQ: case DBEQ_LOOP: return reg.sr.z;
        case BVC: case SVC: case DBVC: case DBVC_LOOP: return !reg.sr.v;
        case BVS: case SVS: case DBVS: case DBVS_LOOP: return reg.sr.v;
        case BPL: case SPL: case DBPL: case DBPL_LOOP: return !reg.sr.n;
        case BMI: case SMI: case DBMI: case DBMI_LOOP: return reg.sr.n;
        case BGE: case SGE: case DBGE: case DBGE_LOOP: return reg.sr.n == reg.sr.v;
        case BLT: case SLT: case DBLT: case DBLT_LOOP: return reg.sr.n != reg.sr.v;
        case BGT: case SGT: case DBGT: case DBGT_LOOP: return reg.sr.n == reg.sr.v && !reg.sr.z;
        case BLE: case SLE: case DBLE: case DBLE_LOOP: return reg.sr.n != reg.sr.v || reg.sr.z;
    }

    fatalError;
}

template <Instr I> int
Moira::cyclesBit(u8 bit)
{
    switch (I)
    {
        case BTST: return 2;
        case BCLR: return MIMIC_MUSASHI ? 6 : (bit > 15 ? 6 : 4);
        case BSET:
        case BCHG: return MIMIC_MUSASHI ? 4 : (bit > 15 ? 4 : 2);
    }

    fatalError;
}

template <Instr I> int
Moira::cyclesMul(u16 data)
{
    int mcycles = 17;

    switch (I)
    {
        case MULU:
        {
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
        case MULS:
        {
            data = ((data << 1) ^ data) & 0xFFFF;
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
    }

    fatalError;
}

template <Instr I> int
Moira::cyclesDiv(u32 op1, u16 op2)
{
    switch (I)
    {
        case DIVU:
        {
            u32 dividend = op1;
            u16 divisor  = op2;
            int mcycles  = 38;

            // Check if quotient is larger than 16 bit
            if ((dividend >> 16) >= divisor) return 10;
            u32 hdivisor = divisor << 16;

            for (int i = 0; i < 15; i++) {
                if ((i32)dividend < 0) {
                    // dividend <<= 1;
                    dividend = (u32)((u64)dividend << 1);
                    dividend = U32_SUB(dividend, hdivisor);
                } else {
                    // dividend <<= 1;
                    dividend = (u32)((u64)dividend << 1);
                    if (dividend >= hdivisor) {
                        dividend = U32_SUB(dividend, hdivisor);
                        mcycles += 1;
                    } else {
                        mcycles += 2;
                    }
                }
            }
            return 2 * mcycles;
        }
        case DIVS:
        {
            i32 dividend = (i32)op1;
            i16 divisor  = (i16)op2;
            int mcycles  = (dividend < 0) ? 7 : 6;

            // Check if quotient is larger than 16 bit
            if ((abs(dividend) >> 16) >= abs(divisor))
                return (mcycles + 2) * 2;

            mcycles += 55;

            if (divisor >= 0) {
                mcycles += (dividend < 0) ? 1 : -1;
            }

            u32 aquot = abs(dividend) / abs(divisor);
            for (int i = 0; i < 15; i++) {
                if ( (i16)aquot >= 0) mcycles++;
                aquot <<= 1;
            }
            return 2 * mcycles;
        }
    }

    fatalError;
}

template <Type CPU, Instr I> u32
Moira::mulMusashi(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case MULS:
        {
            result = (i16)op1 * (i16)op2;
            break;
        }
        case MULU:
        {
            result = op1 * op2;
            break;
        }
        default:
            fatalError;
    }

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Type CPU, Instr I> u32
Moira::divMusashi(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case DIVS:
        {
            sync <CPU> (154, 118);

            if (op1 == 0x80000000 && (i32)op2 == -1) {

                reg.sr.z = 0;
                reg.sr.n = 0;
                reg.sr.v = 0;
                reg.sr.c = 0;
                result = 0;
                break;
            }

            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            if (quotient == (i16)quotient) {

                reg.sr.z = quotient;
                reg.sr.n = NBIT<Word>(quotient);
                reg.sr.v = 0;
                reg.sr.c = 0;
                result = (quotient & 0xffff) | (u16)remainder << 16;

            } else {

                result = op1;
                reg.sr.v = 1;
            }
            break;
        }
        case DIVU:
        {
            sync <CPU> (136, 104);

            i64 quotient  = op1 / op2;
            u16 remainder = (u16)(op1 % op2);

            if(quotient < 0x10000) {

                reg.sr.z = quotient;
                reg.sr.n = NBIT<Word>(quotient);
                reg.sr.v = 0;
                reg.sr.c = 0;

                result = (quotient & 0xffff) | remainder << 16;

            } else {

                result = op1;
                reg.sr.v = 1;
            }
            break;
        }
    }

    return result;
}
