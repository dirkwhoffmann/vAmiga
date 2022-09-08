// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <optional>

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif

namespace moira {

typedef int8_t             i8;
typedef int16_t            i16;
typedef int32_t            i32;
typedef long long          i64;
typedef uint8_t            u8;
typedef uint16_t           u16;
typedef uint32_t           u32;
typedef unsigned long long u64;

typedef enum
{
    M68000,             // Full supported
    M68010,             // Work in progress
    M68EC020,           // Work in progress
    M68020,             // Work in progress
    M68EC030,           // Work in progress
    M68030,             // Work in progress
    M68EC040,           // Work in progress
    M68LC040,           // Work in progress
    M68040              // Work in progress
}
Model;

// Availabilty masks
constexpr u16 AV_68040    = 1 << M68EC040 | 1 << M68LC040 | 1 << M68040;
constexpr u16 AV_68030    = 1 << M68EC030 | 1 << M68030;
constexpr u16 AV_68020    = 1 << M68EC020 | 1 << M68020;
constexpr u16 AV_68030_UP = AV_68030 | AV_68040;
constexpr u16 AV_68020_UP = AV_68020 | AV_68030_UP;
constexpr u16 AV_68010_UP = 1 << M68010 | AV_68020_UP;
constexpr u16 AV_68000_UP = 1 << M68000 | AV_68010_UP;
constexpr u16 AV_MMU      = 1 << M68030 | 1 << M68LC040 | 1 << M68040;
constexpr u16 AV_FPU      = 1 << M68040;

typedef enum
{
    C68000,             // Used by M68000
    C68010,             // Used by M68010
    C68020              // Used by all others
}
Core;

typedef enum
{
    DASM_MOIRA_MOT,      // Official syntax styles
    DASM_MOIRA_MIT,

    DASM_GNU,           // Legacy styles (for unit testing)
    DASM_GNU_MIT,
    DASM_MUSASHI,
}
DasmStyle;

typedef enum
{
    DASM_MIXED_CASE,    // Style is determined by the selected DasmStyle
    DASM_LOWER_CASE,    // Everything is printed in lowercase
    DASM_UPPER_CASE     // Everything is printed in uppercase
}
DasmLetterCase;

typedef struct
{
    const char *prefix;     // Prefix for hexidecimal numbers
    u8 radix;               // 10 or 16
    bool upperCase;         // Lettercase for hexadecimal digits A...F
    bool plainZero;         // Determines if 0 is printed with or without prefix
}
DasmNumberFormat;

typedef enum
{
    // 68000+ instruction set
    ABCD,       ADD,        ADDA,       ADDI,       ADDQ,       ADDX,
    AND,        ANDI,       ANDICCR,    ANDISR,     ASL,        ASR,
    BCC,        BCS,        BEQ,        BGE,        BGT,        BHI,
    BLE,        BLS,        BLT,        BMI,        BNE,        BPL,
    BVC,        BVS,        BCHG,       BCLR,       BRA,        BSET,
    BSR,        BTST,       CHK,        CLR,        CMP,        CMPA,
    CMPI,       CMPM,       DBCC,       DBCS,       DBEQ,       DBGE,
    DBGT,       DBHI,       DBLE,       DBLS,       DBLT,       DBMI,
    DBNE,       DBPL,       DBVC,       DBVS,       DBF,        DBT,
    DIVS,       DIVU,       EOR,        EORI,       EORICCR,    EORISR,
    EXG,        EXT,        ILLEGAL,    JMP,        JSR,        LEA,
    LINE_A,     LINE_F,     LINK,       LSL,        LSR,        MOVE,
    MOVEA,      MOVEFCCR,   MOVETCCR,   MOVEFSR,    MOVETSR,    MOVEUSP,
    MOVEM,      MOVEP,      MOVEQ,      MULS,       MULU,       NBCD,
    NEG,        NEGX,       NOP,        NOT,        OR,         ORI,
    ORICCR,     ORISR,      PEA,        RESET,      ROL,        ROR,
    ROXL,       ROXR,       RTE,        RTR,        RTS,        SBCD,
    SCC,        SCS,        SEQ,        SGE,        SGT,        SHI,
    SLE,        SLS,        SLT,        SMI,        SNE,        SPL,
    SVC,        SVS,        SF,         ST,         STOP,       SUB,
    SUBA,       SUBI,       SUBQ,       SUBX,       SWAP,       TAS,
    TRAP,       TRAPV,      TST,        UNLK,
    
    // 68010+ instructions
    BKPT,       MOVEC,      MOVES,      RTD,
    
    // 68020+ instructions
    BFCHG,      BFCLR,      BFEXTS,     BFEXTU,     BFFFO,      BFINS,
    BFSET,      BFTST,      CALLM,      CAS,        CAS2,       CHK2,
    CMP2,       cpBcc,      cpDBcc,     cpGEN,      cpRESTORE,  cpSAVE,
    cpScc,      cpTRAPcc,   DIVL,       EXTB,       MULL,       PACK,
    RTM,        TRAPCC,     TRAPCS,     TRAPEQ,     TRAPGE,     TRAPGT,
    TRAPHI,     TRAPLE,     TRAPLS,     TRAPLT,     TRAPMI,     TRAPNE,
    TRAPPL,     TRAPVC,     TRAPVS,     TRAPF,      TRAPT,      UNPK,

    // 68040+ instructions
    CINV,       CPUSH,      MOVE16,
    
    // MMU instructions
    PFLUSH,     PFLUSHA,    PFLUSHAN,   PFLUSHN,
    PLOAD,      PMOVE,      PTEST,

    // FPU instructions (68040 and 6888x)
    FABS,       FADD,       FBcc,       FCMP,       FDBcc,      FDIV,
    FMOVE,      FMOVEM,     FMUL,       FNEG,       FNOP,       FRESTORE,
    FSAVE,      FScc,       FSQRT,      FSUB,       FTRAPcc,    FTST,

    // FPU instructions (68040 only)
    FSABS,      FDABS,      FSADD,      FDADD,      FSDIV,      FDDIV,
    FSMOVE,     FDMOVE,     FSMUL,      FDMUL,      FSNEG,      FDNEG,
    FSSQRT,     FDSQRT,     FSSUB,      FDSUB,

    // FPU instructions (6888x only)
    FACOS,      FASIN,      FATAN,      FATANH,     FCOS,       FCOSH,
    FETOX,      FETOXM1,    FGETEXP,    FGETMAN,    FINT,       FINTRZ,
    FLOG10,     FLOG2,      FLOGN,      FLOGNP1,    FMOD,       FMOVECR,
    FREM,       FSCAL,      FSGLDIV,    FSGLMUL,    FSIN,       FSINCOS,
    FSINH,      FTAN,       FTANH,      FTENTOX,    FTWOTOX,

    // Loop mode variants (68010)
    ABCD_LOOP,  ADD_LOOP,   ADDA_LOOP,  ADDX_LOOP,  AND_LOOP,   ASL_LOOP,
    ASR_LOOP,   CLR_LOOP,   CMP_LOOP,   CMPA_LOOP,  DBCC_LOOP,  DBCS_LOOP,
    DBEQ_LOOP,  DBGE_LOOP,  DBGT_LOOP,  DBHI_LOOP,  DBLE_LOOP,  DBLS_LOOP,
    DBLT_LOOP,  DBMI_LOOP,  DBNE_LOOP,  DBPL_LOOP,  DBVC_LOOP,  DBVS_LOOP,
    DBF_LOOP,   DBT_LOOP,   EOR_LOOP,   LSL_LOOP,   LSR_LOOP,   MOVE_LOOP,
    NBCD_LOOP,  NEG_LOOP,   NEGX_LOOP,  NOT_LOOP,   OR_LOOP,    ROL_LOOP,
    ROR_LOOP,   ROXL_LOOP,  ROXR_LOOP,  SBCD_LOOP,  SUB_LOOP,   SUBA_LOOP,
    SUBX_LOOP,  TST_LOOP
}
Instr;

template <Instr I>
constexpr bool looping() { return I >= ABCD_LOOP && I <= TST_LOOP; }

typedef enum
{
    MODE_DN,                //  0         Dn
    MODE_AN,                //  1         An
    MODE_AI,                //  2       (An)
    MODE_PI,                //  3      (An)+
    MODE_PD,                //  4      -(An)
    MODE_DI,                //  5     (d,An)
    MODE_IX,                //  6  (d,An,Xi)
    MODE_AW,                //  7   (####).w
    MODE_AL,                //  8   (####).l
    MODE_DIPC,              //  9     (d,PC)
    MODE_IXPC,              // 10  (d,PC,Xi)
    MODE_IM,                // 11       ####
    MODE_IP                 // 12       ----
}
Mode;

constexpr bool isRegMode(Mode M) { return M == 0 || M == 1;  }
constexpr bool isAbsMode(Mode M) { return M == 7 || M == 8;  }
constexpr bool isIdxMode(Mode M) { return M == 6 || M == 10; }
constexpr bool isMemMode(Mode M) { return M >= 2 && M <= 10; }
constexpr bool isPrgMode(Mode M) { return M == 9 || M == 10; }
constexpr bool isDspMode(Mode M) { return M == 5 || M == 6 || M == 9 || M == 10; }
constexpr bool isImmMode(Mode M) { return M == 11; }

typedef enum
{
    Unsized = 0,
    Byte    = 1,            // .b : Byte addressing
    Word    = 2,            // .w : Word addressing
    Long    = 4             // .l : Long word addressing
}
Size;

typedef enum
{
    COND_BT,                // Always true
    COND_BF,                // Always false
    COND_HI,                // Higher than
    COND_LS,                // Lower or same
    COND_CC,                // Carry clear
    COND_CS,                // Carry set
    COND_NE,                // Not equal
    COND_EQ,                // Equal
    COND_VC,                // Overflow clear
    COND_VS,                // Overflow set
    COND_PL,                // Plus
    COND_MI,                // Minus
    COND_GE,                // Greater or equal
    COND_LT,                // Less than
    COND_GT,                // Greater than
    COND_LE                 // Less than
}
Cond;

typedef enum
{
    EXC_RESET               = 1,
    EXC_BUS_ERROR           = 2,
    EXC_ADDRESS_ERROR       = 3,
    EXC_ILLEGAL             = 4,
    EXC_DIVIDE_BY_ZERO      = 5,
    EXC_CHK                 = 6,
    EXC_TRAPV               = 7,
    EXC_PRIVILEGE           = 8,
    EXC_TRACE               = 9,
    EXC_LINEA               = 10,
    EXC_LINEF               = 11,
    EXC_FORMAT_ERROR        = 14,
    EXC_IRQ_UNINITIALIZED   = 15,
    EXC_IRQ_SPURIOUS        = 24,
    EXC_TRAP                = 32,

    // Experimental
    EXC_BKPT
}
ExceptionType;

typedef struct
{
    Instr I;
    Mode  M;
    Size  S;
}
InstrInfo;

typedef enum
{
    IRQ_AUTO,
    IRQ_USER,
    IRQ_SPURIOUS,
    IRQ_UNINITIALIZED
}
IrqMode;

typedef u8 FunctionCode;
static constexpr u8 FC_USER_DATA = 1;
static constexpr u8 FC_USER_PROG = 2;
static constexpr u8 FC_SUPERVISOR_DATA = 5;
static constexpr u8 FC_SUPERVISOR_PROG = 6;

typedef enum
{
    MEM_DATA                = 1,
    MEM_PROG                = 2
}
MemSpace;

typedef struct
{
    u16 code;
    u32 addr;
    u16 ird;
    u16 sr;
    u32 pc;
}
StackFrame;

struct StatusRegister {
    
    bool t1;                // Trace flag
    bool t0;                // Trace flag         (68020 only)
    bool s;                 // Supervisor flag
    bool m;                 // Master flag        (68020 only)
    bool x;                 // Extend flag
    bool n;                 // Negative flag
    bool z;                 // Zero flag
    bool v;                 // Overflow flag
    bool c;                 // Carry flag
    
    u8 ipl;                 // Required Interrupt Priority Level
};

struct Registers {
    
    u32 pc;                 // Program counter
    u32 pc0;                // Beginning of the currently executed instruction
    StatusRegister sr;      // Status register
    
    union {
        struct {
            u32 d[8];       // D0, D1 ... D7
            u32 a[8];       // A0, A1 ... A7
        };
        struct {
            u32 r[16];      // D0, D1 ... D7, A0, A1 ... A7
        };
        struct {
            u32 _pad[15];
            u32 sp;         // Visible stack pointer (overlays a[7])
        };
    };
    
    u32 usp;                // User Stack Pointer
    u32 isp;                // Interrupt Stack Pointer
    u32 msp;                // Master Stack Pointer             (68020+)
    
    u8 ipl;                 // Polled Interrupt Priority Level
    
    u32 vbr;                // Vector Base Register             (68010+)
    u32 sfc;                // Source Function Code             (68010+)
    u32 dfc;                // Destination Function Code        (68010+)
    
    // Unemulated registers
    u32 cacr;               // Cache Control Register           (68020+)
    u32 caar;               // Cache Address Register           (68020+)
};

struct PrefetchQueue {
    
    u16 irc;                // The most recent word prefetched from memory
    u16 ird;                // The instruction currently being executed
};

struct MMU {

    u64 crp;                // Cpu Root Pointer
    u64 srp;                // Supervisor Root Pointer
    u32 trc;                // Translation Control Register
    u32 tt0;                // Transparent Translation Register 0
    u32 tt1;                // Transparent Translation Register 1
    u16 mmusr;              // MMU status register
};

/* Execution flags
 *
 * The M68k is a well organized processor that breaks down the execution of
 * an instruction to a limited number of general execution schemes. However,
 * the schemes slighty differ between instruction. To take care of the subtle
 * differences, some functions take an additional 'flags' argument to alter
 * their behavior. All flags are passed as a template parameter for efficiency.
 */

typedef u64 Flags;

// Memory access flags
constexpr u64 REVERSE       (1 << 0);   // Reverse the long word access order
constexpr u64 SKIP_LAST_RD  (1 << 1);   // Don't read the extension word

// Interrupt flags
constexpr u64 POLLIPL       (1 << 2);   // Poll the interrupt lines

// Address error flags
constexpr u64 AE_WRITE      (1 << 3);   // Clear read flag in code word
constexpr u64 AE_PROG       (1 << 4);   // Set FC pins to program space
constexpr u64 AE_DATA       (1 << 5);   // Set FC pins to user space
constexpr u64 AE_INC_PC     (1 << 6);   // Increment PC by 2 in stack frame
constexpr u64 AE_DEC_PC     (1 << 7);   // Decrement PC by 2 in stack frame
constexpr u64 AE_INC_A      (1 << 8);   // Increment ADDR by 2 in stack frame
constexpr u64 AE_DEC_A      (1 << 9);   // Decrement ADDR by 2 in stack frame
constexpr u64 AE_SET_CB3    (1 << 10);  // Set bit 3 in CODE segment

// Timing flags
constexpr u64 IMPL_DEC      (1 << 11);  // Omit 2 cycle delay in -(An) mode

}
