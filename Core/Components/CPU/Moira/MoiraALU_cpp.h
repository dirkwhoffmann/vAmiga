// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

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

template <Cond C> bool
Moira::evalCond() {

    switch (C) {

        case Cond::BT: return true;
        case Cond::BF: return false;
        case Cond::HI: return !reg.sr.c && !reg.sr.z;
        case Cond::LS: return reg.sr.c || reg.sr.z;
        case Cond::CC: return !reg.sr.c;
        case Cond::CS: return reg.sr.c;
        case Cond::NE: return !reg.sr.z;
        case Cond::EQ: return reg.sr.z;
        case Cond::VC: return !reg.sr.v;
        case Cond::VS: return reg.sr.v;
        case Cond::PL: return !reg.sr.n;
        case Cond::MI: return reg.sr.n;
        case Cond::GE: return reg.sr.n == reg.sr.v;
        case Cond::LT: return reg.sr.n != reg.sr.v;
        case Cond::GT: return reg.sr.n == reg.sr.v && !reg.sr.z;
        case Cond::LE: return reg.sr.n != reg.sr.v || reg.sr.z;

        default:
            fatalError;
    }
}

template <Instr I> bool
Moira::cond() {

    switch (I) {

        case Instr::BRA: case Instr::ST:  case Instr::DBT:  case Instr::DBT_LOOP:  case Instr::TRAPT:  return evalCond<Cond::BT>();
        case Instr::SF:            case Instr::DBF:  case Instr::DBF_LOOP:  case Instr::TRAPF:  return evalCond<Cond::BF>();
        case Instr::BHI: case Instr::SHI: case Instr::DBHI: case Instr::DBHI_LOOP: case Instr::TRAPHI: return evalCond<Cond::HI>();
        case Instr::BLS: case Instr::SLS: case Instr::DBLS: case Instr::DBLS_LOOP: case Instr::TRAPLS: return evalCond<Cond::LS>();
        case Instr::BCC: case Instr::SCC: case Instr::DBCC: case Instr::DBCC_LOOP: case Instr::TRAPCC: return evalCond<Cond::CC>();
        case Instr::BCS: case Instr::SCS: case Instr::DBCS: case Instr::DBCS_LOOP: case Instr::TRAPCS: return evalCond<Cond::CS>();
        case Instr::BNE: case Instr::SNE: case Instr::DBNE: case Instr::DBNE_LOOP: case Instr::TRAPNE: return evalCond<Cond::NE>();
        case Instr::BEQ: case Instr::SEQ: case Instr::DBEQ: case Instr::DBEQ_LOOP: case Instr::TRAPEQ: return evalCond<Cond::EQ>();
        case Instr::BVC: case Instr::SVC: case Instr::DBVC: case Instr::DBVC_LOOP: case Instr::TRAPVC: return evalCond<Cond::VC>();
        case Instr::BVS: case Instr::SVS: case Instr::DBVS: case Instr::DBVS_LOOP: case Instr::TRAPVS: return evalCond<Cond::VS>();
        case Instr::BPL: case Instr::SPL: case Instr::DBPL: case Instr::DBPL_LOOP: case Instr::TRAPPL: return evalCond<Cond::PL>();
        case Instr::BMI: case Instr::SMI: case Instr::DBMI: case Instr::DBMI_LOOP: case Instr::TRAPMI: return evalCond<Cond::MI>();
        case Instr::BGE: case Instr::SGE: case Instr::DBGE: case Instr::DBGE_LOOP: case Instr::TRAPGE: return evalCond<Cond::GE>();
        case Instr::BLT: case Instr::SLT: case Instr::DBLT: case Instr::DBLT_LOOP: case Instr::TRAPLT: return evalCond<Cond::LT>();
        case Instr::BGT: case Instr::SGT: case Instr::DBGT: case Instr::DBGT_LOOP: case Instr::TRAPGT: return evalCond<Cond::GT>();
        case Instr::BLE: case Instr::SLE: case Instr::DBLE: case Instr::DBLE_LOOP: case Instr::TRAPLE: return evalCond<Cond::LE>();

        default:
            fatalError;
    }
}

template <Core C, Instr I, Size S> u32
Moira::shift(int cnt, u64 data) {

    switch (I) {

        case Instr::ASL:
        case Instr::ASL_LOOP:
        {
            bool carry = false;
            u32 changed = 0;

            for (int i = 0; i < cnt; i++) {

                carry = NBIT<S>(data);
                u64 shifted = (data & 0xFFFFFFFF) << 1;
                changed |= (u32)(data ^ shifted);
                data = shifted;
            }

            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = NBIT<S>(changed);
            break;
        }
        case Instr::ASR:
        case Instr::ASR_LOOP:
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
        case Instr::LSL:
        case Instr::LSL_LOOP:
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
        case Instr::LSR:
        case Instr::LSR_LOOP:
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
        case Instr::ROL:
        case Instr::ROL_LOOP:
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
        case Instr::ROR:
        case Instr::ROR_LOOP:
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
        case Instr::ROXL:
        case Instr::ROXL_LOOP:
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
        case Instr::ROXR:
        case Instr::ROXR_LOOP:
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
Moira::bcd(u32 op1, u32 op2)
{
    u64 tmp, result;

    // Extract digits
    u16 hi1 = op1 & 0xF0, lo1 = op1 & 0x0F;
    u16 hi2 = op2 & 0xF0, lo2 = op2 & 0x0F;

    switch (I) {

        case Instr::ABCD:
        case Instr::ABCD_LOOP:
        {
            // Add digits
            u16 lo = lo1 + lo2 + reg.sr.x;
            u16 hi = hi1 + hi2;
            result = tmp = hi + lo;

            // Rectify first digit
            if (lo > 9) {
                result += 0x06;
            }

            // Rectify second digit
            if ((result & 0x3F0) > 0x90) {
                result += 0x60;
                reg.sr.x = 1;
            } else {
                reg.sr.x = 0;
            }

            // Set V flag
            if constexpr (C != Core::C68020) {
                reg.sr.v = ((tmp & 0x80) == 0) && ((result & 0x80) == 0x80);
            } else {
                reg.sr.v = 0;
            }

            break;
        }
        case Instr::SBCD:
        case Instr::SBCD_LOOP:
        {
            // Subtract digits
            u16 lo = lo2 - lo1 - reg.sr.x;
            u16 hi = hi2 - hi1;
            result = tmp = hi + lo;

            // Rectify first digit
            if (lo & 0xF0) {
                result -= 0x06;
                reg.sr.x = ((op2 - op1 - 6 - reg.sr.x) & 0x300) > 0xFF;
            } else {
                reg.sr.x = ((op2 - op1 - reg.sr.x) & 0x300) > 0xFF;
            }

            // Rectify second digit
            if (((op2 - op1 - reg.sr.x) & 0x100) > 0xFF) {
                result -= 0x60;
            }

            // Set V flag
            if constexpr (C != Core::C68020) {
                reg.sr.v = ((tmp & 0x80) == 0x80) && ((result & 0x80) == 0);
            } else {
                reg.sr.v = 0;
            }

            break;
        }

        default:
            fatalError;
    }

    // Set other flags
    reg.sr.c = reg.sr.x;
    reg.sr.n = NBIT<S>(result);
    if (CLIP<Byte>(result)) reg.sr.z = 0;

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

        case Instr::NOT:
        case Instr::NOT_LOOP:

            result = ~op;
            reg.sr.n = NBIT<S>(result);
            reg.sr.z = ZERO<S>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::NEG:
        case Instr::NEG_LOOP:

            result = addsub<C, Instr::SUB, S>(op, 0);
            break;

        case Instr::NEGX:
        case Instr::NEGX_LOOP:

            result = addsub<C, Instr::SUBX, S>(op, 0);
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

        case Instr::AND: case Instr::ANDI: case Instr::ANDICCR: case Instr::ANDISR: case Instr::AND_LOOP:

            result = op1 & op2;
            break;

        case Instr::OR: case Instr::ORI: case Instr::ORICCR: case Instr::ORISR: case Instr::OR_LOOP:

            result = op1 | op2;
            break;

        case Instr::EOR: case Instr::EORI: case Instr::EORICCR: case Instr::EORISR: case Instr::EOR_LOOP:

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

template <Core C, Instr I> u32
Moira::bit(u32 op, u8 bit)
{
    switch (I) {

        case Instr::BCHG:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op ^= (1 << bit);
            break;

        case Instr::BSET:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op |= (1 << bit);
            break;

        case Instr::BCLR:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op &= ~(1 << bit);
            break;

        case Instr::BTST:

            reg.sr.z = 1 ^ ((op >> bit) & 1);
            break;

        default:
            fatalError;
    }
    return op;
}

template <Instr I> u32
Moira::bitfield(u32 data, u32 offset, u32 width, u32 mask)
{
    u32 result = 0;

    switch (I) {

        case Instr::BFCHG:

            result = data ^ mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::BFCLR:

            result = data & ~mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::BFSET:

            result = data | mask;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::BFEXTS:

            result = SEXT<Long>(data) >> (32 - width);

            reg.sr.n = NBIT<Long>(data);
            reg.sr.z = ZERO<Long>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::BFEXTU:

            result = data >> (32 - width);

            reg.sr.n = NBIT<Long>(data);
            reg.sr.z = ZERO<Long>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        case Instr::BFFFO:

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

        case Instr::BFTST:

            result = 0;

            reg.sr.n = NBIT<Long>(data << offset);
            reg.sr.z = ZERO<Long>(data & mask);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;

        default:
            break;
    }

    return result;
}

template <Core C, Instr I, Size S> u32
Moira::addsub(u32 op1, u32 op2)
{
    u64 result;

    switch (I) {

        case Instr::ADD:
        case Instr::ADD_LOOP:
        case Instr::ADDI:
        case Instr::ADDQ:
        {
            result = U64_ADD(op1, op2);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case Instr::ADDX:
        case Instr::ADDX_LOOP:
        {
            result = U64_ADD3(op1, op2, reg.sr.x);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            if (CLIP<S>(result)) reg.sr.z = 0;
            break;
        }
        case Instr::SUB:
        case Instr::SUB_LOOP:
        case Instr::SUBI:
        case Instr::SUBQ:
        {
            result = U64_SUB(op2, op1);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case Instr::SUBX:
        case Instr::SUBX_LOOP:
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

template <Core C> u32
Moira::muls(u32 op1, u32 op2)
{
    u32 result = (i16)op1 * (i16)op2;

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Core C> u32
Moira::mulu(u32 op1, u32 op2)
{
    u32 result = op1 * op2;

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Core C, Size S> u64
Moira::mulls(u32 op1, u32 op2)
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

template <Core C, Size S> u64
Moira::mullu(u32 op1, u32 op2)
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

template <Core C> u32
Moira::divsMoira(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

    i64 quotient  = (i64)(i32)op1 / (i16)op2;
    i16 remainder = (i64)(i32)op1 % (i16)op2;

    result = (u32)((quotient & 0xffff) | remainder << 16);
    overflow = ((quotient & 0xffff8000) != 0 &&
                (quotient & 0xffff8000) != 0xffff8000);
    overflow |= op1 == 0x80000000 && (i16)op2 == -1;
    reg.sr.v = overflow;

    if (overflow) {

        setUndefinedDIVS<C, Word>(i32(op1), i16(op2));

    } else {

        reg.sr.c = 0;
        reg.sr.n = NBIT<Word>(result);
        reg.sr.z = ZERO<Word>(result);
    }

    return overflow ? op1 : result;
}

template <Core C> u32
Moira::divuMoira(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

    i64 quotient  = op1 / op2;
    u16 remainder = (u16)(op1 % op2);

    result = (u32)((quotient & 0xffff) | remainder << 16);
    overflow = quotient > 0xFFFF;
    reg.sr.v = overflow;

    if (overflow) {

        setUndefinedDIVU<C, Word>(i32(op1), i16(op2));

    } else {

        reg.sr.c = 0;
        reg.sr.n = NBIT<Word>(result);
        reg.sr.z = ZERO<Word>(result);
    }

    return overflow ? op1 : result;
}

template <Core C> u32
Moira::divsMusashi(u32 op1, u32 op2)
{
    u32 result;

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

    return result;
}

template <Core C> u32
Moira::divuMusashi(u32 op1, u32 op2)
{
    u32 result;
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

    return result;
}

template <Size S> std::pair<u32,u32>
Moira::divlsMoira(i64 a, u32 src)
{
    i64 quotient, remainder;

    if constexpr (S == Word) { a = (i64)(i32)a; }

    if ((u64)a == 0x8000000000000000UL && (i32)src == -1) {

        assert(S == Word);
        reg.sr.v = 1;
        return { u32(0), u32(0) };

    } else {

        remainder = a % (i64)(i32)src;
        quotient = a / (i64)(i32)src;

        if ((quotient & u64(0xffffffff80000000)) != 0
            && (quotient & u64(0xffffffff80000000)) != u64(0xffffffff80000000)) {

            assert(S != Word);
            reg.sr.v = 1;
            return { u32(0), u32(0) };

        } else {

            if (((i32)remainder < 0) != ((i64)a < 0)) {
                remainder = -remainder;
            }

            reg.sr.v = 0;
            reg.sr.c = 0;
            reg.sr.z = (i32)quotient == 0;
            reg.sr.n = (i32)quotient < 0;

            return { u32(quotient), u32(remainder) };
        }
    }
}

template <Size S> std::pair<u32,u32>
Moira::divluMoira(u64 a, u32 src)
{
    u64 quotient, remainder;

    remainder = a % (u64)src;
    quotient = a / (u64)src;
    if (quotient > 0xffffffffu) {

        reg.sr.v = 1;
        return { u32(0), u32(0) };

    } else {

        reg.sr.v = 0;
        reg.sr.c = 0;
        reg.sr.z = (i32)quotient == 0;
        reg.sr.n = (i32)quotient < 0;

        return { u32(quotient), u32(remainder) };
    }
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

template <Core C, Instr I> int
Moira::cyclesBit(u8 bit) const
{
    switch (I)
    {
        case Instr::BTST: return 2;
        case Instr::BCLR: return MOIRA_MIMIC_MUSASHI ? 6 : (bit > 15 ? 6 : 4);
        case Instr::BSET:
        case Instr::BCHG: return MOIRA_MIMIC_MUSASHI ? 4 : (bit > 15 ? 4 : 2);

        default:
            fatalError;
    }
}

template <Core C, Instr I> int
Moira::cyclesMul(u16 data) const
{
    int mcycles = 0;

    if constexpr (C == Core::C68000 && I == Instr::MULU) {

        mcycles = 17;
        for (; data; data >>= 1) if (data & 1) mcycles++;
        mcycles *= 2;
    }

    if constexpr (C == Core::C68000 && I == Instr::MULS) {

        mcycles = 17;
        data = ((data << 1) ^ data) & 0xFFFF;
        for (; data; data >>= 1) if (data & 1) mcycles++;
        mcycles *= 2;
    }

    if constexpr (C == Core::C68010 && I == Instr::MULU) {

        mcycles = 36;
    }

    if constexpr (C == Core::C68010 && I == Instr::MULS) {

        mcycles = (data & 0x8000) ? 38 : 36;
    }

    return mcycles;
}

template <Core C, Instr I> int
Moira::cyclesDiv(u32 op1, u16 op2) const
{
    int result = 0;

    if constexpr (C == Core::C68000 && I == Instr::DIVU) {

        u32 dividend = op1;
        u16 divisor  = op2;
        int mcycles  = 38;

        // Check if quotient is larger than 16 bit
        if ((dividend >> 16) >= divisor) {

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

    if constexpr (C == Core::C68000 && I == Instr::DIVS) {

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

    if constexpr (C == Core::C68010 && I == Instr::DIVU) {

        u32 dividend = op1;
        u16 divisor  = op2;
        int mcycles  = 78;

        // Check if quotient is larger than 16 bit
        if ((dividend >> 16) >= divisor) {

            result = 8;

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
                        mcycles += 2;
                    } else {
                        mcycles += 2;
                    }
                }
            }
            result = mcycles;
        }
    }

    if constexpr (C == Core::C68010 && I == Instr::DIVS) {

        i32 dividend = (i32)op1;
        i16 divisor  = (i16)op2;
        int mcycles  = (dividend < 0) ? 120 : 118;

        // Check if quotient is larger than 16 bit
        if ((abs(dividend) >> 16) >= abs(divisor)) {

            result = 16;

        } else {

            result = mcycles;
        }
    }

    return result;
}

template <Core C, Size S> void
Moira::setUndefinedCHK(i32 src, i32 dst)
{
    switch (C) {

        case Core::C68000:
        case Core::C68010:

            reg.sr.c = 0;
            reg.sr.z = dst == 0 ? 1 : 0;
            reg.sr.v = 0;
            break;

        case Core::C68020:

            reg.sr.c = reg.sr.z = reg.sr.n = reg.sr.v = 0;

            if (dst == 0) reg.sr.z = 1;
            if (dst < 0 || dst > src) {

                if constexpr (S == Word) {
                    int flgs = i16(dst) < 0;
                    int flgo = i16(src) < 0;
                    i16 val = i16(i64(src) - i64(dst));
                    int flgn = val < 0;
                    reg.sr.v = (flgs ^ flgo) & (flgn ^ flgo);
                } else {
                    int flgs = dst < 0;
                    int flgo = src < 0;
                    i32 val = i32(i64(src) - i64(dst));
                    int flgn = val < 0;
                    reg.sr.v = (flgs ^ flgo) & (flgn ^ flgo);
                }
                if (dst < 0) {
                    reg.sr.c = dst > src || src >= 0;
                } else {
                    reg.sr.c = src >= 0;
                }
            }
            break;
    }
}

template <Core C, Size S> void
Moira::setUndefinedCHK2(i32 lower, i32 upper, i32 value)
{
    auto diff = [&](i32 arg1, i32 arg2) {
        return i32((i64)arg1 - (i64)arg2);
    };

    // Logic taken from UAE
    reg.sr.n = reg.sr.v = 0;

    if (value == lower || value == upper) return;

    if (lower < 0 && upper >= 0) {
        if (value < lower) {
            reg.sr.n = 1;
        }
        if (value >= 0 && value < upper) {
            reg.sr.n = 1;
        }
        // assert((lower - value >= 0) == i32((u32)lower - (u32)value) >= 0);
        if (value >= 0 && diff(lower, value) >= 0) {
            // if (value >= 0 && lower - value >= 0) {
            reg.sr.v = 1;
            reg.sr.n = 0;
            if (value > upper) {
                reg.sr.n = 1;
            }
        }
    } else if (lower >= 0 && upper < 0) {
        if (value >= 0) {
            reg.sr.n = 1;
        }
        if (value > upper) {
            reg.sr.n = 1;
        }
        if (value > lower && diff(upper, value) >= 0) {
            reg.sr.v = 1;
            reg.sr.n = 0;
        }
    } else if (lower >= 0 && upper >= 0 && lower > upper) {
        if (value > upper && value < lower) {
            reg.sr.n = 1;
        }
        if (value < 0 && diff(lower, value) < 0) {
            reg.sr.v = 1;
        }
        if (value < 0 && diff(lower, value) >= 0) {
            reg.sr.n = 1;
        }
    } else if (lower >= 0 && upper >= 0 && lower <= upper) {
        if (value >= 0 && value < lower) {
            reg.sr.n = 1;
        }
        if (value > upper) {
            reg.sr.n = 1;
        }
        if (value < 0 && diff(upper, value) < 0) {
            reg.sr.n = 1;
            reg.sr.v = 1;
        }
    } else if (lower < 0 && upper < 0 && lower > upper) {
        if (value >= 0) {
            reg.sr.n = 1;
        }
        if (value > upper && value < lower) {
            reg.sr.n = 1;
        }
        if (value >= 0 && diff(value, lower) < 0) {
            reg.sr.n = 0;
            reg.sr.v = 1;
        }
    } else if (lower < 0 && upper < 0 && lower <= upper) {
        if (value < lower) {
            reg.sr.n = 1;
        }
        if (value < 0 && value > upper) {
            reg.sr.n = 1;
        }
        if (value >= 0 && diff(value, lower) < 0) {
            reg.sr.n = 1;
            reg.sr.v = 1;
        }
    }
}

template <Core C, Size S> void
Moira::setUndefinedDIVU(u32 dividend, u16 divisor)
{
    auto iDividend = i32(dividend);

    switch (C) {

        case Core::C68000:
        case Core::C68010:

            reg.sr.c = 0;
            reg.sr.n = 1;
            reg.sr.z = 0;
            break;

        case Core::C68020:

            if (iDividend < 0) reg.sr.n = 1;
            break;
    }
}

template <Core C, Size S> void
Moira::setUndefinedDIVS(i32 dividend, i16 divisor)
{
    auto uDividend = u32(std::abs(dividend));
    auto uDivisor = u16(std::abs(divisor));

    switch (C) {

        case Core::C68000:
        case Core::C68010:

            reg.sr.c = 0;
            reg.sr.n = 1;
            reg.sr.z = 0;
            break;

        case Core::C68020:

            reg.sr.c = 0;
            reg.sr.n = 0;
            reg.sr.z = 0;

            if ((uDividend >> 16) < uDivisor) {

                u32 quot = uDividend / uDivisor;
                if (i8(quot) == 0) reg.sr.z = 1;
                if (i8(quot) < 0) reg.sr.n = 1;
            }
    }
}

template <Core C, Size S> void
Moira::setUndefinedDIVUL(i64 a, i32 divisor)
{
    i32 a32 = (i32)a;
    bool neg32 = a32 < 0;

    reg.sr.n = neg32;
    reg.sr.z = a32 == 0;
    reg.sr.c = 0;
}

template <Core C, Size S> void
Moira::setUndefinedDIVSL(i64 a, i32 divisor)
{
    i32 a32 = (i32)a;
    bool neg64 = a < 0;
    bool neg32 = a32 < 0;

    if constexpr (S == Long) {

        i32 ahigh = a >> 32;
        if (ahigh == 0) {
            reg.sr.z = 1;
            reg.sr.n = 0;
        } else if (ahigh < 0 && divisor < 0 && ahigh > divisor) {
            reg.sr.z = 0;
            reg.sr.n = 0;
        } else {
            if (a32 == 0) {
                reg.sr.z = 1;
                reg.sr.n = 0;
            } else {
                reg.sr.z = 0;
                reg.sr.n = neg32 ^ neg64;
            }
        }
    } else {
        if (a32 == 0) {
            reg.sr.z = 1;
            reg.sr.n = 0;
        } else {
            reg.sr.n = neg32;
            reg.sr.z = 0;
        }
    }
    reg.sr.c = 0;
}

template <Core C, Size S> void
Moira::setDivZeroDIVU(u32 dividend)
{
    switch (C) {

        case Core::C68000:
        case Core::C68010:
        {
            reg.sr.n = 0;
            reg.sr.z = 0;

            i16 d = i16(dividend >> 16);
            if (d < 0) reg.sr.n = 1;
            if (d == 0) reg.sr.z = 1;
            break;
        }
        case Core::C68020:
        {
            reg.sr.n = 0;
            reg.sr.z = 0;
            reg.sr.v = 1;

            i16 d = i16(dividend >> 16);
            if (d < 0) reg.sr.n = 1;
            if (d == 0) reg.sr.z = 1;
            break;
        }
    }
}

template <Core C, Size S> void
Moira::setDivZeroDIVS(u32 dividend)
{
    switch (C) {

        case Core::C68000:
        case Core::C68010:

            reg.sr.n = 0;
            reg.sr.z = 1;
            break;

        case Core::C68020:

            reg.sr.n = 0;
            reg.sr.z = 1;
            break;
    }
}

template <Core C, Size S> void
Moira::setDivZeroDIVUL(i64 dividend)
{
    i32 a32 = (i32)dividend;
    bool neg32 = a32 < 0;
    reg.sr.n = neg32;
    reg.sr.z = a32 == 0;
}

template <Core C, Size S> void
Moira::setDivZeroDIVSL(i64 dividend)
{
    reg.sr.n = 0;
    reg.sr.z = 1;
}
