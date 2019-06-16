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
    DMA_SLOT,         // Disk, Audio, Sprite, and Bitplane DMA
    COP_SLOT,         // Copper DMA
    BLT_SLOT,         // Blitter DMA
    RAS_SLOT,         // Raster line events
    SEC_SLOT,         // Secondary events
    PRIM_SLOT_COUNT,
    
    //
    // Secondary slot table
    //
    
    DSK_SLOT = 0,     // Disk controller
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
    TXD_SLOT,         // Serial data out (UART)
    RXD_SLOT,         // Serial data in (UART)
    INSPECTOR_SLOT,   // Handles periodic calls to inspect()
    SEC_SLOT_COUNT,
} EventSlot;

static inline bool isPrimarySlot(int32_t s) { return s < PRIM_SLOT_COUNT; }
static inline bool isSecondarySlot(int32_t s) { return s < SEC_SLOT_COUNT; }

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
    
    // DMA slot
    DMA_DISK = 1,
    DMA_A0,
    DMA_A1,
    DMA_A2,
    DMA_A3,
    DMA_S0_1,
    DMA_S1_1,
    DMA_S2_1,
    DMA_S3_1,
    DMA_S4_1,
    DMA_S5_1,
    DMA_S6_1,
    DMA_S7_1,
    DMA_S0_2,
    DMA_S1_2,
    DMA_S2_2,
    DMA_S3_2,
    DMA_S4_2,
    DMA_S5_2,
    DMA_S6_2,
    DMA_S7_2,
    DMA_L1,
    DMA_L1_FIRST,
    DMA_L1_LAST,
    DMA_L2,
    DMA_L3,
    DMA_L4,
    DMA_L5,
    DMA_L6,
    DMA_H1,
    DMA_H1_FIRST,
    DMA_H1_LAST,
    DMA_H2,
    DMA_H3,
    DMA_H4,
    DMA_EVENT_COUNT,
    
    // Copper slot
    COP_REQUEST_DMA = 1,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT,
    COP_SKIP,
    COP_JMP1,
    COP_JMP2,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_INIT = 1,
    BLT_EXECUTE,
    BLT_FAST_BLIT,
    BLT_EVENT_COUNT,
    
    // Raster slot
    RAS_HSYNC = 1,
    RAS_EVENT_COUNT,
    
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
    IRQ_SET = 1,
    IRQ_CLEAR,
    IRQ_EVENT_COUNT,

    // Serial data out (UART)
    TXD_BIT = 1,
    TXD_EVENT_COUNT,

    // Serial data out (UART)
    RXD_BIT = 1,
    RXD_EVENT_COUT,

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
static inline bool isDmaEvent(EventID id) { return id < DMA_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id < COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id < BLT_EVENT_COUNT; }
static inline bool isRasEvent(EventID id) { return id < RAS_EVENT_COUNT; }

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

    EventSlotInfo primary[PRIM_SLOT_COUNT];
    EventSlotInfo secondary[SEC_SLOT_COUNT];
}
EventHandlerInfo;

#endif
