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
    CPU_68EC020,
    CPU_68020,
    CPU_68EC030,
    CPU_68030,
    CPU_68EC040,
    CPU_68LC040,
    CPU_68040
};
typedef CPU_REVISION CPURevision;

#ifdef __cplusplus
struct CPURevisionEnum : util::Reflection<CPURevisionEnum, CPURevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CPU_68040;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "CPU"; }
    static const char *key(CPURevision value)
    {
        switch (value) {

            case CPU_68000:     return "68000";
            case CPU_68010:     return "68010";
            case CPU_68EC020:   return "68EC020";
            case CPU_68020:     return "68020";
            case CPU_68EC030:   return "68EC030";
            case CPU_68030:     return "68030";
            case CPU_68EC040:   return "68EC040";
            case CPU_68LC040:   return "68LC040";
            case CPU_68040:     return "68040";
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
    CPURevision dasmRevision;
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
