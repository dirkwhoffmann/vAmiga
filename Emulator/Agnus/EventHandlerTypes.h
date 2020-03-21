// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _EVENT_T_INC
#define _EVENT_T_INC

// Time stamp used for messages that never trigger
#define NEVER INT64_MAX


//
// Enumerations
//

typedef enum : long
{
    // Primary slots
    REG_SLOT = 0,                   // Register changes
    RAS_SLOT,                       // Rasterline
    CIAA_SLOT,                      // CIA A execution
    CIAB_SLOT,                      // CIA B execution
    BPL_SLOT,                       // Bitplane DMA
    DAS_SLOT,                       // Disk, Audio, and Sprite DMA
    COP_SLOT,                       // Copper
    BLT_SLOT,                       // Blitter
    SEC_SLOT,                       // Enables secondary slots

    // Secondary slots
    CH0_SLOT,                       // Audio channel 0
    CH1_SLOT,                       // Audio channel 1
    CH2_SLOT,                       // Audio channel 2
    CH3_SLOT,                       // Audio channel 3
    DSK_SLOT,                       // Disk controller
    DCH_SLOT,                       // Disk changes (insert, eject)
    VBL_SLOT,                       // Vertical blank
    IRQ_SLOT,                       // Interrupts
    IPL_SLOT,                       // CPU Interrupt Priority Lines
    KBD_SLOT,                       // Keyboard
    TXD_SLOT,                       // Serial data out (UART)
    RXD_SLOT,                       // Serial data in (UART)
    POT_SLOT,                       // Potentiometer
    INS_SLOT,                       // Handles periodic calls to inspect()
    SLOT_COUNT

} EventSlot;

static inline bool isEventSlot(long s) { return s < SLOT_COUNT; }
static inline bool isPrimarySlot(long s) { return s <= SEC_SLOT; }
static inline bool isSecondarySlot(long s) { return s > SEC_SLOT && s < SLOT_COUNT; }

inline const char *slotName(EventSlot nr)
{
    switch (nr) {
        case REG_SLOT:  return "Registers";
        case RAS_SLOT:  return "Rasterline";
        case CIAA_SLOT: return "CIA A";
        case CIAB_SLOT: return "CIA B";
        case BPL_SLOT:  return "Bitplane DMA";
        case DAS_SLOT:  return "Other DMA";
        case COP_SLOT:  return "Copper";
        case BLT_SLOT:  return "Blitter";
        case SEC_SLOT:  return "Secondary";

        case CH0_SLOT:  return "Audio channel 0";
        case CH1_SLOT:  return "Audio channel 1";
        case CH2_SLOT:  return "Audio channel 2";
        case CH3_SLOT:  return "Audio channel 3";
        case DSK_SLOT:  return "Disk Controller";
        case DCH_SLOT:  return "Disk Change";
        case VBL_SLOT:  return "Vertical blank";
        case IRQ_SLOT:  return "Interrupts";
        case IPL_SLOT:  return "IPL";
        case KBD_SLOT:  return "Keyboard";
        case TXD_SLOT:  return "UART out";
        case RXD_SLOT:  return "UART in";
        case POT_SLOT:  return "Potentiometer";
        case INS_SLOT:  return "Inspector";

        default:
            assert(false);
            return "*** INVALID ***";
    }
}

typedef enum : long
{
    EVENT_NONE = 0,
    
    //
    // Events in the primary event table
    //

    // REG slot
    REG_CHANGE = 1,
    // REG_HSYNC,
    REG_EVENT_COUNT,

    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // BPL slot
    BPL_DRAW_ODD = 1,
    BPL_DRAW_EVEN,
    BPL_DRAW_BOTH,
    BPL_L1,
    BPL_L1_DRAW_ODD,
    BPL_L1_DRAW_EVEN,
    BPL_L1_DRAW_BOTH,
    BPL_L2,
    BPL_L2_DRAW_ODD,
    BPL_L2_DRAW_EVEN,
    BPL_L2_DRAW_BOTH,
    BPL_L3,
    BPL_L3_DRAW_ODD,
    BPL_L3_DRAW_EVEN,
    BPL_L3_DRAW_BOTH,
    BPL_L4,
    BPL_L4_DRAW_ODD,
    BPL_L4_DRAW_EVEN,
    BPL_L4_DRAW_BOTH,
    BPL_L5,
    BPL_L5_DRAW_ODD,
    BPL_L5_DRAW_EVEN,
    BPL_L5_DRAW_BOTH,
    BPL_L6,
    BPL_L6_DRAW_ODD,
    BPL_L6_DRAW_EVEN,
    BPL_L6_DRAW_BOTH,
    BPL_H1,
    BPL_H1_DRAW_ODD,
    BPL_H1_DRAW_EVEN,
    BPL_H1_DRAW_BOTH,
    BPL_H2,
    BPL_H2_DRAW_ODD,
    BPL_H2_DRAW_EVEN,
    BPL_H2_DRAW_BOTH,
    BPL_H3,
    BPL_H3_DRAW_ODD,
    BPL_H3_DRAW_EVEN,
    BPL_H3_DRAW_BOTH,
    BPL_H4,
    BPL_H4_DRAW_ODD,
    BPL_H4_DRAW_EVEN,
    BPL_H4_DRAW_BOTH,
    BPL_EOL,
    BPL_EOL_DRAW_ODD,
    BPL_EOL_DRAW_EVEN,
    BPL_EOL_DRAW_BOTH,
    BPL_EVENT_COUNT,

    // DAS slot
    DAS_REFRESH = 1,
    DAS_D0,
    DAS_D1,
    DAS_D2,
    DAS_A0,
    DAS_A1,
    DAS_A2,
    DAS_A3,
    DAS_S0_1,
    DAS_S0_2,
    DAS_S1_1,
    DAS_S1_2,
    DAS_S2_1,
    DAS_S2_2,
    DAS_S3_1,
    DAS_S3_2,
    DAS_S4_1,
    DAS_S4_2,
    DAS_S5_1,
    DAS_S5_2,
    DAS_S6_1,
    DAS_S6_2,
    DAS_S7_1,
    DAS_S7_2,
    DAS_SDMA,
    DAS_EVENT_COUNT,

    // Copper slot
    COP_REQ_DMA = 1,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT1,
    COP_WAIT2,
    COP_WAIT_BLIT,
    COP_SKIP1,
    COP_SKIP2,
    COP_JMP1,
    COP_JMP2,
    COP_VBLANK,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_STRT1 = 1,
    BLT_STRT2,
    BLT_COPY_SLOW,
    BLT_COPY_FAKE,
    BLT_LINE_FAKE,
    BLT_EVENT_COUNT,
        
    // SEC slot
    SEC_TRIGGER = 1,
    SEC_EVENT_COUNT,
    
    //
    // Events in secondary event table
    //

    // Audio channels
    CHX_PERFIN = 1,
    CHX_EVENT_COUNT,

    // Disk controller slot
    DSK_ROTATE = 1,
    DSK_EVENT_COUNT,

    // Disk change slot
    DCH_INSERT = 1,
    DCH_EJECT,
    DCH_EVENT_COUNT,

    // Strobe slot
    VBL_STROBE = 1,
    VBL_EVENT_COUNT,
    
    // IRQ slot
    IRQ_CHECK = 1,
    IRQ_EVENT_COUNT,

    // IPL slot
    IPL_CHANGE = 1,
    IPL_EVENT_COUNT,

    // Keyboard
    KBD_SELFTEST = 1,
    KBD_SYNC,
    KBD_STRM_ON,
    KBD_STRM_OFF,
    KBD_TIMEOUT,
    KBD_SEND,
    KBD_EVENT_COUNT,

    // Serial data out (UART)
    TXD_BIT = 1,
    TXD_EVENT_COUNT,

    // Serial data out (UART)
    RXD_BIT = 1,
    RXD_EVENT_COUT,

    // Potentiometer
    POT_DISCHARGE = 1,
    POT_CHARGE,
    POT_EVENT_COUNT,
    
    // Inspector slot
    INS_NONE = 1,
    INS_AMIGA,
    INS_CPU,
    INS_MEM,
    INS_CIA,
    INS_AGNUS,
    INS_PAULA,
    INS_DENISE,
    INS_PORTS,
    INS_EVENTS,
    INS_EVENT_COUNT,

    // Rasterline slot
    RAS_HSYNC = 1,
    RAS_EVENT_COUNT

} EventID;

static inline bool isRegEvent(EventID id) { return id < REG_EVENT_COUNT; }
static inline bool isCiaEvent(EventID id) { return id < CIA_EVENT_COUNT; }
static inline bool isBplEvent(EventID id) { return id < BPL_EVENT_COUNT; }
static inline bool isDasEvent(EventID id) { return id < DAS_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id < COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id < BLT_EVENT_COUNT; }

static inline bool isBplxEvent(EventID id, int x)
{
    assert(1 <= x && x <= 6);

    switch(id & ~0b11) {

        case BPL_L1: case BPL_H1: return x == 1;
        case BPL_L2: case BPL_H2: return x == 2;
        case BPL_L3: case BPL_H3: return x == 3;
        case BPL_L4: case BPL_H4: return x == 4;
        case BPL_L5:              return x == 5;
        case BPL_L6:              return x == 6;

        default:
            return false;
    }
}

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;


//
// Structures
//

typedef struct
{
    const char *slotName;
    const char *eventName;
    long eventId;

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

#endif
