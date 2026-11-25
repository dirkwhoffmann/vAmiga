// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraConfig.h"
#include <bit>
#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>
#include <stdexcept>

namespace MOIRA_NAMESPACE {

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
using isize = std::ptrdiff_t;


//
// Address sizes for various instructions.
//

using Size = u8;
static constexpr Size Unsized    = 0;  ///< No specific size.
static constexpr Size Byte       = 1;  ///< Byte addressing (.b)
static constexpr Size Word       = 2;  ///< Word addressing (.w)
static constexpr Size Long       = 4;  ///< Long word addressing (.l)
static constexpr Size Quad       = 8;  ///< Quad word (FPU)
static constexpr Size Extended   = 12; ///< Extended precision (FPU)


//
// Port sizes, as reported by the DSACK pins (68020)
//

/** A slave device tells the CPU how wide its data port is by asserting DSACK0
 * and DSACK1. Both signals are active low, which gives the following encoding
 * (68020 User's Manual, table 5-2):
 *
 *     DSACK1  DSACK0
 *        1       1     No acknowledge yet, wait states are inserted
 *        1       0     8 bit port
 *        0       1     16 bit port
 *        0       0     32 bit port
 *
 * Bit 1 of a DSACK value holds DSACK1, bit 0 holds DSACK0. The CPU splits
 * every memory access into as many bus cycles as the reported port width
 * requires.
 */
static constexpr u8 DSACK_32   = 0b00;   // Both DSACK lines asserted
static constexpr u8 DSACK_16   = 0b01;   ///< DSACK1 asserted
static constexpr u8 DSACK_8    = 0b10;   ///< DSACK0 asserted
static constexpr u8 DSACK_WAIT = 0b11;   ///< Neither line asserted

/** Returns the width of the addressed port in bytes.
 *
 * DSACK_WAIT means the device has not acknowledged yet. Moira does not model
 * wait states, so it is treated like the narrowest port, which is the most
 * conservative assumption in terms of bus cycles.
 */
static constexpr int portSize(u8 dsack)
{
    return dsack == DSACK_32 ? 4 : dsack == DSACK_16 ? 2 : 1;
}


//
// Enumerations
//

/// CPU variants
enum class CPUModel
{
    M68000,                 ///< Cycle-exact emulation
    M68010,                 ///< Cycle-exact emulation
    M68EC020,               ///< Non-cycle exact emulation
    M68020,                 ///< Non-cycle exact emulation
    M68EC030,               ///< Non-cycle exact emulation (68020 core, no MMU)
    M68030,                 ///< Non-cycle exact emulation (68020 core, 68030 MMU)
    M68EC040,               ///< Non-cycle exact emulation (68020 core, no FPU, no MMU)
    M68LC040,               ///< Non-cycle exact emulation (68020 core, no FPU, 68040 MMU)
    M68040                  ///< Non-cycle exact emulation (68020 core, FPU, 68040 MMU)
};

/// Coprocessors that are connected to the coprocessor interface
/// @details The FPU of the 68040 is part of the CPU and not selected here. A
///          coprocessor can be connected to CPUs with a coprocessor interface
///          (68EC020, 68020, 68EC030, 68030). The selection has no effect on
///          other CPU models.
enum class CPModel
{
    NONE,                   ///< No coprocessor
    M68881,                 ///< MC68881 floating-point coprocessor
    M68882                  ///< MC68882 floating-point coprocessor
};

/// Checks if a CPU model provides a coprocessor interface
static inline constexpr bool hasCPI(CPUModel model)
{
    return model == CPUModel::M68EC020 || model == CPUModel::M68020 ||
           model == CPUModel::M68EC030 || model == CPUModel::M68030;
}

/// Checks if a CPU model has a built-in memory management unit
static inline constexpr bool hasMMU(CPUModel model)
{
    return model == CPUModel::M68030 || model == CPUModel::M68LC040 ||
           model == CPUModel::M68040;
}

/// Checks if a CPU model has a built-in floating-point unit
static inline constexpr bool hasFPU(CPUModel model)
{
    return model == CPUModel::M68040;
}

/// Checks if a CPU model supports a stack frame format
/// @param format The upper four bits of the format word
/// @details The 68000 doesn't write a format word. Its frames count as format 0000.
static inline constexpr bool supportsFrameFormat(CPUModel model, u16 format)
{
    switch (model) {

        case CPUModel::M68000:

            return format == 0b0000;

        case CPUModel::M68010:

            return
            format == 0b0000 ||     // Short frame
            format == 0b1000;       // Long frame

        case CPUModel::M68EC040:
        case CPUModel::M68LC040:
        case CPUModel::M68040:

            return
            format == 0b0000 ||     // Standard frame
            format == 0b0001 ||     // Throwaway frame
            format == 0b0010 ||     // Trap frame
            format == 0b0011 ||     // Floating-point post-instruction frame
            format == 0b0100 ||     // Unimplemented floating-point instruction frame
            format == 0b0111;       // Access error frame

        default:

            return
            format == 0b0000 ||     // Standard frame
            format == 0b0001 ||     // Throwaway frame
            format == 0b0010 ||     // Trap frame
            format == 0b1001 ||     // Coprocessor mid-instruction frame
            format == 0b1010 ||     // Short bus fault frame
            format == 0b1011;       // Long bus fault frame
    }
}

/// Execution cores
enum class Core
{
    C68000,                 ///< Used by M68000
    C68010,                 ///< Used by M68010
    C68020                  ///< Used by all other models
};

/// Compatibility modes (selected at compile time via MOIRA_COMPATIBILITY)
enum class Compatibility
{
    DEFAULT,                ///< Moira's own behavior
    MUSASHI,                ///< Adopt the timing of the Musashi emulator
    UAE,                    ///< Mimic the UAE Amiga emulator (WinUAE, Amiberry)
    MAME                    ///< Mimic the MAME 68000 core (used by the SingleStepTests)
};

inline constexpr Compatibility compat = Compatibility::MOIRA_COMPATIBILITY;

/// Syntax styles for disassembly output
enum class Syntax
{
    MOIRA,                  ///< My personal favorite
    MOIRA_MIT,
    GNU,                    ///< Legacy styles (for testing)
    GNU_MIT,                ///< Legacy styles (for testing)
    MUSASHI,                ///< Musashi compatibility
    UAE                     ///< UAE compatibility (used for testing)
};

/// Letter case preferences for disassembly output
enum class LetterCase
{
    MIXED_CASE,             ///< Determined by the selected syntax
    LOWER_CASE,             ///< Everything in lowercase
    UPPER_CASE              ///< Everything in uppercase
};

/// Processor instructions
enum class Instr : u16
{
    /// 68000 instructions
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
    
    /// 68010 instructions
    BKPT,       MOVEC,      MOVES,      RTD,
    
    /// 68020 instructions
    BFCHG,      BFCLR,      BFEXTS,     BFEXTU,     BFFFO,      BFINS,
    BFSET,      BFTST,      CALLM,      CAS,        CAS2,       CHK2,
    CMP2,       cpBcc,      cpDBcc,     cpGEN,      cpRESTORE,  cpSAVE,
    cpScc,      cpTRAPcc,   DIVL,       EXTB,       MULL,       PACK,
    RTM,        TRAPCC,     TRAPCS,     TRAPEQ,     TRAPGE,     TRAPGT,
    TRAPHI,     TRAPLE,     TRAPLS,     TRAPLT,     TRAPMI,     TRAPNE,
    TRAPPL,     TRAPVC,     TRAPVS,     TRAPF,      TRAPT,      UNPK,
    
    /// 68040 instructions
    CINV,       CPUSH,      MOVE16,
    
    /// MMU instructions
    PFLUSH,     PFLUSHA,    PFLUSHAN,   PFLUSHN,
    PLOAD,      PMOVE,      PTEST,
    
    /// FPU instructions (68040 and 6888x)
    FABS,       FADD,       FBcc,       FCMP,       FDBcc,      FDIV,
    FMOVE,      FMOVEM,     FMUL,       FNEG,       FNOP,       FRESTORE,
    FSAVE,      FScc,       FSQRT,      FSUB,       FTRAPcc,    FTST,
    
    /// FPU instructions (68040 only)
    FSABS,      FDABS,      FSADD,      FDADD,      FSDIV,      FDDIV,
    FSMOVE,     FDMOVE,     FSMUL,      FDMUL,      FSNEG,      FDNEG,
    FSSQRT,     FDSQRT,     FSSUB,      FDSUB,
    
    /// FPU instructions (6888x only)
    FACOS,      FASIN,      FATAN,      FATANH,     FCOS,       FCOSH,
    FETOX,      FETOXM1,    FGETEXP,    FGETMAN,    FINT,       FINTRZ,
    FLOG10,     FLOG2,      FLOGN,      FLOGNP1,    FMOD,       FMOVECR,
    FREM,       FSCAL,      FSGLDIV,    FSGLMUL,    FSIN,       FSINCOS,
    FSINH,      FTAN,       FTANH,      FTENTOX,    FTWOTOX,
    
    /// Loop mode variants (68010)
    ABCD_LOOP,  ADD_LOOP,   ADDA_LOOP,  ADDX_LOOP,  AND_LOOP,   ASL_LOOP,
    ASR_LOOP,   CLR_LOOP,   CMP_LOOP,   CMPA_LOOP,  CMPM_LOOP,  DBCC_LOOP,
    DBCS_LOOP,  DBEQ_LOOP,  DBGE_LOOP,  DBGT_LOOP,  DBHI_LOOP,  DBLE_LOOP,
    DBLS_LOOP,  DBLT_LOOP,  DBMI_LOOP,  DBNE_LOOP,  DBPL_LOOP,  DBVC_LOOP,
    DBVS_LOOP,  DBF_LOOP,   DBT_LOOP,   EOR_LOOP,   LSL_LOOP,   LSR_LOOP,
    MOVE_LOOP,  NBCD_LOOP,  NEG_LOOP,   NEGX_LOOP,  NOT_LOOP,   OR_LOOP,
    ROL_LOOP,   ROR_LOOP,   ROXL_LOOP,  ROXR_LOOP,  SBCD_LOOP,  SUB_LOOP,
    SUBA_LOOP,  SUBX_LOOP,  TST_LOOP
};

/// Checks if an instruction is a loop mode instruction (68010)
template <Instr I> constexpr bool looping()
{
    return I >= Instr::ABCD_LOOP && I <= Instr::TST_LOOP;
}

/// Addressing modes
enum class Mode : u8
{
    DN,                 ///<  0: Dn, Data register direct.
    AN,                 ///<  1: An, Address register direct.
    AI,                 ///<  2: (An), Address register indirect.
    PI,                 ///<  3: (An)+, Address register indirect with post-increment.
    PD,                 ///<  4: -(An), Address register indirect with pre-decrement.
    DI,                 ///<  5: (d,An), Address register indirect with displacement.
    IX,                 ///<  6: (d,An,Xi), Address register indirect with index.
    AW,                 ///<  7: (####).w, Absolute short addressing.
    AL,                 ///<  8: (####).l, Absolute long addressing.
    DIPC,               ///<  9: (d,PC), Program counter relative with displacement.
    IXPC,               ///< 10: (d,PC,Xi), Program counter relative with index.
    IM,                 ///< 11: ####, Immediate value.
    IP                  ///< 12: Implied addressing.
};

constexpr bool isRegMode(Mode M) { return M == Mode::DN || M == Mode::AN;  }
constexpr bool isAbsMode(Mode M) { return M == Mode::AW || M == Mode::AL;  }
constexpr bool isIdxMode(Mode M) { return M == Mode::IX || M == Mode::IXPC; }
constexpr bool isMemMode(Mode M) { return M >= Mode::AI && M <= Mode::IXPC; }
constexpr bool isPrgMode(Mode M) { return M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isDspMode(Mode M) { return M == Mode::DI || M == Mode::IX || M == Mode::DIPC || M == Mode::IXPC; }
constexpr bool isImmMode(Mode M) { return M == Mode::IM; }

/// Condition codes used in conditional instructions
enum class Cond
{
    BT,                     ///< Always true
    BF,                     ///< Always false
    HI,                     ///< Higher than
    LS,                     ///< Lower or same
    CC,                     ///< Carry clear
    CS,                     ///< Carry set
    NE,                     ///< Not equal
    EQ,                     ///< Equal
    VC,                     ///< Overflow clear
    VS,                     ///< Overflow set
    PL,                     ///< Plus
    MI,                     ///< Minus
    GE,                     ///< Greater or equal
    LT,                     ///< Less than
    GT,                     ///< Greater than
    LE                      ///< Less than or equal
};

/// Enumeration of Motorola 68k CPU exceptions
enum class M68kException
{
    RESET             = 1,    ///< CPU reset exception
    BUS_ERROR         = 2,    ///< Bus error
    ADDRESS_ERROR     = 3,    ///< Address error
    ILLEGAL           = 4,    ///< Illegal instruction
    DIVIDE_BY_ZERO    = 5,    ///< Division by zero
    CHK               = 6,    ///< CHK instruction exception
    TRAPV             = 7,    ///< TRAPV instruction exception
    PRIVILEGE         = 8,    ///< Privilege violation
    TRACE             = 9,    ///< Trace exception
    LINEA             = 10,   ///< Line A emulator trap
    LINEF             = 11,   ///< Line F emulator trap
    FORMAT_ERROR      = 14,   ///< Stack frame format error
    IRQ_UNINITIALIZED = 15,   ///< Uninitialized interrupt request
    IRQ_SPURIOUS      = 24,   ///< Spurious interrupt
    TRAP              = 32,   ///< TRAP instruction exception
    
    BKPT,                       ///< Breakpoint (maps to a native exception when triggered)
    FPU_UNIMPLEMENTED,          ///< Unimplemented floating-point instruction (68040, vector 11)
    FPU_ARITHMETIC              ///< Floating-point arithmetic exception (vectors 48 - 54)
};

/// Answers to an interrupt acknowledge cycle
enum class IrqMode
{
    AUTO,                       ///< The autovector 24 + level is used
    USER,                       ///< The device supplies the vector number
    SPURIOUS,                   ///< No device responds (spurious interrupt, vector 24)
    UNINITIALIZED               ///< The device is not initialized (vector 15)
};

/// Types of CPU space cycles
/// @details A CPU space cycle is a bus cycle with all function code pins (FC2-FC0)
///          set. The type of the cycle is driven on address lines A19-A16. Moira
///          reports the cycles that have an observable effect on the system.
///          Coprocessor cycles are reported for the coprocessor interface of the
///          68020 and 68030. Only the first cycle of a coprocessor protocol is
///          reported, i.e., the access that decides whether a coprocessor
///          responds. On a system without a coprocessor, this cycle ends with a
///          bus error that the CPU turns into a line-F exception. Further
///          accesses to the interface registers are not reported. The built-in
///          FPU and MMU of the 68030 and 68040 cause no external cycles.
enum class CpuSpaceType : u8
{
    BREAKPOINT   = 0x0,         ///< Breakpoint acknowledge (BKPT instruction)
    COPROCESSOR  = 0x2,         ///< Coprocessor access (first cycle of the protocol)
    IRQ_ACK      = 0xF          ///< Interrupt acknowledge
};

/// Coprocessor interface registers (CIR), as selected by address lines A4-A0
enum class CIR : u8
{
    RESPONSE  = 0x00,           ///< Response (read)
    CONTROL   = 0x02,           ///< Control
    SAVE      = 0x04,           ///< Save
    RESTORE   = 0x06,           ///< Restore
    OPERATION = 0x08,           ///< Operation word
    COMMAND   = 0x0A,           ///< Command (written by general instructions)
    CONDITION = 0x0E            ///< Condition (written by conditional instructions)
};

/// Address spaces
enum class AddrSpace
{
    DATA = 1,                   ///< Data space
    PROG = 2                    ///< Program space
};


//
// Structures
//

/// The stack frame that is written when an exception is taken
struct StackFrame
{
    u16 code;                   ///< Access information (address error frames)
    u32 addr;                   ///< Fault address
    u16 ird;                    ///< Instruction register
    u16 sr;                     ///< Status register
    u32 pc;                     ///< Program counter
    
    u16 fc;                     ///< Function code
    u16 ssw;                    ///< Special status word (68010)

    bool write;                 ///< Set if the faulting bus access was a write (68020+)
    u32 data;                   ///< Data of a faulting write access (68020+)
};

/// The status register (SR)
struct StatusRegister {
    
    bool t1;                    ///< Trace flag
    bool t0;                    ///< Trace flag         (68020 only)
    bool s;                     ///< Supervisor flag
    bool m;                     ///< Master flag        (68020 only)
    bool x;                     ///< Extend flag
    bool n;                     ///< Negative flag
    bool z;                     ///< Zero flag
    bool v;                     ///< Overflow flag
    bool c;                     ///< Carry flag
    
    u8 ipl;                     ///< Required Interrupt Priority Level
};

/// The register set of the CPU
struct Registers {
    
    u32 pc;                     ///< Program counter (address of the word in IRC)
    u32 pc0;                    ///< Beginning of the current instruction
    u32 prev_pc0;               ///< Beginning of the previous instruction

    StatusRegister sr;          ///< Status register
    
    union {
        struct {
            u32 d[8];           ///< D0, D1 ... D7
            u32 a[8];           ///< A0, A1 ... A7
        };
        struct {
            u32 r[16];          ///< D0, D1 ... D7, A0, A1 ... A7
        };
        struct {
            u32 _pad[15];
            u32 sp;             ///< Visible stack pointer (overlays a[7])
        };
    };
    
    u32 usp;                    ///< User Stack Pointer
    u32 isp;                    ///< Interrupt Stack Pointer
    u32 msp;                    ///< Master Stack Pointer             (68020+)
    
    u8 ipl;                     ///< Polled Interrupt Priority Level
    
    u32 vbr;                    ///< Vector Base Register             (68010+)
    u8 sfc;                     ///< Source Function Code             (68010+)
    u8 dfc;                     ///< Destination Function Code        (68010+)
    
    /// Unemulated registers
    u32 cacr;                   ///< Cache Control Register           (68020+)
    u32 caar;                   ///< Cache Address Register           (68020+)
};

/// The prefetch queue of the CPU
struct PrefetchQueue {
    
    u16 edb;                    ///< The external data bus latch (feeds the IRC)
    u16 irc;                    ///< The next (prefetched) instruction (feeds the IRD)
    u16 ird;                    ///< The currently executed instruction
};


/// Restart information of a faulted instruction (68020 in UAE mode)
/// The handler sees the registers as they were when the fault occurred (as on the
/// real CPU, which continues from internal state). If it returns without touching
/// the frame, RTE rolls the instruction back, so that it is executed again from its start.
struct Restart {

    bool valid;
    Registers pre;              ///< Registers at the start of the instruction
    Registers fault;            ///< Registers at the time of the fault
    u16 preSr;                  ///< Status register at the start of the instruction
    u32 pc;                     ///< Start of the instruction
    u32 frameSp;                ///< Location of the stacked frame
    u16 frameSr;                ///< Stacked status register
    u32 framePC;                ///< Stacked program counter
    u32 entrySsp;               ///< Supervisor stack pointer before the frame was stacked
    u32 entryUsp;               ///< User stack pointer (while in supervisor mode)
};

/// Bookkeeping of the cycle penalty (68020 only)
struct CycleBudget {

    /// Cycle penalty
    int penalty;

    /// Cycles that have not been spent yet
    int accumulated;

    /// Number of consecutive instructions that did not advance the clock
    int stalled;
};

/// Describes an instruction by its mnemonic, addressing mode and operand size
struct InstrInfo
{
    Instr I;                    ///< Mnemonic
    Mode  M;                    ///< Addressing mode
    Size  S;                    ///< Operand size
};

/// The number format used by the disassembler
struct DasmNumberFormat
{
    const char *prefix;         ///< Prefix for hexidecimal numbers
    u8 radix;                   ///< 10 (decimal) or 16 (hexadecimal)
    bool upperCase;             ///< Lettercase for hexadecimal digits A...F
    bool plainZero;             ///< Determines whether 0 is printed with a prefix

    /// Checks the format for consistency
    /// @throws std::runtime_error if the prefix is NULL or the radix is neither 10 nor 16
    void validate() const
    {
        if (prefix == nullptr) throw std::runtime_error("Prefix must not be NULL");
        if (radix != 10 && radix != 16) throw std::runtime_error("Radix must be 10 or 16");
    }
};

/// The visual style of the disassembler output
struct DasmStyle
{
    Syntax syntax;                  ///< The syntax style
    LetterCase letterCase;          ///< The letter case
    DasmNumberFormat numberFormat;  ///< The number format
    int tab;                        ///< Indentation of the operands

    /// Checks the style for consistency
    /// @throws std::runtime_error if the number format is invalid
    void validate() const { numberFormat.validate(); }
};


//
// Flags and masks
//

/// Condition code register bits
namespace CCR {

static constexpr u8 C = 1 << 0;     ///< Carry
static constexpr u8 V = 1 << 1;     ///< Overflow
static constexpr u8 Z = 1 << 2;     ///< Zero
static constexpr u8 N = 1 << 3;     ///< Negative
static constexpr u8 X = 1 << 4;     ///< Extend
static constexpr u8 ALL = 0x1F;     ///< All condition codes
}

/// Cache control register bits (68020 and 68030)
namespace CACR {

static constexpr u32 E      = 1 << 0;   ///< Enable instruction cache
static constexpr u32 F      = 1 << 1;   ///< Freeze instruction cache
static constexpr u32 CE     = 1 << 2;   ///< Clear instruction cache entry
static constexpr u32 C      = 1 << 3;   ///< Clear instruction cache
static constexpr u32 IBE    = 1 << 4;   ///< Enable instruction burst fills (68030)
static constexpr u32 ED     = 1 << 8;   ///< Enable data cache (68030)
static constexpr u32 FD     = 1 << 9;   ///< Freeze data cache (68030)
static constexpr u32 CED    = 1 << 10;  ///< Clear data cache entry (68030)
static constexpr u32 CD     = 1 << 11;  ///< Clear data cache (68030)
static constexpr u32 DBE    = 1 << 12;  ///< Enable data burst fills (68030)
static constexpr u32 WA     = 1 << 13;  ///< Write allocate (68030)
}

/// Special status word of the 68030 (bus fault stack frames)
namespace SSW030 {

static constexpr u16 FC     = 0x8000;   ///< Fault on stage C of the instruction pipe
static constexpr u16 FB     = 0x4000;   ///< Fault on stage B of the instruction pipe
static constexpr u16 RC     = 0x2000;   ///< Rerun flag for stage C
static constexpr u16 RB     = 0x1000;   ///< Rerun flag for stage B
static constexpr u16 DF     = 0x0100;   ///< Fault on a data cycle
static constexpr u16 RM     = 0x0080;   ///< Read-modify-write cycle
static constexpr u16 RW     = 0x0040;   ///< Read cycle (0 = write cycle)
static constexpr u16 SIZ_B  = 0x0010;   ///< Transfer size: byte
static constexpr u16 SIZ_W  = 0x0020;   ///< Transfer size: word
static constexpr u16 SIZ_L  = 0x0000;   ///< Transfer size: long word
}

namespace SSW040 {

static constexpr u16 MA     = 0x0800;   ///< Misaligned access
static constexpr u16 ATC    = 0x0400;   ///< Fault caused by the address translation
static constexpr u16 LK     = 0x0200;   ///< Locked transfer (read-modify-write)
static constexpr u16 RW     = 0x0100;   ///< Read cycle (0 = write cycle)
static constexpr u16 SIZ_L  = 0x0000;   ///< Transfer size: long word
static constexpr u16 SIZ_B  = 0x0020;   ///< Transfer size: byte
static constexpr u16 SIZ_W  = 0x0040;   ///< Transfer size: word
}

/** Function codes for CPU privilege levels and address space separation.
 *
 * The M68k CPU provides three function code pins (FC2–FC0) indicating the
 * current privilege level and memory access type. FC2 distinguishes between
 * user and supervisor modes, while FC1 and FC0 signal accesses to program
 * and data memory, respectively.
 */
namespace FC {

static constexpr u8 USER_DATA       = 1;  ///< User-mode access to data space
static constexpr u8 USER_PROG       = 2;  ///< User-mode access to program space
static constexpr u8 SUPERVISOR_DATA = 5;  ///< Supervisor access to data space
static constexpr u8 SUPERVISOR_PROG = 6;  ///< Supervisor access to program space
static constexpr u8 CPU_SPACE       = 7;  ///< CPU space cycle
}

/** Availability masks for different CPU models.
 *
 * These masks are utilized to indicate the presence of instructions for
 * specific CPU models.
 */
namespace AV {

static constexpr u16 M68000         = 1 << int(CPUModel::M68000);
static constexpr u16 M68010         = 1 << int(CPUModel::M68010);
static constexpr u16 M68020         = 1 << int(CPUModel::M68EC020) | 1 << int(CPUModel::M68020);
static constexpr u16 M68030         = 1 << int(CPUModel::M68EC030) | 1 << int(CPUModel::M68030);
static constexpr u16 M68040         = 1 << int(CPUModel::M68EC040) | 1 << int(CPUModel::M68LC040) | 1 << int(CPUModel::M68040);
static constexpr u16 MMU            = 1 << int(CPUModel::M68030) | 1 << int(CPUModel::M68LC040) | 1 << int(CPUModel::M68040);
static constexpr u16 FPU            = M68020 | M68030 | 1 << int(CPUModel::M68040);
static constexpr u16 FPU040         = 1 << int(CPUModel::M68040);
static constexpr u16 M68030_UP      = M68030 | M68040;
static constexpr u16 M68020_UP      = M68020 | M68030_UP;
static constexpr u16 M68010_UP      = M68010 | M68020_UP;
static constexpr u16 M68000_UP      = M68000 | M68010_UP;
}

/** CPU Execution Flags
 *
 * These flags control the CPU's execution state and behavior.
 */
namespace State {

/// CPU is in a halted state due to a double fault. Cleared only on reset.
static constexpr int HALTED         = (1 << 0);

/// CPU is stopped after executing a STOP instruction. Cleared on the next interrupt.
static constexpr int STOPPED        = (1 << 1);

/// CPU is in loop mode. Loop mode is a 68010 feature for optimizing DBcc loops.
static constexpr int LOOPING        = (1 << 2);

/// Enables instruction logging. Register states are stored in a ring buffer.
static constexpr int LOGGING        = (1 << 3);

/// Reflects the T flag from the status register. Used to speed up emulation.
static constexpr int TRACING        = (1 << 4);

/// Triggers a trace exception when set.
static constexpr int TRACE_EXC      = (1 << 5);

/// CPU checks for pending interrupts. Cleared when no interrupt is possible.
static constexpr int CHECK_IRQ      = (1 << 6);

/// Enables checking for breakpoints.
static constexpr int CHECK_BP       = (1 << 7);

/// Enables checking for watchpoints.
static constexpr int CHECK_WP       = (1 << 8);

/// Enables checking for catchpoints.
static constexpr int CHECK_CP       = (1 << 9);

/// Enables register saving, so that a faulted instruction can be restarted.
static constexpr int SAVE_REGS      = (1 << 10);

/// Set while address translation is active.
static constexpr int MMU            = (1 << 11);

}

/** Instruction Flags
 *
 * The M68k is a well organized processor that breaks down the execution of
 * an instruction to a limited number of general execution schemes. However,
 * the schemes slighty differ between instruction. To take care of the subtle
 * differences, some functions take an additional 'flags' argument to alter
 * their behavior. All flags are passed as a template parameter for efficiency.
 */
typedef u64 Flags;

namespace Flag {

/// Memory access flags
static constexpr u64 REVERSE        = (1 << 0);   // Reverse the long word access order (MAME: reads, too)
static constexpr u64 SKIP_LAST_RD   = (1 << 1);   ///< Don't read the extension word

/// Interrupt flags
static constexpr u64 POLL           = (1 << 2);   // Poll the interrupt lines

/// Address error flags
static constexpr u64 AE_WRITE       = (1 << 3);   // Clear read flag in code word
static constexpr u64 AE_PROG        = (1 << 4);   ///< Set FC pins to program space
static constexpr u64 AE_DATA        = (1 << 5);   ///< Set FC pins to user space
static constexpr u64 AE_INC_PC      = (1 << 6);   ///< Increment PC by 2 in stack frame
static constexpr u64 AE_DEC_PC      = (1 << 7);   ///< Decrement PC by 2 in stack frame
static constexpr u64 AE_INC_A       = (1 << 8);   ///< Increment ADDR by 2 in stack frame
static constexpr u64 AE_DEC_A       = (1 << 9);   ///< Decrement ADDR by 2 in stack frame
static constexpr u64 AE_SET_CB3     = (1 << 10);  ///< Set bit 3 in CODE segment
static constexpr u64 AE_FETCH       = (1 << 11);  ///< The fault hit an instruction fetch (IF bit of the 68010 special status word)

/// Bus error flags
static constexpr u64 UAE_DEC_PC     = (1 << 12);  ///< UAE stacks the PC of the executing instruction if the access fails (68000)

/// Timing flags
static constexpr u64 IMPL_DEC       = (1 << 14);  // Omit 2 cycle delay in -(An) mode
static constexpr u64 DEC_LATE       = (1 << 13);  ///< -(An) is decremented after the first word of a long operand has been read (68000)

/// Extension word flags
static constexpr u64 NO_DEC_PC      = (1 << 15);  ///< A failing extension word fetch stacks the PC of the next word (68000)

/// Flags of memory accesses that follow an early prefetch (see fetchIrc)
static constexpr u64 COMMIT_IRD    = (1 << 16);  ///< The access contains the last bus cycle of the instruction. The IRD is replaced
                                                 ///< when that cycle starts (after the address error check, before the second half
                                                 ///< of a split access). Faults before that point stack the old IRD.
                                                 ///< Only the 68000 defers the IRD update (see commitIrd).

/// Returns the flags for completing a bus error stack frame. The ADDR adjustments of address error frames don't apply.
template <Flags F> constexpr Flags beFrameFlags() {
    return (F & ~(AE_INC_A | AE_DEC_A)) | ((F & UAE_DEC_PC) && compat == Compatibility::UAE ? AE_DEC_PC : 0);
}
}

/// Version number of the 68010. It is stored in bits 10 to 13 of the first internal information word of a
/// format $8 frame (recorded on a real 68010: vAmigaTS, CPU/Exceptions/StackFrame/addrerr37 and addrerr38).
static constexpr u16 VERSION_68010 = 2;

//
// Exceptions
//

/// Thrown when an illegal instruction is executed
struct IllegalInstruction : public std::exception { };

/// Thrown when an exception occurs while another exception is processed
struct DoubleFault : public std::exception { };

/// Thrown when a word or long word is accessed at an odd address
struct AddressError : public std::exception {
    
    StackFrame stackFrame;
    AddressError(const StackFrame frame) { stackFrame = frame; }
};

/** Thrown when a bus error occurs
 *
 * A bus error can be signaled in two ways. If a host callback (read8, read16,
 * write8, ...) throws a BusError that only carries the fault address, Moira
 * completes the stack frame itself. This is the preferred way, because the
 * frame content (stacked PC, instruction register, status register and access
 * information) depends on the executed instruction and its addressing mode.
 * Alternatively, the host can create the complete stack frame and pass it to
 * the constructor that takes a StackFrame.
 */
struct BusError : public std::exception {
    
    StackFrame stackFrame;

    /// Indicates whether the stack frame is complete
    bool complete = true;

    /// Indicates whether the stack frame has been supplied by the host. Moira never modifies such a frame.
    bool custom = false;

    /// Indicates which half of a word-split long access has failed (true = upper word at ea, false = lower word at ea + 2)
    bool upperWord = false;

    /// Creates a bus error with a complete stack frame (supplied by the host unless 'custom' is false)
    BusError(const StackFrame frame, bool custom = true) { stackFrame = frame; this->custom = custom; }

    /// Creates a bus error that only carries the fault address
    BusError(u32 addr) { stackFrame = StackFrame(); stackFrame.addr = addr; complete = false; }
};

}
