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

#include "TODTypes.h"
#include "Aliases.h"

namespace vamiga {

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
/// CIA chip revision.
enum class CIARev : long
{
    MOS_8520_DIP,
    MOS_8520_PLCC
};

struct CIARevEnum : utl::Reflectable<CIARevEnum, CIARev>
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

/// Registers of a CIA chip.
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

struct CIARegEnum : utl::Reflectable<CIARegEnum, CIAReg>
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

/// Configuration options of a CIA chip.
typedef struct
{
    CIARev revision;                        ///< Chip revision
    bool todBug;                            ///< Emulate the TOD bug
    bool eClockSyncing;                     ///< Sync accesses with the E clock
    bool idleSleep;                         ///< Enter the idle state while the chip is not in use
}
CIAConfig;

/// State of a CIA port.
typedef struct
{
    u8 port;                                ///< State of the port pins
    u8 reg;                                 ///< Port register (PRA or PRB)
    u8 dir;                                 ///< Data direction register (DDRA or DDRB)
}
CIAPortInfo;

/// State of a CIA timer.
typedef struct
{
    u16 count;                              ///< Current counter value
    u16 latch;                              ///< Latch value the counter is reloaded from
    bool running;                           ///< The timer is running
    bool toggle;                            ///< Toggle mode is on
    bool pbout;                             ///< The timer output appears on port B
    bool oneShot;                           ///< One-shot mode is on
}
CIATimerInfo;

/// State of a CIA chip.
typedef struct
{
    CIAPortInfo portA;                      ///< Port A
    CIAPortInfo portB;                      ///< Port B
    
    CIATimerInfo timerA;                    ///< Timer A
    CIATimerInfo timerB;                    ///< Timer B
    
    u8 sdr;                                 ///< Serial data register (SDR)
    u8 ssr;                                 ///< Serial shift register
    u8 icr;                                 ///< Interrupt control register (pending interrupts)
    u8 imr;                                 ///< Interrupt mask register
    bool irq;                               ///< The interrupt line is active
    
    TODInfo tod;                            ///< Time-of-day clock
    bool todIrqEnable;                      ///< The TOD alarm interrupt is enabled
}
CIAInfo;

/// Statistics collected by a CIA chip.
typedef struct
{
    Cycle totalCycles;                      ///< Total number of executed cycles
    Cycle idleCycles;                       ///< Number of cycles spent in the idle state
    
    double idlePercentage;                  ///< Share of idle cycles
}
CIAMetrics;

}
