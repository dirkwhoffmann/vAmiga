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
// Basic data types
//

using i8  = int8_t;
using i16 = int16_t;             ;
using i32 = int32_t;
using i64 = long long;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = unsigned long long;


//
// Address sizes for various instructions.
//

using Size = int;
static constexpr int Unsized     = 0;  // No specific size.
static constexpr int Byte        = 1;  // Byte addressing (.b)
static constexpr int Word        = 2;  // Word addressing (.w)
static constexpr int Long        = 4;  // Long word addressing (.l)
static constexpr int Quad        = 8;  // Quad word (FPU)
static constexpr int Extended    = 12; // Extended precision (FPU)


//
// Enumerations
//

// CPU variants
enum class Model
{
    M68000,                 // Cycle-exact emulation
    M68010,                 // Cycle-exact emulation
    M68EC020,               // Non-cycle exaxt emulation
    M68020,                 // Non-cycle exaxt emulation
    M68EC030,               // Disassembler only
    M68030,                 // Disassembler only
    M68EC040,               // Disassembler only
    M68LC040,               // Disassembler only
    M68040                  // Disassembler only
};

// Execution cores
enum class Core
{
    C68000,                 // Used by M68000
    C68010,                 // Used by M68010
    C68020                  // Used by all other models
};

// Syntax styles for disassembly output
enum class Syntax
{
    MOIRA,                  // My personal favorite
    MOIRA_MIT,
    GNU,                    // Legacy styles (for testing)
    GNU_MIT,                // Legacy styles (for testing)
    MUSASHI                 // Musashi compatibility
};

// Letter case preferences for disassembly output
enum class LetterCase
{
    MIXED_CASE,             // Determined by the selected syntax
    LOWER_CASE,             // Everything in lowercase
    UPPER_CASE              // Everything in uppercase
};

// Processor instructions
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

// Addressing modes
enum class Mode
{
    DN,                 //  0: Dn, Data register direct.
    AN,                 //  1: An, Address register direct.
    AI,                 //  2: (An), Address register indirect.
    PI,                 //  3: (An)+, Address register indirect with post-increment.
    PD,                 //  4: -(An), Address register indirect with pre-decrement.
    DI,                 //  5: (d,An), Address register indirect with displacement.
    IX,                 //  6: (d,An,Xi), Address register indirect with index.
    AW,                 //  7: (####).w, Absolute short addressing.
    AL,                 //  8: (####).l, Absolute long addressing.
    DIPC,               //  9: (d,PC), Program counter relative with displacement.
    IXPC,               // 10: (d,PC,Xi), Program counter relative with index.
    IM,                 // 11: ####, Immediate value.
    IP                  // 12: Implied addressing.
};

constexpr bool isRegMode(Mode M) { return M == Mode::DN || M == Mode::AN;  }
constexpr bool isAbsMode(Mode M) { return M == Mode::AW || M == Mode::AL;  }
constexpr bool isIdxMode(Mode M) { return M == Mode::IX || M == Mode::IXPC; }
constexpr bool isMemMode(Mode M) { return M >= Mode::AI && M <= Mode::IXPC; }
constexpr bool isPrgMode(Mode M) { return M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isDspMode(Mode M) { return M == Mode::DI || M == Mode::IX || M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isImmMode(Mode M) { return M == Mode::IM; }

// Condition codes used in conditional instructions
enum class Cond
{
    BT,                     // Always true
    BF,                     // Always false
    HI,                     // Higher than
    LS,                     // Lower or same
    CC,                     // Carry clear
    CS,                     // Carry set
    NE,                     // Not equal
    EQ,                     // Equal
    VC,                     // Overflow clear
    VS,                     // Overflow set
    PL,                     // Plus
    MI,                     // Minus
    GE,                     // Greater or equal
    LT,                     // Less than
    GT,                     // Greater than
    LE                      // Less than or equal
};

// Enumeration of Motorola 68k CPU exceptions
enum class M68kException
{
    RESET             = 1,    // CPU reset exception
    BUS_ERROR         = 2,    // Bus error
    ADDRESS_ERROR     = 3,    // Address error
    ILLEGAL           = 4,    // Illegal instruction
    DIVIDE_BY_ZERO    = 5,    // Division by zero
    CHK               = 6,    // CHK instruction exception
    TRAPV             = 7,    // TRAPV instruction exception
    PRIVILEGE         = 8,    // Privilege violation
    TRACE             = 9,    // Trace exception
    LINEA             = 10,   // Line A emulator trap
    LINEF             = 11,   // Line F emulator trap
    FORMAT_ERROR      = 14,   // Stack frame format error
    IRQ_UNINITIALIZED = 15,   // Uninitialized interrupt request
    IRQ_SPURIOUS      = 24,   // Spurious interrupt
    TRAP              = 32,   // TRAP instruction exception
    
    BKPT                        // Breakpoint (maps to a native exception when triggered)
};

// M68k interrupt modes
enum class IrqMode
{
    AUTO,                       // Automatic IRQ handling
    USER,                       // User-defined IRQ handling
    SPURIOUS,                   // Spurious IRQ handling
    UNINITIALIZED               // Uninitialized IRQ mode
};

// Address spaces
enum class AddrSpace
{
    DATA = 1,                   // Data space
    PROG = 2                    // Program space
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
    
    u16 fc;                     // Function code
    u16 ssw;                    // Special status word (68010)
};

struct StatusRegister {
    
    bool t1;                    // Trace flag
    bool t0;                    // Trace flag         (68020 only)
    bool s;                     // Supervisor flag
    bool m;                     // Master flag        (68020 only)
    bool x;                     // Extend flag
    bool n;                     // Negative flag
    bool z;                     // Zero flag
    bool v;                     // Overflow flag
    bool c;                     // Carry flag
    
    u8 ipl;                     // Required Interrupt Priority Level
};

struct Registers {
    
    u32 pc;                     // Program counter
    u32 pc0;                    // Beginning of the current instruction
    StatusRegister sr;          // Status register
    
    union {
        struct {
            u32 d[8];           // D0, D1 ... D7
            u32 a[8];           // A0, A1 ... A7
        };
        struct {
            u32 r[16];          // D0, D1 ... D7, A0, A1 ... A7
        };
        struct {
            u32 _pad[15];
            u32 sp;             // Visible stack pointer (overlays a[7])
        };
    };
    
    u32 usp;                    // User Stack Pointer
    u32 isp;                    // Interrupt Stack Pointer
    u32 msp;                    // Master Stack Pointer             (68020+)
    
    u8 ipl;                     // Polled Interrupt Priority Level
    
    u32 vbr;                    // Vector Base Register             (68010+)
    u32 sfc;                    // Source Function Code             (68010+)
    u32 dfc;                    // Destination Function Code        (68010+)
    
    // Unemulated registers
    u32 cacr;                   // Cache Control Register           (68020+)
    u32 caar;                   // Cache Address Register           (68020+)
};

struct PrefetchQueue {
    
    u16 irc;                    // The most recent word prefetched from memory
    u16 ird;                    // The instruction currently being executed
};

struct InstrInfo
{
    Instr I;
    Mode  M;
    Size  S;
};

struct DasmNumberFormat
{
    const char *prefix;         // Prefix for hexidecimal numbers
    u8 radix;                   // 10 (decimal) or 16 (hexadecimal)
    bool upperCase;             // Lettercase for hexadecimal digits A...F
    bool plainZero;             // Determines whether 0 is printed with a prefix
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

/* Function codes for CPU privilege levels and address space separation.
 *
 * The M68k CPU provides three function code pins (FC2–FC0) indicating the
 * current privilege level and memory access type. FC2 distinguishes between
 * user and supervisor modes, while FC1 and FC0 signal accesses to program
 * and data memory, respectively.
 */
namespace FC {


static constexpr u8 USER_DATA       = 1;  // User-mode access to data space
static constexpr u8 USER_PROG       = 2;  // User-mode access to program space
static constexpr u8 SUPERVISOR_DATA = 5;  // Supervisor access to data space
static constexpr u8 SUPERVISOR_PROG = 6;  // Supervisor access to program space
}

/* Availability masks for different CPU models.
 *
 * These masks are utilized to indicate the presence of instructions for
 * specific CPU models.
 */
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

/* CPU Execution Flags
 *
 * These flags control the CPU's execution state and behavior.
 */
namespace State {

// CPU is in a halted state due to a double fault. Cleared only on reset.
static constexpr int HALTED         = (1 << 0);

// CPU is stopped after executing a STOP instruction. Cleared on the next interrupt.
static constexpr int STOPPED        = (1 << 1);

// CPU is in loop mode. Loop mode is a 68010 feature for optimizing DBcc loops.
static constexpr int LOOPING        = (1 << 2);

// Enables instruction logging. Register states are stored in a ring buffer.
static constexpr int LOGGING        = (1 << 3);

// Reflects the T flag from the status register. Used to speed up emulation.
static constexpr int TRACING        = (1 << 4);

// Triggers a trace exception when set.
static constexpr int TRACE_EXC      = (1 << 5);

// CPU checks for pending interrupts only if this flag is set. Cleared when no interrupt is possible.
static constexpr int CHECK_IRQ      = (1 << 6);

// Enables checking for breakpoints.
static constexpr int CHECK_BP       = (1 << 7);

// Enables checking for watchpoints.
static constexpr int CHECK_WP       = (1 << 8);

// Enables checking for catchpoints.
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

namespace Flag {

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
}

//
// Exceptions
//

struct IllegalInstruction : public std::exception { };
struct DoubleFault : public std::exception { };

struct AddressError : public std::exception {
    
    StackFrame stackFrame;
    AddressError(const StackFrame frame) { stackFrame = frame; }
};

struct BusError : public std::exception {
    
    StackFrame stackFrame;
    BusError(const StackFrame frame) { stackFrame = frame; }
};

}
