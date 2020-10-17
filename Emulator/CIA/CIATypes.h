// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _CIA_TYPES_H
#define _CIA_TYPES_H

#include "Aliases.h"

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
typedef VA_ENUM(long, CIAType)
{
    CIA_8520_DIP,
    CIA_8520_PLCC
};

inline bool
isCIAType(long value)
{
    return value >= CIA_8520_DIP && value <= CIA_8520_PLCC;
}

inline const char *
ciaTypeName(CIAType type)
{
    assert(isCIAType(type));

    switch (type) {
        case CIA_8520_DIP:   return "CIA_8520_DIP";
        case CIA_8520_PLCC:  return "CIA_8520_PLCC";
        default:             return "???";
    }
}

typedef struct
{
    CIAType type;
    bool    todBug;
    bool    eClockSyncing;
}
CIAConfig;

typedef struct
{
    long value;
    long latch;
    long alarm;
}
CounterInfo;

typedef struct
{
    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portA;

    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portB;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerA;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerB;

    u8 sdr;
    u8 ssr;
    u8 icr;
    u8 imr;
    bool intLine;
    
    CounterInfo cnt;
    bool cntIntEnable;
    
    Cycle idleSince;
    Cycle idleTotal;
    double idlePercentage;
}
CIAInfo;

#endif
