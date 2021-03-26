// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Event.h"

//
// Enumerations
//

enum_long(SLOT)
{
    // Primary slots
    SLOT_REG,                       // Register changes
    SLOT_RAS,                       // Rasterline
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
    
    SLOT_COUNT
};
typedef SLOT EventSlot;

/*
static inline bool isEventSlot(long s) { return s < SLOT_COUNT; }
static inline bool isPrimarySlot(long s) { return s <= SLOT_SEC; }
static inline bool isSecondarySlot(long s) { return s > SLOT_SEC && s < SLOT_COUNT; }

inline const char *slotName(EventSlot nr)
{
    switch (nr) {
        case SLOT_REG:  return "Registers";
        case SLOT_RAS:  return "Rasterline";
        case SLOT_CIAA: return "CIA A";
        case SLOT_CIAB: return "CIA B";
        case SLOT_BPL:  return "Bitplane DMA";
        case SLOT_DAS:  return "Other DMA";
        case SLOT_COP:  return "Copper";
        case SLOT_BLT:  return "Blitter";
        case SLOT_SEC:  return "Secondary";

        case SLOT_CH0:  return "Audio channel 0";
        case SLOT_CH1:  return "Audio channel 1";
        case SLOT_CH2:  return "Audio channel 2";
        case SLOT_CH3:  return "Audio channel 3";
        case SLOT_DSK:  return "Disk Controller";
        case SLOT_DCH:  return "Disk Change";
        case SLOT_VBL:  return "Vertical blank";
        case SLOT_IRQ:  return "Interrupts";
        case SLOT_IPL:  return "IPL";
        case SLOT_KBD:  return "Keyboard";
        case SLOT_TXD:  return "UART out";
        case SLOT_RXD:  return "UART in";
        case SLOT_POT:  return "Potentiometer";
        case SLOT_INS:  return "Inspector";

        default:
            // assert(false);
            return "*** INVALID ***";
    }
}
*/

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;


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

    EventSlotInfo slotInfo[SLOT_COUNT];
}
EventInfo;
