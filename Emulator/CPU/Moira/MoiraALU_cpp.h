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


template <Size S> u32 MSBIT() {
    if constexpr (S == Byte) return 0x00000080;
    if constexpr (S == Word) return 0x00008000;
    if constexpr (S == Long) return 0x80000000;
}

template <Size S> u32 CLIP(u64 data) {
    if constexpr (S == Byte) return data & 0x000000FF;
    if constexpr (S == Word) return data & 0x0000FFFF;
    if constexpr (S == Long) return data & 0xFFFFFFFF;
}

template <Size S> u32 CLEAR(u64 data) {
    if constexpr (S == Byte) return data & 0xFFFFFF00;
    if constexpr (S == Word) return data & 0xFFFF0000;
    if constexpr (S == Long) return data & 0x00000000;
}

template <Size S> i32 SEXT(u64 data) {
    if constexpr (S == 0)    return (i32)data;
    if constexpr (S == Byte) return (i8)data;
    if constexpr (S == Word) return (i16)data;
    if constexpr (S == Long) return (i32)data;
}

template <Size S> bool NBIT(u64 data) {
    if constexpr (S == Byte) return (data & 0x00000080) != 0;
    if constexpr (S == Word) return (data & 0x00008000) != 0;
    if constexpr (S == Long) return (data & 0x80000000) != 0;
}

template <Size S> bool CARRY(u64 data) {
    if constexpr (S == Byte) return data & 0x000000100;
    if constexpr (S == Word) return data & 0x000010000;
    if constexpr (S == Long) return data & 0x100000000;
}

template <Size S> bool ZERO(u64 data) {
    if constexpr (S == Byte) return !(data & 0x000000FF);
    if constexpr (S == Word) return !(data & 0x0000FFFF);
    if constexpr (S == Long) return !(data & 0xFFFFFFFF);
}

template <Size S> u32 WRITE(u32 d1, u32 d2) {
    if constexpr (S == Byte) return (d1 & 0xFFFFFF00) | (d2 & 0x000000FF);
    if constexpr (S == Word) return (d1 & 0xFFFF0000) | (d2 & 0x0000FFFF);
    if constexpr (S == Long) return d2;
}

template <Core C, Instr I, Size S> u32
Moira::shift(int cnt, u64 data) {

    switch (I) {

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

template <Core C, Instr I, Size S> u32
Moira::addsub(u32 op1, u32 op2)
{
    u64 result;

    switch (I) {

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
            fatalError;
    }

    reg.sr.n = NBIT<S>(result);

    return (u32)result;
}

template <Core C, Instr I> u32
Moira::mul(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case MULS:

            result = (i16)op1 * (i16)op2;
            break;

        case MULU:

            result = op1 * op2;
            break;

        default:
            fatalError;
    }

    reg.sr.n = NBIT <Long> (result);
    reg.sr.z = ZERO <Long> (result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Core C, Instr I> u32
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
            fatalError;
    }

    reg.sr.v = overflow ? 1        : reg.sr.v;
    reg.sr.n = overflow ? 1        : NBIT<Word>(result);
    reg.sr.z = overflow ? reg.sr.z : ZERO<Word>(result);

    return overflow ? op1 : result;
}

template <Core C, Instr I, Size S> u32
Moira::bcd(u32 op1, u32 op2)
{
    u64 result, tmpResult;

    switch (I) {

        case ABCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op1_lo + op2_lo + reg.sr.x;
            u16 resHi = op1_hi + op2_hi;

            result = tmpResult = resHi + resLo;
            if (resLo > 9) result += 6;
            reg.sr.x = reg.sr.c = (result & 0x3F0) > 0x90;
            if (reg.sr.c) result += 0x60;
            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmpResult & 0x80) == 0) && ((result & 0x80) == 0x80);
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

            result = tmpResult = resHi + resLo;
            int bcd = 0;
            if (resLo & 0xf0) {
                bcd = 6;
                result -= 6;
            }
            if (((op2 - op1 - reg.sr.x) & 0x100) > 0xff) result -= 0x60;
            reg.sr.c = reg.sr.x = ((op2 - op1 - bcd - reg.sr.x) & 0x300) > 0xff;

            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmpResult & 0x80) == 0x80) && ((result & 0x80) == 0);
            break;
        }

        default:
            fatalError;
    }

    reg.sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Core C, Size S> void
Moira::cmp(u32 op1, u32 op2)
{
    u64 result = U64_SUB(op2, op1);

    reg.sr.c = NBIT<S>(result >> 1);
    reg.sr.v = NBIT<S>((op2 ^ op1) & (op2 ^ result));
    reg.sr.z = ZERO<S>(result);
    reg.sr.n = NBIT<S>(result);
}

template <Core C, Instr I, Size S> u32
Moira::logic(u32 op)
{
    u32 result;

    switch (I) {

        case NOT:
        case NOT_LOOP:

            result = ~op;
            reg.sr.n = NBIT<S>(result);
            reg.sr.z = ZERO<S>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case NEG:
        case NEG_LOOP:

            result = addsub<C, SUB, S>(op, 0);
            break;

        case NEGX:
        case NEGX_LOOP:

            result = addsub<C, SUBX, S>(op, 0);
            break;

        default:
            fatalError;
    }
    return result;
}

template <Core C, Instr I, Size S> u32
Moira::logic(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case AND: case ANDI: case ANDICCR: case ANDISR:

            result = op1 & op2;
            break;

        case OR: case ORI: case ORICCR: case ORISR:

            result = op1 | op2;
            break;

        case EOR: case EORI: case EORICCR: case EORISR:

            result = op1 ^ op2;
            break;

        default:
            fatalError;
    }

    reg.sr.n = NBIT<S>(result);
    reg.sr.z = ZERO<S>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;
    return result;
}

template <Instr I> u32
Moira::bitfield(u32 data, u32 offset, u32 width, u32 mask)
{
    u32 result;

    switch (I) {

        case BFCHG:

            result = data ^ mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case BFCLR:

            result = data & ~mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case BFSET:

            result = data | mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case BFEXTS:

            result = SEXT<Long>(data) >> (32 - width);

            reg.sr.n = NBIT<Long>(data);
            reg.sr.z = ZERO<Long>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case BFEXTU:

            result = data >> (32 - width);

            reg.sr.n = NBIT<Long>(data);
            reg.sr.z = ZERO<Long>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case BFFFO:

            reg.sr.n = NBIT<Long>(data);
            data >>= 32 - width;
            reg.sr.z = ZERO<Long>(data);
            reg.sr.v = 0;
            reg.sr.c = 0;

            result = offset;
            for(u32 bit = 1 << (width - 1); bit && !(data & bit); bit >>= 1) {
                result++;
            }
            break;

        case BFTST:

            result = 0;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        default:
            fatalError;
    }

    return result;
}

template <Core C, Instr I> u32
Moira::bit(u32 op, u8 bit)
{
    switch (I) {

        case BCHG:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op ^= (1 << bit);
            break;

        case BSET:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op |= (1 << bit);
            break;

        case BCLR:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op &= ~(1 << bit);
            break;

        case BTST:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            break;

        default:
            fatalError;
    }
    return op;
}

bool
Moira::cond(Cond C) {

    switch (C) {

        case COND_BT: return true;
        case COND_BF: return false;
        case COND_HI: return !reg.sr.c && !reg.sr.z;
        case COND_LS: return reg.sr.c || reg.sr.z;
        case COND_CC: return !reg.sr.c;
        case COND_CS: return reg.sr.c;
        case COND_NE: return !reg.sr.z;
        case COND_EQ: return reg.sr.z;
        case COND_VC: return !reg.sr.v;
        case COND_VS: return reg.sr.v;
        case COND_PL: return !reg.sr.n;
        case COND_MI: return reg.sr.n;
        case COND_GE: return reg.sr.n == reg.sr.v;
        case COND_LT: return reg.sr.n != reg.sr.v;
        case COND_GT: return reg.sr.n == reg.sr.v && !reg.sr.z;
        case COND_LE: return reg.sr.n != reg.sr.v || reg.sr.z;

        default:
            fatalError;
    }
}

bool
Moira::cond(Instr I) {

    switch (I) {

        case BRA: case ST:  case DBT:  case DBT_LOOP:  case TRAPT:  return cond(COND_BT);
        case SF:            case DBF:  case DBF_LOOP:  case TRAPF:  return cond(COND_BF);
        case BHI: case SHI: case DBHI: case DBHI_LOOP: case TRAPHI: return cond(COND_HI);
        case BLS: case SLS: case DBLS: case DBLS_LOOP: case TRAPLS: return cond(COND_LS);
        case BCC: case SCC: case DBCC: case DBCC_LOOP: case TRAPCC: return cond(COND_CC);
        case BCS: case SCS: case DBCS: case DBCS_LOOP: case TRAPCS: return cond(COND_CS);
        case BNE: case SNE: case DBNE: case DBNE_LOOP: case TRAPNE: return cond(COND_NE);
        case BEQ: case SEQ: case DBEQ: case DBEQ_LOOP: case TRAPEQ: return cond(COND_EQ);
        case BVC: case SVC: case DBVC: case DBVC_LOOP: case TRAPVC: return cond(COND_VC);
        case BVS: case SVS: case DBVS: case DBVS_LOOP: case TRAPVS: return cond(COND_VS);
        case BPL: case SPL: case DBPL: case DBPL_LOOP: case TRAPPL: return cond(COND_PL);
        case BMI: case SMI: case DBMI: case DBMI_LOOP: case TRAPMI: return cond(COND_MI);
        case BGE: case SGE: case DBGE: case DBGE_LOOP: case TRAPGE: return cond(COND_GE);
        case BLT: case SLT: case DBLT: case DBLT_LOOP: case TRAPLT: return cond(COND_LT);
        case BGT: case SGT: case DBGT: case DBGT_LOOP: case TRAPGT: return cond(COND_GT);
        case BLE: case SLE: case DBLE: case DBLE_LOOP: case TRAPLE: return cond(COND_LE);

        default:
            fatalError;
    }
}

template <Core C, Instr I> int
Moira::cyclesBit(u8 bit)
{
    switch (I)
    {
        case BTST: return 2;
        case BCLR: return MIMIC_MUSASHI ? 6 : (bit > 15 ? 6 : 4);
        case BSET:
        case BCHG: return MIMIC_MUSASHI ? 4 : (bit > 15 ? 4 : 2);

        default:
            fatalError;
    }
}

template <Core C, Instr I> int
Moira::cyclesMul(u16 data)
{
    int mcycles = 17;

    switch (I)
    {
        case MULU:

            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;

        case MULS:

            data = ((data << 1) ^ data) & 0xFFFF;
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;

        default:
            fatalError;
    }
}

template <Core C, Instr I> int
Moira::cyclesDiv(u32 op1, u16 op2)
{
    int result;

    if constexpr (I == DIVU) {

        u32 dividend = op1;
        u16 divisor  = op2;
        int mcycles  = 38;

        if ((dividend >> 16) >= divisor) {

            // Quotient is larger than 16 bit
            result = 10;

        } else {

            u32 hdivisor = divisor << 16;

            for (int i = 0; i < 15; i++) {
                if ((i32)dividend < 0) {
                    dividend = (u32)((u64)dividend << 1);
                    dividend = U32_SUB(dividend, hdivisor);
                } else {
                    dividend = (u32)((u64)dividend << 1);
                    if (dividend >= hdivisor) {
                        dividend = U32_SUB(dividend, hdivisor);
                        mcycles += 1;
                    } else {
                        mcycles += 2;
                    }
                }
            }
            result = 2 * mcycles;
        }
    }

    if constexpr (I == DIVS) {

        i32 dividend = (i32)op1;
        i16 divisor  = (i16)op2;
        int mcycles  = (dividend < 0) ? 7 : 6;

        // Check if quotient is larger than 16 bit
        if ((abs(dividend) >> 16) >= abs(divisor)) {

            result = (mcycles + 2) * 2;

        } else {

            mcycles += 55;

            if (divisor >= 0) {
                mcycles += (dividend < 0) ? 1 : -1;
            }

            u32 aquot = abs(dividend) / abs(divisor);
            for (int i = 0; i < 15; i++) {
                if ( (i16)aquot >= 0) mcycles++;
                aquot <<= 1;
            }
            result = 2 * mcycles;
        }
    }

    return result;
}

template <Core C, Instr I> u32
Moira::mulMusashi(u32 op1, u32 op2)
{
    u32 result;

    if constexpr (I == MULS) result = (i16)op1 * (i16)op2;
    if constexpr (I == MULU) result = op1 * op2;

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Size S> u64
Moira::mullsMusashi(u32 op1, u32 op2)
{
    u64 result = u64(i64(i32(op1)) * i64(i32(op2)));

    if constexpr (S == Word) {

        reg.sr.n = NBIT<Long>(result);
        reg.sr.z = ZERO<Long>(result);
        reg.sr.v = result != u64(i32(result));
        reg.sr.c = 0;
    }

    if constexpr (S == Long) {

        reg.sr.n = NBIT<Long>(result >> 32);
        reg.sr.z = result == 0;
        reg.sr.v = 0;
        reg.sr.c = 0;
    }

    return result;
}

template <Size S> u64
Moira::mulluMusashi(u32 op1, u32 op2)
{
    u64 result = u64(op1) * u64(op2);

    if constexpr (S == Word) {

        reg.sr.n = NBIT<Long>(result);
        reg.sr.z = ZERO<Long>(result);
        reg.sr.v = (result >> 32) != 0;
        reg.sr.c = 0;
    }

    if constexpr (S == Long) {

        reg.sr.n = NBIT<Long>(result >> 32);
        reg.sr.z = result == 0;
        reg.sr.v = 0;
        reg.sr.c = 0;
    }

    return result;
}

template <Core C, Instr I> u32
Moira::divMusashi(u32 op1, u32 op2)
{
    u32 result;

    if constexpr (I == DIVS) {

        if (op1 == 0x80000000 && (i32)op2 == -1) {

            reg.sr.z = 0;
            reg.sr.n = 0;
            reg.sr.v = 0;
            reg.sr.c = 0;
            result = 0;

        } else {

            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            if (quotient == (i16)quotient) {

                result = (quotient & 0xffff) | (u16)remainder << 16;

                reg.sr.z = quotient;
                reg.sr.n = NBIT<Word>(quotient);
                reg.sr.v = 0;
                reg.sr.c = 0;

            } else {

                result = op1;
                reg.sr.v = 1;
            }
        }
    }

    if constexpr (I == DIVU) {

        i64 quotient  = op1 / op2;
        u16 remainder = (u16)(op1 % op2);

        if(quotient < 0x10000) {

            result = (quotient & 0xffff) | remainder << 16;

            reg.sr.z = quotient;
            reg.sr.n = NBIT<Word>(quotient);
            reg.sr.v = 0;
            reg.sr.c = 0;

        } else {

            result = op1;
            reg.sr.v = 1;
        }
    }

    return result;
}

template <Size S> std::pair<u32,u32>
Moira::divlsMusashi(u64 op1, u32 op2)
{
    u64 quotient, remainder;

    if constexpr (S == Word) {

        quotient  = (u64)((i64)((i32)op1) / (i64)((i32)op2));
        remainder = (u64)((i64)((i32)op1) % (i64)((i32)op2));

        reg.sr.n = NBIT<Long>(quotient);
        reg.sr.z = ZERO<Long>(quotient);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }

    if constexpr (S == Long) {

        quotient  = u64(i64(op1) / i64(i32(op2)));
        remainder = u64(i64(op1) % i64(i32(op2)));

        if (i64(quotient) == i64(i32(quotient))) {

            reg.sr.n = NBIT<Long>(quotient);
            reg.sr.z = ZERO<Long>(quotient);
            reg.sr.v = 0;
            reg.sr.c = 0;

        } else {

            reg.sr.v = 1;
        }
    }

    return { u32(quotient), u32(remainder) };
}

template <Size S> std::pair<u32,u32>
Moira::divluMusashi(u64 op1, u32 op2)
{
    u64 quotient, remainder;

    if (S == Word) {

        quotient  = op1 / op2;
        remainder = op1 % op2;

        reg.sr.n = NBIT<Long>(quotient);
        reg.sr.z = ZERO<Long>(quotient);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }

    if constexpr (S == Long) {

        quotient  = op1 / op2;
        remainder = op1 % op2;

        if (quotient <= 0xffffffff) {

            reg.sr.n = NBIT<Long>(quotient);
            reg.sr.z = ZERO<Long>(quotient);
            reg.sr.v = 0;
            reg.sr.c = 0;

        } else {

            reg.sr.v = 1;
        }
    }

    return { u32(quotient), u32(remainder) };
}
