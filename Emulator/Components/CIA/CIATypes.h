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
#include "TODTypes.h"

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
enum_long(CIA_REVISION)
{
    CIA_MOS_8520_DIP,
    CIA_MOS_8520_PLCC
};
typedef CIA_REVISION CIARevision;

#ifdef __cplusplus
struct CIARevisionEnum : vamiga::util::Reflection<CIARevisionEnum, CIARevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CIA_MOS_8520_PLCC;

    static const char *prefix() { return "CIA"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case CIA_MOS_8520_DIP:   return "MOS_8520_DIP";
            case CIA_MOS_8520_PLCC:  return "MOS_8520_PLCC";
        }
        return "???";
    }
};
#endif

enum_long(CIAREG)
{
    CIAREG_PRA,
    CIAREG_PRB,
    CIAREG_DDRA,
    CIAREG_DDRB,
    CIAREG_TALO,
    CIAREG_TAHI,
    CIAREG_TBLO,
    CIAREG_TBHI,
    CIAREG_TODTHS,
    CIAREG_TODSEC,
    CIAREG_TODMIN,
    CIAREG_TODHR,
    CIAREG_SDR,
    CIAREG_ICR,
    CIAREG_CRA,
    CIAREG_CRB
};
typedef CIAREG CIAReg;

#ifdef __cplusplus
struct CIARegEnum : vamiga::util::Reflection<CIARegEnum, CIAReg>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = CIAREG_CRB;
    
    static const char *prefix() { return "CIAREG"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case CIAREG_PRA:     return "PRA";
            case CIAREG_PRB:     return "PRB";
            case CIAREG_DDRA:    return "DDRA";
            case CIAREG_DDRB:    return "DDRB";
            case CIAREG_TALO:    return "TALO";
            case CIAREG_TAHI:    return "TAHI";
            case CIAREG_TBLO:    return "TBLO";
            case CIAREG_TBHI:    return "TBHI";
            case CIAREG_TODTHS:  return "TODTHS";
            case CIAREG_TODSEC:  return "TODSEC";
            case CIAREG_TODMIN:  return "TODMIN";
            case CIAREG_TODHR:   return "TODHR";
            case CIAREG_SDR:     return "SDR";
            case CIAREG_ICR:     return "ICR";
            case CIAREG_CRA:     return "CRA";
            case CIAREG_CRB:     return "CRB";
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
    CIARevision revision;
    bool todBug;
    bool eClockSyncing;
    bool idleSleep;
}
CIAConfig;

typedef struct
{
    u8 port;
    u8 reg;
    u8 dir;
}
CIAPortInfo;

typedef struct
{
    u16 count;
    u16 latch;
    bool running;
    bool toggle;
    bool pbout;
    bool oneShot;
}
CIATimerInfo;

typedef struct
{
    CIAPortInfo portA;
    CIAPortInfo portB;

    CIATimerInfo timerA;
    CIATimerInfo timerB;

    u8 sdr;
    u8 ssr;
    u8 icr;
    u8 imr;
    bool irq;
    
    TODInfo tod;
    bool todIrqEnable;    
}
CIAInfo;

typedef struct
{
    Cycle idleSince;
    Cycle idleTotal;
    double idlePercentage;
}
CIAStats;

