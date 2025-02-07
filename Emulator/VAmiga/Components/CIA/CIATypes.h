// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "TODTypes.h"

namespace vamiga {

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
enum class CIARev : long
{
    MOS_8520_DIP,
    MOS_8520_PLCC
};

struct CIARevEnum : Reflection<CIARevEnum, CIARev>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(CIARev::MOS_8520_PLCC);
    
    static const char *_key(CIARev value)
    {
        switch (value) {
                
            case CIARev::MOS_8520_DIP:   return "MOS_8520_DIP";
            case CIARev::MOS_8520_PLCC:  return "MOS_8520_PLCC";
        }
        return "???";
    }
    static const char *help(CIARev value)
    {
        switch (value) {
                
            case CIARev::MOS_8520_DIP:   return "MOS 8520 DIP package";
            case CIARev::MOS_8520_PLCC:  return "MOS_8520 PLCC package";
        }
        return "???";
    }
};

enum class CIAReg : long
{
    PRA,
    PRB,
    DDRA,
    DDRB,
    TALO,
    TAHI,
    TBLO,
    TBHI,
    TODTHS,
    TODSEC,
    TODMIN,
    TODHR,
    SDR,
    ICR,
    CRA,
    CRB
};

struct CIARegEnum : Reflection<CIARegEnum, CIAReg>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(CIAReg::CRB);
    
    static const char *_key(CIAReg value)
    {
        switch (value) {
                
            case CIAReg::PRA:     return "PRA";
            case CIAReg::PRB:     return "PRB";
            case CIAReg::DDRA:    return "DDRA";
            case CIAReg::DDRB:    return "DDRB";
            case CIAReg::TALO:    return "TALO";
            case CIAReg::TAHI:    return "TAHI";
            case CIAReg::TBLO:    return "TBLO";
            case CIAReg::TBHI:    return "TBHI";
            case CIAReg::TODTHS:  return "TODTHS";
            case CIAReg::TODSEC:  return "TODSEC";
            case CIAReg::TODMIN:  return "TODMIN";
            case CIAReg::TODHR:   return "TODHR";
            case CIAReg::SDR:     return "SDR";
            case CIAReg::ICR:     return "ICR";
            case CIAReg::CRA:     return "CRA";
            case CIAReg::CRB:     return "CRB";
        }
        return "???";
    }
    static const char *help(CIAReg value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    CIARev revision;
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

}
