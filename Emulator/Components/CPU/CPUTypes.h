// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"
#include "RingBuffer.h"

namespace vamiga {

#define CPUINFO_INSTR_COUNT 256


//
// Enumerations
//

enum class CPURevision : long
{
    CPU_68000,
    CPU_68010,
    CPU_68EC020
};

struct CPURevisionEnum : util::Reflection<CPURevisionEnum, CPURevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(CPURevision::CPU_68EC020);
    
    static const char *_key(CPURevision value)
    {
        switch (value) {
                
            case CPURevision::CPU_68000:     return "68000";
            case CPURevision::CPU_68010:     return "68010";
            case CPURevision::CPU_68EC020:   return "68EC020";
        }
        return "???";
    }
    static const char *help(CPURevision value)
    {
        switch (value) {
                
            case CPURevision::CPU_68000:     return "Motorola 68000 CPU";
            case CPURevision::CPU_68010:     return "Motorola 68010 CPU";
            case CPURevision::CPU_68EC020:   return "Motorola 68EC020 CPU";
        }
        return "???";
    }
};

enum class DasmRevision : long
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

struct DasmRevisionEnum : util::Reflection<DasmRevisionEnum, DasmRevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DasmRevision::DASM_68040);
    
    static const char *_key(DasmRevision value)
    {
        switch (value) {
                
            case DasmRevision::DASM_68000:    return "68000";
            case DasmRevision::DASM_68010:    return "68010";
            case DasmRevision::DASM_68EC020:  return "68EC020";
            case DasmRevision::DASM_68020:    return "68020";
            case DasmRevision::DASM_68EC030:  return "68EC030";
            case DasmRevision::DASM_68030:    return "68030";
            case DasmRevision::DASM_68EC040:  return "68EC040";
            case DasmRevision::DASM_68LC040:  return "68LC040";
            case DasmRevision::DASM_68040:    return "68040";
        }
        return "???";
    }
    static const char *help(DasmRevision value)
    {
        switch (value) {
                
            case DasmRevision::DASM_68000:    return "Motorola 68000 instruction set";
            case DasmRevision::DASM_68010:    return "Motorola 68010 instruction set";
            case DasmRevision::DASM_68EC020:  return "Motorola 68EC020 instruction set";
            case DasmRevision::DASM_68020:    return "Motorola 68020 instruction set";
            case DasmRevision::DASM_68EC030:  return "Motorola 68EC030 instruction set";
            case DasmRevision::DASM_68030:    return "Motorola 68030 instruction set";
            case DasmRevision::DASM_68EC040:  return "Motorola 68EC040 instruction set";
            case DasmRevision::DASM_68LC040:  return "Motorola 68LC040 instruction set";
            case DasmRevision::DASM_68040:    return "Motorola 68040 instruction set";
        }
        return "???";
    }
};

enum class DasmSyntax : long
{
    MOIRA,
    MOIRA_MIT,
    GNU,
    GNU_MIT,
    MUSASHI
};

struct DasmSyntaxEnum : util::Reflection<DasmSyntaxEnum, DasmSyntax>
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

enum class DasmNumbers : long
{
    HEX,
    DEC
};

struct DasmNumbersEnum : util::Reflection<DasmNumbersEnum, DasmNumbers>
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
                
            case DasmNumbers::HEX:      return "Hexadecimal numbers";
            case DasmNumbers::DEC:      return "Decimal numbers";
        }
        return "???";
    }
};

enum class GuardType : long
{
    BREAKPOINT,
    WATCHPOINT,
    CATCHPOINT,
};

struct GuardTypeEnum : util::Reflection<GuardTypeEnum, GuardType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(GuardType::CATCHPOINT);
    
    static const char *_key(GuardType value)
    {
        switch (value) {
                
            case GuardType::BREAKPOINT:  return "BREAKPOINT";
            case GuardType::WATCHPOINT:  return "WATCHPOINT";
            case GuardType::CATCHPOINT:  return "CATCHPOINT";
        }
        return "???";
    }
    static const char *help(GuardType value)
    {
        switch (value) {
                
            case GuardType::BREAKPOINT:  return "Breakpoint";
            case GuardType::WATCHPOINT:  return "Watchpoint";
            case GuardType::CATCHPOINT:  return "Catchpoint";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    CPURevision revision;
    DasmRevision dasmRevision;
    DasmSyntax dasmSyntax;
    DasmNumbers dasmNumbers;
    isize overclocking;
    u32 regResetVal;
}
CPUConfig;

typedef struct
{
    Cycle clock;
    
    u32 pc0;
    u16 ird;
    u16 irc;
    u32 d[8];
    u32 a[8];
    u32 isp;
    u32 usp;
    u32 msp;
    u32 vbr;
    u16 sr;
    u8 sfc;
    u8 dfc;
    u8 cacr;
    u8 caar;
    u8 ipl;
    u8 fc;
    
    bool halt;
}
CPUInfo;

}
