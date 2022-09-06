// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include <cmath>

static const char *mnemonics[]
{
    // 68000
    "abcd" ,    "add",      "adda",     "addi",     "addq",     "addx",
    "and",      "andi",     "andi",     "andi",     "asl",      "asr",
    "bcc",      "bcs",      "beq",      "bge",      "bgt",      "bhi",
    "ble",      "bls",      "blt",      "bmi",      "bne",      "bpl",
    "bvc",      "bvs",      "bchg",     "bclr",     "bra",      "bset",
    "bsr",      "btst",     "chk",      "clr",      "cmp",     "cmpa",
    "cmpi",     "cmpm",     "dbcc",    "dbcs",      "dbeq",     "dbge",
    "dbgt",     "dbhi",     "dble",     "dbls",     "dblt",     "dbmi",
    "dbne",     "dbpl",     "dbvc",     "dbvs",     "dbra",     "dbt",
    "divs",     "divu",     "eor",      "eori",     "eori",     "eori",
    "exg",      "ext",      "???",      "jmp",      "jsr",      "lea",
    "???",      "???",      "link",     "lsl",      "lsr",      "move",
    "movea",    "move",     "move",     "move",     "move",     "move",
    "movem",    "movep",    "moveq",    "muls",     "mulu",     "nbcd",
    "neg",      "negx",     "nop",      "not",      "or",       "ori",
    "ori",      "ori",      "pea",      "reset",    "rol",      "ror",
    "roxl",     "roxr",     "rte",      "rtr",      "rts",      "sbcd",
    "scc",      "scs",      "seq",      "sge",      "sgt",      "shi",
    "sle",      "sls",      "slt",      "smi",      "sne",      "spl",
    "svc",      "svs",      "sf",       "st",       "stop",     "sub",
    "suba",     "subi",     "subq",     "subx",     "swap",     "tas",
    "trap",     "trapv",    "tst",      "unlk",
    
    // 68010
    "bkpt",     "movec",    "moves",    "rtd",
    
    // 68020
    "bfchg",    "bfclr",    "bfexts",   "bfextu",   "bfffo",    "bfins",
    "bfset",    "bftst",    "callm",    "cas",      "cas2",     "chk2",
    "cmp2",     "b",        "db",       "gen",      "restore",  "save",
    "s",        "trap",     "divl",     "extb",     "mull",     "pack",
    "rtm",      "trapcc",   "trapcs",   "trapeq",   "trapge",   "trapgt",
    "traphi",   "traple",   "trapls",   "traplt",   "trapmi",   "trapne",
    "trappl",   "trapvc",   "trapvs",   "trapf",    "trapt",    "unpk",

    // 68040
    "cinv",     "cpush",    "move16",

    // MMU
    "pflush",   "pflusha",  "pflushan", "pflushn",  "pload",    "pmove",    "ptest",

    // FPU
    "fabs",     "fadd",     "fb",       "fcmp",     "fdb",      "fdiv",
    "fmove",    "fmovem",   "fmul",     "fneg",     "fnop",     "frestore",
    "fsave",    "fs",       "fsqrt",    "fsub",     "ftrap",    "ftst",

    "fsabs",    "fdabs",    "fsadd",    "fdadd",    "fsdiv",    "fddiv",
    "fsmove",   "fdmove",   "fsmul",    "fdmul",    "fsneg",    "fdneg",
    "fssqrt",   "fdsqrt",   "fssub",    "fdsub",

    "facos",    "fasin",    "fatan",    "fatanh",   "fcos",     "fcosh",
    "fetox",    "fetoxm1",  "fgetexp",  "fgetman",  "fint",     "fintrz",
    "flog10",   "flog2",    "flogn",    "flognp1",  "fmod",     "fmovecr",
    "frem",     "fscale",   "fsgldiv",  "fsglmul",  "fsin",     "fsincos",
    "fsinh",    "ftan",     "ftanh",    "ftentox",  "ftwotox"
};

static int decDigits(u64 value) { return value ? 1 + (int)log10(value) : 1; }
static int binDigits(u64 value) { return value ? 1 + (int)log2(value) : 1; }
static int hexDigits(u64 value) { return (binDigits(value) + 3) / 4; }

static void sprintd(char *&s, u64 value, int digits)
{
    for (int i = digits - 1; i >= 0; i--) {
        
        u8 digit = value % 10;
        s[i] = '0' + digit;
        value /= 10;
    }
    s += digits;
}

static void sprintd(char *&s, u64 value)
{
    sprintd(s, value, decDigits(value));
}

static void sprintd_signed(char *&s, i64 value)
{
    if (value < 0) { *s++ = '-'; value *= -1; }
    sprintd(s, value, decDigits(value));
}

static void sprintx(char *&s, u64 value, const DasmNumberFormat &fmt, int digits)
{
    char a = (fmt.upperCase ? 'A' : 'a') - 10;
    
    if (value || !fmt.plainZero) {
        
        for (int i = 0; fmt.prefix[i]; i++) *s++ = fmt.prefix[i];
    }
    for (int i = digits - 1; i >= 0; i--) {
        
        auto digit = char(value % 16);
        s[i] = (digit <= 9) ? ('0' + digit) : (a + digit);
        value /= 16;
    }
    s += digits;
}

static void sprintx(char *&s, u64 value, const DasmNumberFormat &fmt)
{
    sprintx(s, value, fmt, hexDigits(value));
}

static void sprintx_signed(char *&s, i64 value, const DasmNumberFormat &fmt)
{
    if (value < 0) { *s++ = '-'; value *= -1; }
    sprintx(s, value, fmt, hexDigits(value));
}

static void sprint(char *&s, u64 value, const DasmNumberFormat &fmt)
{
    fmt.radix == 10 ? sprintd(s, value) : sprintx(s, value, fmt);
}

static void sprint_signed(char *&s, i64 value, const DasmNumberFormat &fmt)
{
    fmt.radix == 10 ? sprintd_signed(s, value) : sprintx_signed(s, value, fmt);
}

StrWriter&
StrWriter::operator<<(char c)
{
    *ptr++ = c;
    return *this;
}

StrWriter&
StrWriter::operator<<(const char *str)
{
    while (*str) { *ptr++ = *str++; };
    return *this;
}

StrWriter&
StrWriter::operator<<(int value)
{
    sprintd_signed(ptr, value);
    return *this;
}

StrWriter&
StrWriter::operator<<(unsigned int value)
{
    sprintd(ptr, value);
    return *this;
}

StrWriter&
StrWriter::operator<<(Int i)
{
    sprint_signed(ptr, i.raw, nf);
    return *this;
}

StrWriter&
StrWriter::operator<<(UInt u)
{
    sprint(ptr, u.raw, nf);
    return *this;
}

StrWriter&
StrWriter::operator<<(UInt8 u)
{
    if (nf.radix == 16) {
        sprintx(ptr, u.raw, nf, 2);
    } else {
        sprintd(ptr, u.raw, 3);
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(UInt16 u)
{
    if (nf.radix == 16) {
        sprintx(ptr, u.raw, nf, 4);
    } else {
        sprintd(ptr, u.raw, 5);
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(UInt32 u)
{
    if (nf.radix == 16) {
        sprintx(ptr, u.raw, nf, 8);
    } else {
        sprintd(ptr, u.raw, 10);
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Imu im)
{
    *ptr++ = '#';
    *this << UInt(im.raw);
    return *this;
}

template <Size S> StrWriter&
StrWriter::operator<<(Ims<S> im)
{
    *ptr++ = '#';
    *this << Int(SEXT<S>(im.raw));
    return *this;
}

StrWriter&
StrWriter::operator<<(Imd im)
{
    *ptr++ = '#';
    sprintd(ptr, im.raw);
    return *this;
}

template <Instr I> StrWriter&
StrWriter::operator<<(Ins<I> i)
{
    if constexpr (I == DBF) {
        
        if (style == DASM_GNU || style == DASM_GNU_MIT) {
            *this << "dbf";
        } else {
            *this << "dbra";
        }
        
    } else {
        
        *this << mnemonics[I];
    }
    
    return *this;
}

template <Size S> StrWriter&
StrWriter::operator<<(Sz<S>)
{
    switch (style) {

        case DASM_MOIRA_MIT:
        case DASM_GNU:
        case DASM_GNU_MIT:

            *this << ((S == Byte) ? "b" : (S == Word) ? "w" : "l");
            break;

        default:

            *this << ((S == Byte) ? ".b" : (S == Word) ? ".w" : ".l");
    }

    return *this;
}

template <Size S> StrWriter&
StrWriter::operator<<(Szb<S>)
{
    switch (style) {

        case DASM_MOIRA_MOT:

            *ptr++ = '.';
            [[fallthrough]];

        case DASM_MOIRA_MIT:
        case DASM_GNU:
        case DASM_GNU_MIT:

            if constexpr (S == Byte) *ptr++ = 's';
            if constexpr (S == Word) *ptr++ = 'w';
            if constexpr (S == Long) *ptr++ = 'l';
            break;

        default:
            break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Cc cc)
{
    switch (Cond(cc.raw)) {
            
        case COND_BT: *this << "t";   break;
        case COND_BF: *this << "f";   break;
        case COND_HI: *this << "hi";  break;
        case COND_LS: *this << "ls";  break;
        case COND_CC: *this << "cc";  break;
        case COND_CS: *this << "cs";  break;
        case COND_NE: *this << "ne";  break;
        case COND_EQ: *this << "eq";  break;
        case COND_VC: *this << "vc";  break;
        case COND_VS: *this << "vs";  break;
        case COND_PL: *this << "pl";  break;
        case COND_MI: *this << "mi";  break;
        case COND_GE: *this << "ge";  break;
        case COND_LT: *this << "lt";  break;
        case COND_GT: *this << "gt";  break;
        case COND_LE: *this << "le";  break;
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Cpcc cpcc)
{
    switch (cpcc.raw) {

        case 0:  *this << "f";      break;
        case 1:  *this << "eq";     break;
        case 2:  *this << "ogt";    break;
        case 3:  *this << "oge";    break;
        case 4:  *this << "olt";    break;
        case 5:  *this << "ole";    break;
        case 6:  *this << "ogl";    break;
        case 7:  *this << "or";     break;
        case 8:  *this << "un";     break;
        case 9:  *this << "ueq";    break;
        case 10: *this << "ugt";    break;
        case 11: *this << "uge";    break;
        case 12: *this << "ult";    break;
        case 13: *this << "ule";    break;
        case 14: *this << "ne";     break;
        case 15: *this << "t";      break;
        case 16: *this << "sf";     break;
        case 17: *this << "seq";    break;
        case 18: *this << "gt";     break;
        case 19: *this << "ge";     break;
        case 20: *this << "lt";     break;
        case 21: *this << "le";     break;
        case 22: *this << "gl";     break;
        case 23: *this << "gle";    break;
        case 24: *this << "ngle";   break;
        case 25: *this << "ngl";    break;
        case 26: *this << "nle";    break;
        case 27: *this << "nlt";    break;
        case 28: *this << "nge";    break;
        case 29: *this << "ngt";    break;
        case 30: *this << "sne";    break;
        case 31: *this << "st";     break;
            
        default:
            *this << "?";
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Fcc fcc)
{
    switch (fcc.raw & 0x1F) {

        case 0:  *this << "f";      break;
        case 1:  *this << "eq";     break;
        case 2:  *this << "ogt";    break;
        case 3:  *this << "oge";    break;
        case 4:  *this << "olt";    break;
        case 5:  *this << "ole";    break;
        case 6:  *this << "ogl";    break;
        case 7:  *this << "or";     break;
        case 8:  *this << "un";     break;
        case 9:  *this << "ueq";    break;
        case 10: *this << "ugt";    break;
        case 11: *this << "uge";    break;
        case 12: *this << "ult";    break;
        case 13: *this << "ule";    break;
        case 14: *this << "ne";     break;
        case 15: *this << "t";      break;
        case 16: *this << "sf";     break;
        case 17: *this << "seq";    break;
        case 18: *this << "gt";     break;
        case 19: *this << "ge";     break;
        case 20: *this << "lt";     break;
        case 21: *this << "le";     break;
        case 22: *this << "gl";     break;
        case 23: *this << "gle";    break;
        case 24: *this << "ngle";   break;
        case 25: *this << "ngl";    break;
        case 26: *this << "nle";    break;
        case 27: *this << "nlt";    break;
        case 28: *this << "nge";    break;
        case 29: *this << "ngt";    break;
        case 30: *this << "sne";    break;
        case 31: *this << "st";     break;
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Pcc pcc)
{
    switch (pcc.raw) {

        case 0:  *this << "bs"; break;
        case 1:  *this << "bc"; break;
        case 2:  *this << "ls"; break;
        case 3:  *this << "lc"; break;
        case 4:  *this << "ss"; break;
        case 5:  *this << "sc"; break;
        case 6:  *this << "as"; break;
        case 7:  *this << "ac"; break;
        case 8:  *this << "ws"; break;
        case 9:  *this << "wc"; break;
        case 10: *this << "is"; break;
        case 11: *this << "ic"; break;
        case 12: *this << "gs"; break;
        case 13: *this << "gc"; break;
        case 14: *this << "cs"; break;
        case 15: *this << "cc"; break;

        default:
            *this << "?";
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Dn dn)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'd'; *ptr++ = '0' + (char)dn.raw; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'D'; *ptr++ = '0' + (char)dn.raw; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(An an)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'a'; *ptr++ = '0' + (char)an.raw; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'A'; *ptr++ = '0' + (char)an.raw; break;
    }

    switch (style) {

        case DASM_GNU_MIT:

            if (an.raw == 6) { ptr[-2] = 'f'; ptr[-1] = 'p'; }
            [[fallthrough]];

        case DASM_GNU:

            if (an.raw == 7) { ptr[-2] = 's'; ptr[-1] = 'p'; }
            break;

        default:
            break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Rn rn)
{
    rn.raw < 8 ? *this << Dn{rn.raw} : *this << An{rn.raw - 8};
    return *this;
}

StrWriter&
StrWriter::operator<<(Ccr _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'c'; *ptr++ = 'c'; *ptr++ = 'r'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'C'; *ptr++ = 'C'; *ptr++ = 'R'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Pc _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'p'; *ptr++ = 'c'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'P'; *ptr++ = 'C'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Zpc _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'z'; *ptr++ = 'p'; *ptr++ = 'c'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'Z'; *ptr++ = 'P'; *ptr++ = 'C'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Sr _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 's'; *ptr++ = 'r'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'S'; *ptr++ = 'R'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Usp _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'u'; *ptr++ = 's'; *ptr++ = 'p'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'U'; *ptr++ = 'S'; *ptr++ = 'P'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Cn cn)
{
    bool valid = cn.raw <= 0x007 || (cn.raw >= 0x800 && cn.raw <= 0x807);
    bool upper = style != DASM_GNU && style != DASM_GNU_MIT;

    if (valid) {

        if (style == DASM_GNU_MIT || style == DASM_MOIRA_MIT) {
            *ptr++ = '%';
        }

        switch (cn.raw) {
                
            case 0x000: *this << (upper ? "SFC"   : "sfc");   break;
            case 0x001: *this << (upper ? "DFC"   : "dfc");   break;
            case 0x002: *this << (upper ? "CACR"  : "cacr");  break;
            case 0x003: *this << (upper ? "TC"    : "tc");    break;
            case 0x004: *this << (upper ? "ITT0"  : "itt0");  break;
            case 0x005: *this << (upper ? "ITT1"  : "itt1");  break;
            case 0x006: *this << (upper ? "DTT0"  : "dtt0");  break;
            case 0x007: *this << (upper ? "DTT1"  : "dtt1");  break;
            case 0x008: *this << (upper ? "BUSCR" : "buscr"); break;
            case 0x800: *this << (upper ? "USP"   : "usp");   break;
            case 0x801: *this << (upper ? "VBR"   : "vbr");   break;
            case 0x802: *this << (upper ? "CAAR"  : "caar");  break;
            case 0x803: *this << (upper ? "MSP"   : "msp");   break;
            case 0x804: *this << (upper ? "ISP"   : "isp");   break;
            case 0x805: *this << (upper ? "MMUSR" : "mmusr"); break;
            case 0x806: *this << (upper ? "URP"   : "urp");   break;
            case 0x807: *this << (upper ? "SRP"   : "srp");   break;
            case 0x808: *this << (upper ? "PCR"   : "pcr");   break;
        }
        
    } else {

        if (style == DASM_MUSASHI || style == DASM_GNU || style == DASM_GNU_MIT) {
            *this << UInt(cn.raw);
        } else {
            *this << "INVALID";
        }
    }
    
    return *this;
}

StrWriter&
StrWriter::operator<<(RegList l)
{
    int r[16];
    
    // Step 1: Fill array r with the register list bits, e.g., 11101101
    for (int i = 0; i <= 15; i++) { r[i] = !!(l.raw & (1 << i)); }
    
    // Step 2: Convert 11101101 to 12301201
    for (int i = 1; i <= 15; i++) { if (r[i]) r[i] = r[i-1] + 1; }
    
    // Step 3: Convert 12301201 to 33302201
    for (int i = 14; i >= 0; i--) { if (r[i] && r[i+1]) r[i] = r[i+1]; }
    
    // Step 4: Convert 33302201 to "D0-D2/D4/D5/D7"
    bool first = true;
    for (int i = 0; i <= 15; i += r[i] + 1) {
        
        if (r[i] == 0) continue;
        
        // Print delimiter
        if (first) { first = false; } else { *this << "/"; }
        
        // Format variant 1: Single register
        if (r[i] == 1) { *this << Rn{i}; }
        
        // Format variant 2: Register range
        else { *this << Rn{i} << "-" << Rn{i+r[i]-1}; }
    }
    
    return *this;
}

StrWriter&
StrWriter::operator<<(RegRegList l)
{
    switch (style) {

        case DASM_GNU:
        case DASM_GNU_MIT:

            l.raw ? *this << RegList{l.raw} : *this << "#0";
            break;

        default:

            u16 regsD = l.raw & 0x00FF;
            u16 regsA = l.raw & 0xFF00;

            *this << RegList { regsD };
            if (regsD && regsA) *this << "/";
            *this << RegList { regsA };
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(FRegList l)
{
    int r[8];

    // Step 1: Fill array r with the register list bits, e.g., 11101101
    for (int i = 0; i <= 7; i++) { r[i] = !!(l.raw & (1 << i)); }

    // Step 2: Convert 11101101 to 12301201
    for (int i = 1; i <= 7; i++) { if (r[i]) r[i] = r[i-1] + 1; }

    // Step 3: Convert 12301201 to 33302201
    for (int i = 6; i >= 0; i--) { if (r[i] && r[i+1]) r[i] = r[i+1]; }

    // Step 4: Convert 33302201 to "FP0-FP2/FP4/FP5/FP7"
    bool first = true;
    for (int i = 0; i <= 7; i += r[i] + 1) {

        if (r[i] == 0) continue;

        // Print delimiter
        if (first) { first = false; } else { *this << "/"; }

        // Format variant 1: Single register
        if (r[i] == 1) { *this << Fp{i}; }

        // Format variant 2: Register range
        else { *this << Fp{i} << "-" << Fp{i+r[i]-1}; }
    }

    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(const Ea<M, S> &ea)
{
    switch (M) {
            
        case MODE_DN:   *this << Dn{ea.reg};    break;
        case MODE_AN:   *this << An{ea.reg};    break;
        case MODE_AI:   *this << Ai<M,S>{ea};   break;
        case MODE_PI:   *this << Pi<M,S>{ea};   break;
        case MODE_PD:   *this << Pd<M,S>{ea};   break;
        case MODE_DI:   *this << Di<M,S>{ea};   break;
        case MODE_IX:   *this << Ix<M,S>{ea};   break;
        case MODE_AW:   *this << Aw<M,S>{ea};   break;
        case MODE_AL:   *this << Al<M,S>{ea};   break;
        case MODE_DIPC: *this << DiPc<M,S>{ea}; break;
        case MODE_IXPC: *this << Ix<M,S>{ea};   break;
        case MODE_IM:   *this << Im<M,S>{ea};   break;

        default:
            *this << "???";
    }
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Ai<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:
        case DASM_GNU:

            *this << "(" << An{ea.reg} << ")";
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << An{ea.reg} << "@";
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Pi<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:
        case DASM_GNU:

            *this << "(" << An{ea.reg} << ")+";
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << An{ea.reg} << "@+";
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Pd<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:
        case DASM_GNU:

            *this << "-(" << An{ea.reg} << ")";
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << An{ea.reg} << "@-";
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Di<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {

        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:

            *this << "(" << Int{(i16)ea.ext1} << "," << An{ea.reg} << ")";
            return *this;
            
        case DASM_GNU:

            *this << Int{(i16)ea.ext1} << "(" << An{ea.reg} << ")";
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << An{ea.reg} << "@(" << Int{(i16)ea.ext1} << ")";
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Ix<M, S> wrapper)
{
    switch (style) {

        case DASM_MUSASHI:

            *this << IxMus<M, S>{wrapper.ea};
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << IxMit<M, S>{wrapper.ea};
            break;

        default:

            *this << IxMot<M, S>{wrapper.ea};
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(IxMot<M, S> wrapper)
{
    assert(M == 6 || M == 10);

    auto &ea = wrapper.ea;

    u16 full = _______x________ (ea.ext1);

    if (!full) {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 0  | DISPLACEMENT                         |
        // --------------------------------------------------------------------

        u16 reg   = xxxx____________ (ea.ext1);
        u16 lw    = ____x___________ (ea.ext1);
        u16 scale = _____xx_________ (ea.ext1);
        u16 disp  = ________xxxxxxxx (ea.ext1);

        *this << "(" << Int{(i8)disp} << ",";
        M == 10 ? *this << Pc{} : *this << An{ea.reg};
        *this << "," << Rn{reg} << (lw ? ".l" : ".w") << Scale{scale} << ")";

    } else {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 1  | BS | IS | BD SIZE  | 0  | IIS        |
        // --------------------------------------------------------------------

        u16  reg   = xxxx____________ (ea.ext1);
        u16  lw    = ____x___________ (ea.ext1);
        u16  scale = _____xx_________ (ea.ext1);
        u16  bs    = ________x_______ (ea.ext1);
        u16  is    = _________x______ (ea.ext1);
        u16  size  = __________xx____ (ea.ext1);
        u16  iis   = _____________xxx (ea.ext1);
        u32  base  = ea.ext2;
        u32  outer = ea.ext3;

        auto baseDisplacement = [&]() {
            size == 3 ? (*this << Int{(i32)base}) : (*this << Int{(i16)base});
        };
        auto baseRegister = [&]() {
            if constexpr (M == 10) {
                if (!bs) { *this << Sep{} << Pc{}; } else { *this << Sep{} << Zpc{}; }
            } else {
                if (!bs) { *this << Sep{} << An{ea.reg}; }
            }
        };
        auto indexRegister = [&]() {
            if (!is) *this << Sep{} << Rn{reg} << (lw ? ".l" : ".w") << Scale{scale};
        };
        auto outerDisplacement = [&]() {
            *this << Sep{} << Int(outer);
        };

        if (iis == 0) {

            // Memory Indirect Unindexed
            *this << "(";
            baseDisplacement(); baseRegister(); indexRegister();
            *this << ")";

        } else if (iis & 0b100) {

            // Memory Indirect Postindexed
            *this << "([";
            baseDisplacement(); baseRegister();
            *this << "]";
            indexRegister(); outerDisplacement();
            *this << ")";

        } else {

            // Memory Indirect Preindexed
            *this << "([";
            baseDisplacement(); baseRegister(); indexRegister();
            *this << "]";
            outerDisplacement();
            *this << ")";
        }
    }

    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(IxMit<M, S> wrapper)
{
    assert(M == 6 || M == 10);

    auto &ea = wrapper.ea;

    u16 full = _______x________ (ea.ext1);

    if (!full) {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 0  | DISPLACEMENT                         |
        // --------------------------------------------------------------------

        u16 reg   = xxxx____________ (ea.ext1);
        u16 lw    = ____x___________ (ea.ext1);
        u16 scale = _____xx_________ (ea.ext1);
        u16 disp  = ________xxxxxxxx (ea.ext1);

        M == 10 ? *this << Pc{} : *this << An{ea.reg};
        *this << "@(" << Int{(i8)disp};
        *this << "," << Rn{reg} << (lw ? ":l" : ":w") << Scale{scale} << ")";

    } else {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 1  | BS | IS | BD SIZE  | 0  | IIS        |
        // --------------------------------------------------------------------

        u16  reg   = xxxx____________ (ea.ext1);
        u16  lw    = ____x___________ (ea.ext1);
        u16  scale = _____xx_________ (ea.ext1);
        u16  bs    = ________x_______ (ea.ext1);
        u16  is    = _________x______ (ea.ext1);
        u16  size  = __________xx____ (ea.ext1);
        u16  iis   = _____________xxx (ea.ext1);
        u32  base  = ea.ext2;
        u32  outer = ea.ext3;

        auto baseDisplacement = [&]() {
            size == 3 ? (*this << Int{(i32)base}) : (*this << Int{(i16)base});
        };
        auto baseRegister = [&]() {
            if constexpr (M == 10) {
                bs ? *this << Zpc{} : *this << Pc{};
            } else {
                if (!bs) *this << An{ea.reg};
            }
        };
        auto indexRegister = [&]() {
            if (!is) *this << Sep{} << Rn{reg} << (lw ? ":l" : ":w") << Scale{scale};
        };
        auto outerDisplacement = [&]() {
            *this << Int(outer);
        };

        if (iis == 0) {

            // Memory Indirect Unindexed
            baseRegister();
            *this << "@(";
            baseDisplacement(); indexRegister();
            *this << ")";

        } else if (iis & 0b100) {

            // Memory Indirect Postindexed
            baseRegister();
            *this << "@(";
            baseDisplacement();
            *this << ")@(";
            outerDisplacement(); indexRegister();
            *this << ")";

        } else {

            // Memory Indirect Preindexed
            baseRegister();
            *this << "@(";
            baseDisplacement(); indexRegister();
            *this << ")@(";
            outerDisplacement();
            *this << ")";
        }
    }

    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(IxMus<M, S> wrapper)
{
    assert(M == 6 || M == 10);

    auto &ea = wrapper.ea;

    u16 full = _______x________ (ea.ext1);

    if (!full) {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 0  | DISPLACEMENT                         |
        // --------------------------------------------------------------------

        u16 reg   = xxxx____________ (ea.ext1);
        u16 lw    = ____x___________ (ea.ext1);
        u16 scale = _____xx_________ (ea.ext1);
        u16 disp  = ________xxxxxxxx (ea.ext1);

        *this << "(";
        if (disp) *this << Int{(i8)disp} << ",";
        M == 10 ? *this << Pc{} : *this << An{ea.reg};
        *this << "," << Rn{reg};
        *this << (lw ? ".l" : ".w");
        *this << Scale{scale} << ")";

    } else {

        //   15-12   11   10   09   08   07   06   05   04   03   02   01   00
        // --------------------------------------------------------------------
        // | REG   | LW | SCALE   | 1  | BS | IS | BD SIZE  | 0  | IIS        |
        // --------------------------------------------------------------------

        u16  reg   = xxxx____________ (ea.ext1);
        u16  lw    = ____x___________ (ea.ext1);
        u16  scale = _____xx_________ (ea.ext1);
        u16  bs    = ________x_______ (ea.ext1);
        u16  is    = _________x______ (ea.ext1);
        u16  size  = __________xx____ (ea.ext1);
        u16  iis   = _____________xxx (ea.ext1);
        u32  base  = ea.ext2;
        u32  outer = ea.ext3;

        bool preindex = (iis > 0 && iis < 4);
        bool postindex = (iis > 4);
        bool effectiveZero = (ea.ext1 & 0xe4) == 0xC4 || (ea.ext1 & 0xe2) == 0xC0;
        bool comma = false;

        if (effectiveZero) {

            *this << "0";
            return *this;
        }

        *this << "(";

        if (preindex || postindex) {

            *this << "[";
        }
        if (base) {

            size == 3 ? (*this << Int{(i32)base}) : (*this << Int{(i16)base});
            comma = true;
        }
        if (!bs) {

            if (comma) *this << ",";
            M == 10 ? *this << Pc{} : *this << An{ea.reg};
            comma = true;
        }
        if (postindex) {

            *this << "]";
            comma = true;
        }
        if (!is) {

            if (comma) *this << ",";
            *this << Rn{reg};
            *this << (lw ? ".l" : ".w");
            *this << Scale{scale};
            comma = true;
        }
        if (preindex) {

            *this << "]";
            comma = true;
        }
        if(outer)
        {
            if (comma) *this << ",";
            *this << Int(outer);
        }

        *this << ")";
    }

    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Aw<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MOIRA_MIT:
        case DASM_MUSASHI:

            *this << UInt(ea.ext1) << Sz<Word>{};
            break;

        case DASM_GNU:
        case DASM_GNU_MIT:

            *this << UInt(ea.ext1);
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Al<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MOIRA_MIT:
        case DASM_MUSASHI:

            *this << UInt(ea.ext1) << Sz<Long>{};
            break;
            
        case DASM_GNU:
        case DASM_GNU_MIT:

            *this << UInt(ea.ext1);
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(DiPc<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    u32 resolved;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:

            *this << "(" << Int{(i16)ea.ext1} << ",PC)";
            resolved = U32_ADD(U32_ADD(ea.pc, (i16)ea.ext1), 2);
            StrWriter(comment, style, nf) << "; (" << UInt(resolved) << ")" << Finish{};
            break;
            
        case DASM_GNU:

            resolved = U32_ADD(U32_ADD(ea.pc, (i16)ea.ext1), 2);
            *this << UInt(resolved) << "(" << Pc{} << ")";
            break;

        case DASM_MOIRA_MIT:
        case DASM_GNU_MIT:

            resolved = U32_ADD(U32_ADD(ea.pc, (i16)ea.ext1), 2);
            *this << Pc{} << "@(" << UInt(resolved) << ")";
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Im<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MOIRA_MIT:
        case DASM_GNU:
        case DASM_GNU_MIT:

            *this << Ims<S>(ea.ext1);
            break;
            
        case DASM_MUSASHI:
            
            *this << Imu(ea.ext1);
            break;
    }
    
    return *this;
}

template <Mode M, Size S> StrWriter&
StrWriter::operator<<(Ip<M, S> wrapper)
{
    auto &ea = wrapper.ea;
    
    switch (style) {
            
        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:
        case DASM_GNU:

            *this << "-(" << An{ea.reg} << ")";
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *this << An{ea.reg} << "@-";
            break;
    }
    
    return *this;
}

StrWriter&
StrWriter::operator<<(Scale s)
{
    if (!s.raw) return *this;
    
    switch (style) {

        case DASM_MOIRA_MOT:
        case DASM_MUSASHI:
        case DASM_GNU:

            *ptr++ = '*';
            *ptr++ = '0' + (char)(1 << s.raw);
            break;

        case DASM_GNU_MIT:
        case DASM_MOIRA_MIT:

            *ptr++ = ':';
            *ptr++ = '0' + (char)(1 << s.raw);
            break;
    }
    
    return *this;
}

StrWriter&
StrWriter::operator<<(Fc fc)
{
    // 10XXX — Function code is specified as bits XXX
    // 01DDD — Function code is specified as bits 2–0 of data register DDD
    // 00000 — Function code is specified as source function code register
    // 00001 — Function code is specified as destination function code register

    switch (fc.raw >> 3 & 0b11) {

        case 0b10:  *this << Imu(fc.raw & 0b111); break;
        case 0b01:  *this << Dn(fc.raw & 0b111); break;

        case 0b00:

            if (fc.raw == 0) { *this << Sfc(); break; }
            if (fc.raw == 1) { *this << Dfc(); break; }
            [[fallthrough]];

        default:    *this << Imu(fc.raw & 0b1111);
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Sfc _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 's'; *ptr++ = 'f'; *ptr++ = 'c'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'S'; *ptr++ = 'F'; *ptr++ = 'C'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Dfc _)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'd'; *ptr++ = 'f'; *ptr++ = 'c'; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'D'; *ptr++ = 'F'; *ptr++ = 'C'; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Fp fp)
{
    switch (style) {

        case DASM_GNU_MIT:      *ptr++ = '%'; [[fallthrough]];
        case DASM_GNU:          *ptr++ = 'f'; *ptr++ = 'p'; *ptr++ = '0' + (char)fp.raw; break;
        case DASM_MOIRA_MIT:    *ptr++ = '%'; [[fallthrough]];
        default:                *ptr++ = 'F'; *ptr++ = 'P'; *ptr++ = '0' + (char)fp.raw; break;
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Ffmt ffmt)
{
    if (style != DASM_MOIRA_MIT && style != DASM_GNU_MIT) *ptr++ = '.';

    switch (ffmt.raw) {

        case 0: *ptr++ = 'l'; break;
        case 1: *ptr++ = 's'; break;
        case 2: *ptr++ = 'x'; break;
        case 3: *ptr++ = 'p'; break;
        case 4: *ptr++ = 'w'; break;
        case 5: *ptr++ = 'd'; break;
        case 6: *ptr++ = 'b'; break;

        default:
            *ptr++ = '?';
    }

    return *this;
}

StrWriter&
StrWriter::operator<<(Fctrl fctrl)
{
    const char *prefix = style == DASM_GNU_MIT || style == DASM_MOIRA_MIT ? "%" : "";
    const char *delim = "";

    if (fctrl.raw == 0 && style != DASM_GNU) { *this << "{}"; }
    if (fctrl.raw & 1) { *this << delim << prefix << "fpiar"; delim = "/"; }
    if (fctrl.raw & 2) { *this << delim << prefix << "fpsr";  delim = "/"; }
    if (fctrl.raw & 4) { *this << delim << prefix << "fpcr";  delim = "/"; }

    return *this;
}

StrWriter&
StrWriter::operator<<(Tab tab)
{
    switch (style) {

        case DASM_GNU:
        case DASM_GNU_MIT:

            *ptr++ = ' ';
            break;

        default:

            do { *ptr++ = ' '; } while (ptr < base + tab.raw);
    }

    return *this;
}

template <Instr I, Mode M, Size S> StrWriter&
StrWriter::operator<<(const Av<I, M, S> &av)
{
    if (style == DASM_GNU || style == DASM_GNU_MIT) { return *this; }
    
    switch (I) {
            
        case BKPT:
        case MOVES:
        case MOVEFCCR:
        case RTD:
            
            *this << "; (1+)";
            break;
            
        case CMPI:
            
            *this << (isPrgMode(M) ? "; (1+)" : "");
            break;
            
        case CALLM:
        case RTM:
            
            *this << "; (2)";
            break;
            
        case cpGEN:
        case cpRESTORE:
        case cpSAVE:
        case cpScc:
        case cpTRAPcc:
            
            *this << "; (2-3)";
            break;
            
        case BFCHG:
        case BFCLR:
        case BFEXTS:
        case BFEXTU:
        case BFFFO:
        case BFINS:
        case BFSET:
        case BFTST:
        case CAS:
        case CAS2:
        case CHK2:
        case CMP2:
        case DIVL:
        case EXTB:
        case MULL:
        case PACK:
        case TRAPCC:
        case TRAPCS:
        case TRAPEQ:
        case TRAPGE:
        case TRAPGT:
        case TRAPHI:
        case TRAPLE:
        case TRAPLS:
        case TRAPLT:
        case TRAPMI:
        case TRAPNE:
        case TRAPPL:
        case TRAPVC:
        case TRAPVS:
        case TRAPF:
        case TRAPT:
        case UNPK:
            
            *this << "; (2+)";
            break;
            
        case CHK:
        case LINK:
        case BRA:
        case BHI:
        case BLS:
        case BCC:
        case BCS:
        case BNE:
        case BEQ:
        case BVC:
        case BVS:
        case BPL:
        case BMI:
        case BGE:
        case BLT:
        case BGT:
        case BLE:
        case BSR:
            
            *this << (S == Long ? "; (2+)" : "");
            break;
            
        case TST:
            *this << (M == 1 || M >= 9 ? "; (2+)" : "");
            break;

        case CINV:
        case CPUSH:
        case MOVE16:

            *this << "; (4)";
            break;

        case MOVEC:
            
            switch (av.ext1 & 0x0FFF) {
                    
                case 0x000:
                case 0x001:
                case 0x800:
                case 0x801: *this << "; (1+)"; break;
                case 0x002:
                case 0x803:
                case 0x804: *this << "; (2+)"; break;
                case 0x802: *this << "; (2,3)"; break;
                case 0x003:
                case 0x004:
                case 0x005:
                case 0x006:
                case 0x007:
                case 0x805:
                case 0x806:
                case 0x807: *this << "; (4+)"; break;
                    
                default:    *this << "; (?)";
            }
            
        default:
            break;
    }
    return *this;
}

StrWriter&
StrWriter::operator<<(Sep)
{
    *ptr++ = ',';
    if (style == DASM_MUSASHI || style == DASM_MOIRA_MOT || style == DASM_MOIRA_MIT) *ptr++ = ' ';
    return *this;
}

StrWriter&
StrWriter::operator<<(Finish)
{
    // Append comment
    for (int i = 0; comment[i] != 0; i++) *ptr++ = comment[i];
    
    // Terminate the string
    *ptr = 0;
    
    return *this;
}
