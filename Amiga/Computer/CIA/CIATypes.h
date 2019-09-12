// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _CIA_T_INC
#define _CIA_T_INC

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600 behaviour)
 */
typedef enum : long
{
    CIA_8520_DIP,
    CIA_8520_PLCC
}
CIAType;

inline bool isCIAType(long value)
{
    return value >= CIA_8520_DIP && value <= CIA_8520_PLCC;
}

inline const char *ciaTypeName(CIAType type)
{
    assert(isCIAType(type));

    switch (type) {
        case CIA_8520_DIP:   return "CIA_8520_DIP";
        case CIA_8520_PLCC:  return "CIA_8520_PLCC";
        default:             return "???";
    }
}

typedef union
{
    struct
    {
        uint8_t hi;
        uint8_t mid;
        uint8_t lo;
    };
    uint32_t value;
}
Counter24;

typedef struct
{
    Counter24 value;
    Counter24 latch;
    Counter24 alarm;
}
CounterInfo;

typedef struct
{
    CIAType type;
}
CIAConfig;

typedef struct
{
    struct {
        uint8_t port;
        uint8_t reg;
        uint8_t dir;
    } portA;

    struct {
        uint8_t port;
        uint8_t reg;
        uint8_t dir;
    } portB;

    struct {
        uint16_t count;
        uint16_t latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerA;

    struct {
        uint16_t count;
        uint16_t latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerB;

    uint8_t sdr;
    uint8_t icr;
    uint8_t imr;
    bool intLine;
    CounterInfo cnt;
    bool cntIntEnable;
    Cycle idleCycles;
    double idlePercentage;
}
CIAInfo;

#endif
