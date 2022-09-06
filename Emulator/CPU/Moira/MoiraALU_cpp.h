// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
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
                u64 shifted = (data & 0xFFFFFFFF) << 1;
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

/*
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
    
    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;
    
    return result;
}
*/

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

template <Core C, Instr I> u32
Moira::div(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

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

    reg.sr.v = overflow;

    if (I == DIVS) {

        if (overflow) {

            setUndefinedDIVS<C, Word>(i32(op1), i16(op2));

        } else {

            reg.sr.c = 0;
            reg.sr.n = NBIT<Word>(result);
            reg.sr.z = ZERO<Word>(result);
        }
    }
    if (I == DIVU) {

        if (overflow) {

            setUndefinedDIVU<C, Word>(i32(op1), i16(op2));

        } else {

            reg.sr.c = 0;
            reg.sr.n = NBIT<Word>(result);
            reg.sr.z = ZERO<Word>(result);
        }
    }

    return overflow ? op1 : result;
}

template <Core C, Instr I, Size S> u32
Moira::bcd(u32 op1, u32 op2)
{
    u64 tmp, result;

    // Extract digits
    u16 hi1 = op1 & 0xF0, lo1 = op1 & 0x0F;
    u16 hi2 = op2 & 0xF0, lo2 = op2 & 0x0F;

    switch (I) {
            
        case ABCD:
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
            if constexpr (C != C68020) {
                reg.sr.v = ((tmp & 0x80) == 0) && ((result & 0x80) == 0x80);
            } else {
                reg.sr.v = 0;
            }

            break;
        }
        case SBCD:
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
            if constexpr (C != C68020) {
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
            
        case AND: case ANDI: case ANDICCR: case ANDISR: case AND_LOOP:

            result = op1 & op2;
            break;
            
        case OR: case ORI: case ORICCR: case ORISR: case OR_LOOP:
            
            result = op1 | op2;
            break;
            
        case EOR: case EORI: case EORICCR: case EORISR: case EOR_LOOP:
            
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
    u32 result = 0;
    
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
            break;
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
Moira::cyclesBit(u8 bit) const
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
Moira::cyclesMul(u16 data) const
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
Moira::cyclesDiv(u32 op1, u16 op2) const
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

template <Size S> std::pair<u32,u32>
Moira::divlsMoira(i64 a, u32 src)
{
    i64 quotient, remainder;

    // TODO: CLEAN THIS UP
    if (S == Word) { a = (i64)(i32)a; }

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

template <Core C, Instr I, Size S> void
Moira::setUndefinedFlags(i32 arg1, i32 arg2, i32 arg3)
{
    switch (I) {

        case CHK:
        {
            i32 src = arg1;
            i32 dst = arg2;

            switch (C) {

                case C68000:
                case C68010:

                    reg.sr.c = 0;
                    reg.sr.z = dst == 0 ? 1 : 0;
                    reg.sr.v = 0;
                    break;

                case C68020:

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
            break;
        }
        case CHK2:
        {
            auto diff = [&](i32 arg1, i32 arg2) {
                return i32((i64)arg1 - (i64)arg2);
            };

            i32 lower = arg1;
            i32 upper = arg2;
            i32 value = arg3;

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
            break;
        }
        default:
            fatalError;
    }
}

template <Core C, Size S> void
Moira::setUndefinedDIVU(u32 dividend, u16 divisor)
{
    auto iDividend = i32(dividend);

    switch (C) {

        case C68000:
        case C68010:

            reg.sr.c = 0;
            reg.sr.n = 1;
            reg.sr.z = 0;
            break;

        case C68020:

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

        case C68000:
        case C68010:

            reg.sr.c = 0;
            reg.sr.n = 1;
            reg.sr.z = 0;
            break;

        case C68020:

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

        case C68000:
        case C68010:
        {
            reg.sr.n = 0;
            reg.sr.z = 0;

            i16 d = i16(dividend >> 16);
            if (d < 0) reg.sr.n = 1;
            if (d == 0) reg.sr.z = 1;
            break;
        }
        case C68020:
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

        case C68000:
        case C68010:

            reg.sr.n = 0;
            reg.sr.z = 1;
            break;

        case C68020:

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
