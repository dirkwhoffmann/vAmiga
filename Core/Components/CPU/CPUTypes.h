// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {


//
// Enumerations
//

/// CPU revision.
enum class CPURev : long
{
    CPU_68000,
    CPU_68010,
    CPU_68EC020,
    CPU_68020,
    CPU_68EC030,
    CPU_68030,
    CPU_68EC040,
    CPU_68LC040,
    CPU_68040
};

struct CPURevEnum : utl::Reflectable<CPURevEnum, CPURev>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(CPURev::CPU_68040);
    
    static const char *_key(CPURev value)
    {
        switch (value) {
                
            case CPURev::CPU_68000:     return "68000";
            case CPURev::CPU_68010:     return "68010";
            case CPURev::CPU_68EC020:   return "68EC020";
            case CPURev::CPU_68020:     return "68020";
            case CPURev::CPU_68EC030:   return "68EC030";
            case CPURev::CPU_68030:     return "68030";
            case CPURev::CPU_68EC040:   return "68EC040";
            case CPURev::CPU_68LC040:   return "68LC040";
            case CPURev::CPU_68040:     return "68040";
        }
        return "???";
    }
    static const char *help(CPURev value)
    {
        switch (value) {
                
            case CPURev::CPU_68000:     return "Motorola 68000";
            case CPURev::CPU_68010:     return "Motorola 68010";
            case CPURev::CPU_68EC020:   return "Motorola 68EC020";
            case CPURev::CPU_68020:     return "Motorola 68020";
            case CPURev::CPU_68EC030:   return "Motorola 68EC030";
            case CPURev::CPU_68030:     return "Motorola 68030";
            case CPURev::CPU_68EC040:   return "Motorola 68EC040";
            case CPURev::CPU_68LC040:   return "Motorola 68LC040";
            case CPURev::CPU_68040:     return "Motorola 68040";
        }
        return "???";
    }
};

/// CPU revision the disassembler targets.
enum class DasmRev : long
{
    DASM_68000,
    DASM_68010,
    DASM_68EC020,
    DASM_68020,
    DASM_68EC030,
    DASM_68030,
    DASM_68EC040,
    DASM_68LC040,
    DASM_68040
};

struct DasmRevEnum : utl::Reflectable<DasmRevEnum, DasmRev>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DasmRev::DASM_68040);
    
    static const char *_key(DasmRev value)
    {
        switch (value) {
                
            case DasmRev::DASM_68000:    return "68000";
            case DasmRev::DASM_68010:    return "68010";
            case DasmRev::DASM_68EC020:  return "68EC020";
            case DasmRev::DASM_68020:    return "68020";
            case DasmRev::DASM_68EC030:  return "68EC030";
            case DasmRev::DASM_68030:    return "68030";
            case DasmRev::DASM_68EC040:  return "68EC040";
            case DasmRev::DASM_68LC040:  return "68LC040";
            case DasmRev::DASM_68040:    return "68040";
        }
        return "???";
    }
    static const char *help(DasmRev value)
    {
        switch (value) {
                
            case DasmRev::DASM_68000:    return "Motorola 68000";
            case DasmRev::DASM_68010:    return "Motorola 68010";
            case DasmRev::DASM_68EC020:  return "Motorola 68EC020";
            case DasmRev::DASM_68020:    return "Motorola 68020";
            case DasmRev::DASM_68EC030:  return "Motorola 60EC0302";
            case DasmRev::DASM_68030:    return "Motorola 68030";
            case DasmRev::DASM_68EC040:  return "Motorola 68EC040";
            case DasmRev::DASM_68LC040:  return "Motorola 68LC040";
            case DasmRev::DASM_68040:    return "Motorola 68040";
        }
        return "???";
    }
};

/// Syntax style of the disassembler.
enum class DasmSyntax : long
{
    MOIRA,
    MOIRA_MIT,
    GNU,
    GNU_MIT,
    MUSASHI
};

struct DasmSyntaxEnum : utl::Reflectable<DasmSyntaxEnum, DasmSyntax>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DasmSyntax::MUSASHI);
    
    static const char *_key(DasmSyntax value)
    {
        switch (value) {
                
            case DasmSyntax::MOIRA:      return "MOIRA";
            case DasmSyntax::MOIRA_MIT:  return "MOIRA_MIT";
            case DasmSyntax::GNU:        return "GNU";
            case DasmSyntax::GNU_MIT:    return "GNU_MIT";
            case DasmSyntax::MUSASHI:    return "MUSASHI";
        }
        return "???";
    }
    static const char *help(DasmSyntax value)
    {
        switch (value) {
                
            case DasmSyntax::MOIRA:      return "Default style";
            case DasmSyntax::MOIRA_MIT:  return "Moira MIT style";
            case DasmSyntax::GNU:        return "GNU style";
            case DasmSyntax::GNU_MIT:    return "GNU MIT style";
            case DasmSyntax::MUSASHI:    return "Musashi style";
        }
        return "???";
    }
};

/// Number format of the disassembler.
enum class DasmNumbers : long
{
    HEX,
    DEC
};

struct DasmNumbersEnum : utl::Reflectable<DasmNumbersEnum, DasmNumbers>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DasmNumbers::DEC);
    
    static const char *_key(DasmNumbers value)
    {
        switch (value) {
                
            case DasmNumbers::HEX:      return "HEX";
            case DasmNumbers::DEC:      return "DEC";
        }
        return "???";
    }
    static const char *help(DasmNumbers value)
    {
        switch (value) {
                
            case DasmNumbers::HEX:      return "Hexadecimal";
            case DasmNumbers::DEC:      return "Decimal";
        }
        return "???";
    }
};

/// Coprocessor
enum class Copro : long
{
    CP_NONE,
    CP_68881,
    CP_68882
};

struct CoproEnum : utl::Reflectable<CoproEnum, Copro>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Copro::CP_68882);
    
    static const char *_key(Copro value)
    {
        switch (value) {
                
            case Copro::CP_NONE:       return "NONE";
            case Copro::CP_68881:      return "68881";
            case Copro::CP_68882:      return "68882";
        }
        return "???";
    }
    static const char *help(Copro value)
    {
        switch (value) {

            case Copro::CP_NONE:       return "NONE";
            case Copro::CP_68881:      return "M68881 FPU";
            case Copro::CP_68882:      return "M68882 FPU";
        }
        return "???";
    }
};


//
// Structures
//

/// Configuration options of the CPU.
typedef struct
{
    CPURev revision;                        ///< Chip revision
    Copro copro;                            ///< Coprocessor
    DasmRev dasmRevision;                   ///< Chip revision used by the disassembler
    DasmSyntax dasmSyntax;                  ///< Disassembler syntax
    DasmNumbers dasmNumbers;                ///< Disassembler number format
    isize overclocking;                     ///< Overclocking factor
    u32 regResetVal;                        ///< Register reset value
    isize startDelay;                       ///< Delay before the CPU starts (in CPU cycles)
}
CPUConfig;

/// State of the CPU (registers and flags).
typedef struct
{
    Cycle clock;                            ///< CPU clock (in master cycles)
    
    u32 pc0;                                ///< Address of the current instruction
    u16 ird;                                ///< Instruction register (decoder)
    u16 irc;                                ///< Instruction register (prefetch queue)
    u32 d[8];                               ///< Data registers D0 - D7
    u32 a[8];                               ///< Address registers A0 - A7
    u32 isp;                                ///< Interrupt stack pointer
    u32 usp;                                ///< User stack pointer
    u32 msp;                                ///< Master stack pointer
    u32 vbr;                                ///< Vector base register
    u16 sr;                                 ///< Status register
    u8 sfc;                                 ///< Source function code register
    u8 dfc;                                 ///< Destination function code register
    u8 cacr;                                ///< Cache control register
    u8 caar;                                ///< Cache address register
    u8 ipl;                                 ///< Interrupt priority level on the interrupt lines
    u8 fc;                                  ///< Function code pins
    
    bool halt;                              ///< The CPU is halted
}
CPUInfo;

}
