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
    //
    // Primary slot table
    //
    
    CIAA_SLOT = 0,    // CIA A execution
    CIAB_SLOT,        // CIA B execution
    BPL_SLOT,         // Bitplane DMA
    DAS_SLOT,         // Disk, Audio, and Sprite DMA
    COP_SLOT,         // Copper DMA
    BLT_SLOT,         // Blitter DMA
    SEC_SLOT,         // Secondary events

    LAST_PRIM_SLOT = SEC_SLOT,
    FIRST_SEC_SLOT,

    //
    // Secondary slot table
    //

    REG_COP_SLOT = FIRST_SEC_SLOT, // Delayed Copper writes to custom registers
    REG_CPU_SLOT1,                 // Delayed CPU writes to custom registers
    REG_CPU_SLOT2,                 // Second slot for delayed CPU writes

    DSK_SLOT,         // Disk controller
    IRQ_SLOT,         // Interrupts
    IRQ_TBE_SLOT,     // Source 0 IRQ (Serial port transmit buffer empty)
    IRQ_DSKBLK_SLOT,  // Source 1 IRQ (Disk block finished)
    IRQ_SOFT_SLOT,    // Source 2 IRQ (Software-initiated)
    IRQ_PORTS_SLOT,   // Source 3 IRQ (I/O ports and CIA A)
    IRQ_COPR_SLOT,    // Source 4 IRQ (Copper)
    IRQ_VERTB_SLOT,   // Source 5 IRQ (Start of vertical blank)
    IRQ_BLIT_SLOT,    // Source 6 IRQ (Blitter finished)
    IRQ_AUD0_SLOT,    // Source 7 IRQ (Audio channel 0 block finished)
    IRQ_AUD1_SLOT,    // Source 8 IRQ (Audio channel 1 block finished)
    IRQ_AUD2_SLOT,    // Source 9 IRQ (Audio channel 2 block finished)
    IRQ_AUD3_SLOT,    // Source 10 IRQ (Audio channel 3 block finished)
    IRQ_RBF_SLOT,     // Source 11 IRQ (Serial port receive buffer full)
    IRQ_DSKSYN_SLOT,  // Source 12 IRQ (Disk sync register matches disk data)
    IRQ_EXTER_SLOT,   // Source 13 IRQ (I/O ports and CIA B)
    KBD_SLOT,         // Keyboard
    TXD_SLOT,         // Serial data out (UART)
    RXD_SLOT,         // Serial data in (UART)
    POT_SLOT,         // Potentiometer
    SYNC_SLOT,        // Synchronization (HSYNC) 
    INSPECTOR_SLOT,   // Handles periodic calls to inspect()

    LAST_SEC_SLOT = INSPECTOR_SLOT,
    SLOT_COUNT

} EventSlot;

static inline bool isEventSlot(int32_t s) { return s < SLOT_COUNT; }
static inline bool isPrimarySlot(int32_t s) { return s <= LAST_PRIM_SLOT; }
static inline bool isSecondarySlot(int32_t s) { return s >= FIRST_SEC_SLOT && s <= LAST_SEC_SLOT; }

inline const char *slotName(EventSlot nr)
{
    switch (nr) {
        case CIAA_SLOT:          return "CIA A"; 
        case CIAB_SLOT:          return "CIA B";
        case BPL_SLOT:           return "Bitplane DMA";
        case DAS_SLOT:           return "Disk, Audio, Sprite DMA";
        case COP_SLOT:           return "Copper";
        case BLT_SLOT:           return "Blitter";
        case SEC_SLOT:           return "Secondary";

        case DSK_SLOT:           return "Disk Controller";
        case IRQ_SLOT:           return "Interrupts";
        case IRQ_TBE_SLOT:       return "Serial Out IRQ";
        case IRQ_DSKBLK_SLOT:    return "Disk DMA IRQ";
        case IRQ_SOFT_SLOT:      return "Software IRQ";
        case IRQ_PORTS_SLOT:     return "CIA A IRQ";
        case IRQ_COPR_SLOT:      return "Copper IRQ";
        case IRQ_VERTB_SLOT:     return "VBlank IRQ";
        case IRQ_BLIT_SLOT:      return "Blitter IRQ";
        case IRQ_AUD0_SLOT:      return "Audio 0 IRQ";
        case IRQ_AUD1_SLOT:      return "Audio 1 IRQ";
        case IRQ_AUD2_SLOT:      return "Audio 2 IRQ";
        case IRQ_AUD3_SLOT:      return "Audio 3 IRQ";
        case IRQ_RBF_SLOT:       return "Serial In IRQ";
        case IRQ_DSKSYN_SLOT:    return "Disk Sync IRQ";
        case IRQ_EXTER_SLOT:     return "CIA B IRQ";
        case REG_COP_SLOT:       return "Copper Write";
        case REG_CPU_SLOT1:      return "CPU Write (1)";
        case REG_CPU_SLOT2:      return "CPU Write (2)";
        case KBD_SLOT:           return "Keyboard";
        case TXD_SLOT:           return "UART out";
        case RXD_SLOT:           return "UART in";
        case POT_SLOT:           return "Potentiometer";
        case SYNC_SLOT:          return "Sync";
        case INSPECTOR_SLOT:     return "Debugger";

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
    
    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // BPL slot
    BPL_L1 = 1,
    BPL_L2,
    BPL_L3,
    BPL_L4,
    BPL_L5,
    BPL_L6,
    BPL_H1,
    BPL_H2,
    BPL_H3,
    BPL_H4,
    BPL_HSYNC,
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
    DAS_EVENT_COUNT,

    // Copper slot
    COP_REQ_DMA = 1,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT1,
    COP_WAIT2,
    COP_SKIP1,
    COP_SKIP2,
    COP_JMP1,
    COP_JMP2,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_START = 1,
    BLT_EXEC_SLOW,
    BLT_EXEC_FAST,
    BLT_EVENT_COUNT,
        
    // SEC slot
    SEC_TRIGGER = 1,
    SEC_EVENT_COUNT,
    
    //
    // Events in secondary event table
    //
    
    // Disk controller slot
    DSK_ROTATE = 1,
    DSK_EVENT_COUNT,
    
    // IRQ slots
    IRQ_CHECK = 1,
    IRQ_SET, // DEPRECATED
    IRQ_CLEAR, // DEPRECATED
    IRQ_EVENT_COUNT,

    // REG slots
    REG_DMACON = 1,
    REG_DIWSTRT,
    REG_DIWSTOP,
    REG_BPLCON0,
    REG_BPLCON1,
    REG_BPLCON2,
    REG_BPL1MOD,
    REG_BPL2MOD,
    REG_BPLxPTH,
    REG_BPLxPTL,
    REG_EVENT_COUNT,

    // Keyboard
    KBD_QUERY = 1,
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

    // SYNC slot (DEPRECATED)
    SYNC_EOL = 1,
    SYNC_EVENT_COUNT,
    
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
    INS_EVENT_COUNT

} EventID;

static inline bool isCiaEvent(EventID id) { return id < CIA_EVENT_COUNT; }
static inline bool isBplEvent(EventID id) { return id < BPL_EVENT_COUNT; }
static inline bool isDasEvent(EventID id) { return id < DAS_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id < COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id < BLT_EVENT_COUNT; }

static inline bool isBplxEvent(EventID id, int x)
{
    assert(1 <= x && x <= 6);

    switch(id) {

        case BPL_L1:
        case BPL_H1: return x == 1;
        case BPL_L2:
        case BPL_H2: return x == 2;
        case BPL_L3:
        case BPL_H3: return x == 3;
        case BPL_L4:
        case BPL_H4: return x == 4;
        case BPL_L5: return x == 5;
        case BPL_L6: return x == 6;

        default:
            return false;
    }
}

static inline int DASEventCycle(EventID id)
{
    switch(id) {

        case DAS_REFRESH: return 0x01;
        case DAS_D0:      return 0x07;
        case DAS_D1:      return 0x09;
        case DAS_D2:      return 0x0B;
        case DAS_A0:      return 0x0D;
        case DAS_A1:      return 0x0F;
        case DAS_A2:      return 0x11;
        case DAS_A3:      return 0x13;
        case DAS_S0_1:    return 0x15;
        case DAS_S0_2:    return 0x17;
        case DAS_S1_1:    return 0x19;
        case DAS_S1_2:    return 0x1B;
        case DAS_S2_1:    return 0x1D;
        case DAS_S2_2:    return 0x1F;
        case DAS_S3_1:    return 0x21;
        case DAS_S3_2:    return 0x23;
        case DAS_S4_1:    return 0x25;
        case DAS_S4_2:    return 0x27;
        case DAS_S5_1:    return 0x29;
        case DAS_S5_2:    return 0x2B;
        case DAS_S6_1:    return 0x2D;
        case DAS_S6_2:    return 0x2F;
        case DAS_S7_1:    return 0x31;
        case DAS_S7_2:    return 0x33;

        default:
            assert(false);
            return 0;
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
    Cycle trigger;
    Cycle triggerRel;

    // The trigger cycle translated to a beam position.
    long vpos;
    long hpos;

    // Indicates if (vpos, hpos) is a position inside the current frame.
    bool currentFrame;
}
EventSlotInfo;

typedef struct
{
    Cycle masterClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;

    EventSlotInfo slotInfo[SLOT_COUNT];
}
EventInfo;

struct Event
{
    // Indicates when the event is due.
    Cycle triggerCycle;

    // Identifier of the scheduled event.
    EventID id;

    // Optional data value
    // Can be used to pass additional information to the event handler.
    int64_t data;
};

#endif
