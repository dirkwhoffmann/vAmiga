// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

#ifdef __cplusplus
#include "RingBuffer.h"
#endif

#define CPUINFO_INSTR_COUNT 256


//
// Enumerations
//

enum_long(CPU_REVISION)
{
    CPU_68000,
    CPU_68010,
    CPU_68EC020
};
typedef CPU_REVISION CPURevision;

#ifdef __cplusplus
struct CPURevisionEnum : util::Reflection<CPURevisionEnum, CPURevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CPU_68EC020;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "CPU"; }
    static const char *key(CPURevision value)
    {
        switch (value) {

            case CPU_68000:     return "68000";
            case CPU_68010:     return "68010";
            case CPU_68EC020:   return "68EC020";
        }
        return "???";
    }
};
#endif

enum_long(DASM_REVISION)
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
typedef DASM_REVISION DasmRevision;

#ifdef __cplusplus
struct DasmRevisionEnum : util::Reflection<DasmRevisionEnum, DasmRevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DASM_68040;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "DASM"; }
    static const char *key(DasmRevision value)
    {
        switch (value) {

            case DASM_68000:    return "68000";
            case DASM_68010:    return "68010";
            case DASM_68EC020:  return "68EC020";
            case DASM_68020:    return "68020";
            case DASM_68EC030:  return "68EC030";
            case DASM_68030:    return "68030";
            case DASM_68EC040:  return "68EC040";
            case DASM_68LC040:  return "68LC040";
            case DASM_68040:    return "68040";
        }
        return "???";
    }
};
#endif

enum_long(DASM_SYNTAX)
{
    DASM_SYNTAX_MOIRA,
    DASM_SYNTAX_MOIRA_MIT,
    DASM_SYNTAX_GNU,
    DASM_SYNTAX_GNU_MIT,
    DASM_SYNTAX_MUSASHI
};
typedef DASM_SYNTAX DasmSyntax;

#ifdef __cplusplus
struct DasmSyntaxEnum : util::Reflection<DasmSyntaxEnum, DasmSyntax>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = DASM_SYNTAX_MUSASHI;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "DASM_SYNTAX"; }
    static const char *key(CPURevision value)
    {
        switch (value) {

            case DASM_SYNTAX_MOIRA:      return "MOIRA";
            case DASM_SYNTAX_MOIRA_MIT:  return "MOIRA_MIT";
            case DASM_SYNTAX_GNU:        return "GNU";
            case DASM_SYNTAX_GNU_MIT:    return "GNU_MIT";
            case DASM_SYNTAX_MUSASHI:    return "MUSASHI";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    CPURevision revision;
    DasmRevision dasmRevision;
    DasmSyntax dasmSyntax;
    isize overclocking;
    u32 regResetVal;
}
CPUConfig;

typedef struct
{
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

#ifdef __cplusplus
struct CallStackEntry
{
    // Opcode of the branch instruction
    u16 opcode;
    
    // Program counter and subroutine address
    u32 oldPC;
    u32 newPC;
    
    // Register contents
    u32 d[8];
    u32 a[8];

    template <class W>
    void operator<<(W& worker)
    {
        worker << opcode << oldPC << newPC << d << a;
    }
};

struct CallstackRecorder : public util::SortedRingBuffer<CallStackEntry, 64>
{
    template <class W>
    void operator<<(W& worker)
    {
        worker >> this->elements << this->r << this->w << this->keys;
    }
};
#endif
