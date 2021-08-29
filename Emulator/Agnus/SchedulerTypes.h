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
#include "EventTypes.h"

#define isPrimarySlot(s) ((s) <= SLOT_SEC)
// #define isSecondarySlot(s) ((s) > SLOT_SEC && (s) < SLOT_COUNT)
#define isSecondarySlot(s) ((s) > SLOT_SEC)

//
// Enumerations
//

enum_long(SLOT)
{
    // Primary slots
    SLOT_RAS,                       // Rasterline
    SLOT_REG,                       // Register changes
    SLOT_CIAA,                      // CIA A execution
    SLOT_CIAB,                      // CIA B execution
    SLOT_BPL,                       // Bitplane DMA
    SLOT_DAS,                       // Disk, Audio, and Sprite DMA
    SLOT_COP,                       // Copper
    SLOT_BLT,                       // Blitter
    SLOT_SEC,                       // Enables secondary slots

    // Secondary slots
    SLOT_CH0,                       // Audio channel 0
    SLOT_CH1,                       // Audio channel 1
    SLOT_CH2,                       // Audio channel 2
    SLOT_CH3,                       // Audio channel 3
    SLOT_DSK,                       // Disk controller
    SLOT_DCH,                       // Disk changes (insert, eject)
    SLOT_VBL,                       // Vertical blank
    SLOT_IRQ,                       // Interrupts
    SLOT_IPL,                       // CPU Interrupt Priority Lines
    SLOT_KBD,                       // Keyboard
    SLOT_TXD,                       // Serial data out (UART)
    SLOT_RXD,                       // Serial data in (UART)
    SLOT_POT,                       // Potentiometer
    SLOT_INS,                       // Handles periodic calls to inspect()
    SLOT_EOL,                       // HSYNC handler (End of Line)
    SLOT_COUNT
};
typedef SLOT EventSlot;

#ifdef __cplusplus
struct EventSlotEnum : util::Reflection<EventSlotEnum, EventSlot>
{
    static long min() { return 0; }
    static long max() { return SLOT_COUNT - 1; }
    static bool isValid(long value) { return value >= min() && value <= max(); }
    
    static const char *prefix() { return "SLOT"; }
    static const char *key(EventSlot value)
    {
        switch (value) {
                
            case SLOT_REG:   return "REG";
            case SLOT_RAS:   return "RAS";
            case SLOT_CIAA:  return "CIAA";
            case SLOT_CIAB:  return "CIAB";
            case SLOT_BPL:   return "BPL";
            case SLOT_DAS:   return "DAS";
            case SLOT_COP:   return "COP";
            case SLOT_BLT:   return "BLT";
            case SLOT_SEC:   return "SEC";

            case SLOT_CH0:   return "CH0";
            case SLOT_CH1:   return "CH1";
            case SLOT_CH2:   return "CH2";
            case SLOT_CH3:   return "CH3";
            case SLOT_DSK:   return "DSK";
            case SLOT_DCH:   return "DCH";
            case SLOT_VBL:   return "VBL";
            case SLOT_IRQ:   return "IRQ";
            case SLOT_IPL:   return "IPL";
            case SLOT_KBD:   return "KBD";
            case SLOT_TXD:   return "TXD";
            case SLOT_RXD:   return "RXD";
            case SLOT_POT:   return "POT";
            case SLOT_INS:   return "INS";
            case SLOT_EOL:   return "EOL";
            case SLOT_COUNT: return "???";
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
    EventSlot slot;
    EventID eventId;
    const char *eventName;

    // Trigger cycle of the event
    Cycle trigger;
    Cycle triggerRel;

    // Trigger relative to the current frame
    // -1 = earlier frame, 0 = current frame, 1 = later frame
    long frameRel;

    // The trigger cycle translated to a beam position.
    long vpos;
    long hpos;
}
EventSlotInfo;

typedef struct
{
    Cycle cpuClock;
    Cycle cpuCycles;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
EventInfo;
