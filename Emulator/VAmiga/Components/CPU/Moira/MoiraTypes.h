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

namespace vamiga::moira {


//
// Datatypes
//

typedef int8_t              i8;
typedef int16_t             i16;
typedef int32_t             i32;
typedef long long           i64;
typedef uint8_t             u8;
typedef uint16_t            u16;
typedef uint32_t            u32;
typedef unsigned long long  u64;


//
// Enumerations
//

enum class Model
{
    M68000,                 // Fully supported
    M68010,                 // Fully supported
    M68EC020,               // Work in progress
    M68020,                 // Work in progress
    M68EC030,               // Disassembler only
    M68030,                 // Disassembler only
    M68EC040,               // Disassembler only
    M68LC040,               // Disassembler only
    M68040                  // Disassembler only
};

/*
enum class FPUModel
{
    INTERNAL_FPU,           // Built-in FPU (in any)
    M68881,                 // Floating-point coprocessor
    M68882                  // Floating-point coprocessor
};
*/

enum class Core
{
    C68000,                 // Used by M68000
    C68010,                 // Used by M68010
    C68020                  // Used by all others
};

enum class Syntax
{
    MOIRA,                  // Official syntax styles
    MOIRA_MIT,
    GNU,                    // Legacy styles (for unit testing)
    GNU_MIT,
    MUSASHI,
};

enum class LetterCase
{
    MIXED_CASE,        // Style is determined by the selected DasmSyntax
    LOWER_CASE,        // Everything is printed in lowercase
    UPPER_CASE         // Everything is printed in uppercase
};

enum class Instr
{
    // 68000 instructions
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

    // 68010 instructions
    BKPT,       MOVEC,      MOVES,      RTD,

    // 68020 instructions
    BFCHG,      BFCLR,      BFEXTS,     BFEXTU,     BFFFO,      BFINS,
    BFSET,      BFTST,      CALLM,      CAS,        CAS2,       CHK2,
    CMP2,       cpBcc,      cpDBcc,     cpGEN,      cpRESTORE,  cpSAVE,
    cpScc,      cpTRAPcc,   DIVL,       EXTB,       MULL,       PACK,
    RTM,        TRAPCC,     TRAPCS,     TRAPEQ,     TRAPGE,     TRAPGT,
    TRAPHI,     TRAPLE,     TRAPLS,     TRAPLT,     TRAPMI,     TRAPNE,
    TRAPPL,     TRAPVC,     TRAPVS,     TRAPF,      TRAPT,      UNPK,

    // 68040 instructions
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
    ASR_LOOP,   CLR_LOOP,   CMP_LOOP,   CMPA_LOOP,  CMPM_LOOP,  DBCC_LOOP,
    DBCS_LOOP,  DBEQ_LOOP,  DBGE_LOOP,  DBGT_LOOP,  DBHI_LOOP,  DBLE_LOOP,
    DBLS_LOOP,  DBLT_LOOP,  DBMI_LOOP,  DBNE_LOOP,  DBPL_LOOP,  DBVC_LOOP,
    DBVS_LOOP,  DBF_LOOP,   DBT_LOOP,   EOR_LOOP,   LSL_LOOP,   LSR_LOOP,
    MOVE_LOOP,  NBCD_LOOP,  NEG_LOOP,   NEGX_LOOP,  NOT_LOOP,   OR_LOOP,
    ROL_LOOP,   ROR_LOOP,   ROXL_LOOP,  ROXR_LOOP,  SBCD_LOOP,  SUB_LOOP,
    SUBA_LOOP,  SUBX_LOOP,  TST_LOOP
};

using Size = int;
static constexpr int Unsized     = 0;
static constexpr int Byte        = 1;        // .b : Byte addressing
static constexpr int Word        = 2;        // .w : Word addressing
static constexpr int Long        = 4;        // .l : Long word addressing
static constexpr int Quad        = 8;        // FPU
static constexpr int Extended    = 12;       // FPU

/*
typedef enum
{
    Unsized     = 0,
    Byte        = 1,        // .b : Byte addressing
    Word        = 2,        // .w : Word addressing
    Long        = 4,        // .l : Long word addressing

    Quad        = 8,        // FPU
    Extended    = 12        // FPU
}
Size;
*/

enum class Mode
{
    DN,                //  0: Dn
    AN,                //  1: An
    AI,                //  2: (An)
    PI,                //  3: (An)+
    PD,                //  4: -(An)
    DI,                //  5: (d,An)
    IX,                //  6: (d,An,Xi)
    AW,                //  7: (####).w
    AL,                //  8: (####).l
    DIPC,              //  9: (d,PC)
    IXPC,              // 10: (d,PC,Xi)
    IM,                // 11: ####
    IP                 // 12: ----
};

constexpr bool isRegMode(Mode M) { return M == Mode::DN || M == Mode::AN;  }
constexpr bool isAbsMode(Mode M) { return M == Mode::AW || M == Mode::AL;  }
constexpr bool isIdxMode(Mode M) { return M == Mode::IX || M == Mode::IXPC; }
constexpr bool isMemMode(Mode M) { return M >= Mode::AI && M <= Mode::IXPC; }
constexpr bool isPrgMode(Mode M) { return M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isDspMode(Mode M) { return M == Mode::DI || M == Mode::IX || M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isImmMode(Mode M) { return M == Mode::IM; }

/*
constexpr bool isRegMode(Mode M) { return M == 0 || M == 1;  }
constexpr bool isAbsMode(Mode M) { return M == 7 || M == 8;  }
constexpr bool isIdxMode(Mode M) { return M == 6 || M == 10; }
constexpr bool isMemMode(Mode M) { return M >= 2 && M <= 10; }
constexpr bool isPrgMode(Mode M) { return M == 9 || M == 10; }
constexpr bool isDspMode(Mode M) { return M == 5 || M == 6 || M == 9 || M == 10; }
constexpr bool isImmMode(Mode M) { return M == 11; }
*/

enum class Cond
{
    BT,                // Always true
    BF,                // Always false
    HI,                // Higher than
    LS,                // Lower or same
    CC,                // Carry clear
    CS,                // Carry set
    NE,                // Not equal
    EQ,                // Equal
    VC,                // Overflow clear
    VS,                // Overflow set
    PL,                // Plus
    MI,                // Minus
    GE,                // Greater or equal
    LT,                // Less than
    GT,                // Greater than
    LE                 // Less than
};

enum class M68kException
{
    // Native exceptions
    RESET               = 1,
    BUS_ERROR           = 2,
    ADDRESS_ERROR       = 3,
    ILLEGAL             = 4,
    DIVIDE_BY_ZERO      = 5,
    CHK                 = 6,
    TRAPV               = 7,
    PRIVILEGE           = 8,
    TRACE               = 9,
    LINEA               = 10,
    LINEF               = 11,
    FORMAT_ERROR        = 14,
    IRQ_UNINITIALIZED   = 15,
    IRQ_SPURIOUS        = 24,
    TRAP                = 32,

    // Exception aliases (will be mapped to a native exception)
    BKPT
};

enum class IrqMode
{
    AUTO,
    USER,
    SPURIOUS,
    UNINITIALIZED
};

enum class AddrSpace
{
    DATA = 1,
    PROG = 2
};


//
// Floating-point types (unused)
//

enum class FltFormat
{
    LONG,
    SINGLE,
    EXTENDED,
    PACKED,
    WORD,
    DOUBLE,
    BYTE
};

enum class FpuPrecision
{
    EXTENDED,
    SINGLE,
    DOUBLE,
    UNDEFINED
};

enum class FpuRoundingMode
{
    NEAREST,
    ZERO,
    DOWNWARD,
    UPWARD
};

enum class FpuFrameType
{
    INVALID,
    NULLFRAME,
    IDLE,
    UNIMP,
    BUSY
};


//
// Structures
//

struct StackFrame
{
    u16 code;
    u32 addr;
    u16 ird;
    u16 sr;
    u32 pc;

    u16 fc;                 // Function code
    u16 ssw;                // Special status word (68010)
};

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

struct InstrInfo
{
    Instr I;
    Mode  M;
    Size  S;
};

struct DasmNumberFormat
{
    const char *prefix;     // Prefix for hexidecimal numbers
    u8 radix;               // 10 (decimal) or 16 (hexadecimal)
    bool upperCase;         // Lettercase for hexadecimal digits A...F
    bool plainZero;         // Determines whether 0 is printed with a prefix
};

struct DasmStyle
{
    Syntax syntax;
    LetterCase letterCase;
    DasmNumberFormat numberFormat;
    int tab;
};


//
// Flags and masks
//

// Function codes
namespace FC {

static constexpr u8 USER_DATA       = 1;
static constexpr u8 USER_PROG       = 2;
static constexpr u8 SUPERVISOR_DATA = 5;
static constexpr u8 SUPERVISOR_PROG = 6;
}

// Availabilty masks
namespace AV {

static constexpr u16 M68000         = 1 << int(Model::M68000);
static constexpr u16 M68010         = 1 << int(Model::M68010);
static constexpr u16 M68020         = 1 << int(Model::M68EC020) | 1 << int(Model::M68020);
static constexpr u16 M68030         = 1 << int(Model::M68EC030) | 1 << int(Model::M68030);
static constexpr u16 M68040         = 1 << int(Model::M68EC040) | 1 << int(Model::M68LC040) | 1 << int(Model::M68040);
static constexpr u16 MMU            = 1 << int(Model::M68030) | 1 << int(Model::M68LC040) | 1 << int(Model::M68040);
static constexpr u16 FPU            = 1 << int(Model::M68040);
static constexpr u16 M68030_UP      = M68030 | M68040;
static constexpr u16 M68020_UP      = M68020 | M68030_UP;
static constexpr u16 M68010_UP      = M68010 | M68020_UP;
static constexpr u16 M68000_UP      = M68000 | M68010_UP;
}

/**
 * @brief CPU Execution Flags
 * @details These flags control the CPU's execution state and behavior.
 */
namespace State {

/** CPU is in a halted state due to a double fault. Cleared only on reset. */
static constexpr int HALTED         = (1 << 0);

/** CPU is stopped after executing a STOP instruction. Cleared on the next interrupt. */
static constexpr int STOPPED        = (1 << 1);

/** CPU is in loop mode (68010 feature for optimizing DBcc loops). */
static constexpr int LOOPING        = (1 << 2);

/** Enables instruction logging, storing register states in a log buffer. */
static constexpr int LOGGING        = (1 << 3);

/** Reflects the T flag from the status register, used to speed up emulation. */
static constexpr int TRACING        = (1 << 4);

/** Triggers a trace exception when set. */
static constexpr int TRACE_EXC      = (1 << 5);

/** CPU checks for pending interrupts only if this flag is set. Cleared when no interrupt is possible. */
static constexpr int CHECK_IRQ      = (1 << 6);

/** Enables checking for breakpoints. */
static constexpr int CHECK_BP       = (1 << 7);

/** Enables checking for watchpoints. */
static constexpr int CHECK_WP       = (1 << 8);

/** Enables checking for catchpoints. */
static constexpr int CHECK_CP       = (1 << 9);

}

/* Instruction Flags
 *
 * The M68k is a well organized processor that breaks down the execution of
 * an instruction to a limited number of general execution schemes. However,
 * the schemes slighty differ between instruction. To take care of the subtle
 * differences, some functions take an additional 'flags' argument to alter
 * their behavior. All flags are passed as a template parameter for efficiency.
 */
typedef u64 Flags;

// Memory access flags
static constexpr u64 REVERSE        = (1 << 0);   // Reverse the long word access order
static constexpr u64 SKIP_LAST_RD   = (1 << 1);   // Don't read the extension word

// Interrupt flags
static constexpr u64 POLL           = (1 << 2);   // Poll the interrupt lines

// Address error flags
static constexpr u64 AE_WRITE       = (1 << 3);   // Clear read flag in code word
static constexpr u64 AE_PROG        = (1 << 4);   // Set FC pins to program space
static constexpr u64 AE_DATA        = (1 << 5);   // Set FC pins to user space
static constexpr u64 AE_INC_PC      = (1 << 6);   // Increment PC by 2 in stack frame
static constexpr u64 AE_DEC_PC      = (1 << 7);   // Decrement PC by 2 in stack frame
static constexpr u64 AE_INC_A       = (1 << 8);   // Increment ADDR by 2 in stack frame
static constexpr u64 AE_DEC_A       = (1 << 9);   // Decrement ADDR by 2 in stack frame
static constexpr u64 AE_SET_CB3     = (1 << 10);  // Set bit 3 in CODE segment
static constexpr u64 AE_SET_RW      = (1 << 11);  // Set bit 8 in the special status word (68010)
static constexpr u64 AE_SET_DF      = (1 << 12);  // Set bit 12 in the special status word (68010)
static constexpr u64 AE_SET_IF      = (1 << 13);  // Set bit 13 in the special status word (68010)

// Timing flags
static constexpr u64 IMPL_DEC       = (1 << 14);  // Omit 2 cycle delay in -(An) mode


//
// Exceptions
//

struct AddressError : public std::exception {

    StackFrame stackFrame;
    AddressError(const StackFrame frame) { stackFrame = frame; }
};

struct IllegalInstruction : public std::exception { };
struct BusErrorException : public std::exception { };
struct DoubleFault : public std::exception { };

}
