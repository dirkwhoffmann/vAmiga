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
#include "BeamTypes.h"
#include "BusTypes.h"
#include "AgnusTypes.h"
#include "SequencerTypes.h"
#include "Reflection.h"


//
// Macros and constants
//

// Checks the category of an event slot
#define isPrimarySlot(s) ((s) <= SLOT_SEC)
#define isSecondarySlot(s) ((s) > SLOT_SEC && (s) <= SLOT_TER)
#define isTertiarySlot(s) ((s) > SLOT_TER)

// Time stamp used for messages that never trigger
#define NEVER INT64_MAX

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;


//
// Enumerations
//

enum_long(AGNUS_REVISION)
{
    AGNUS_OCS_OLD,          // Revision 8367 (A1000, A2000A)
    AGNUS_OCS,              // Revision 8371 (A500, A2000B)
    AGNUS_ECS_1MB,          // Revision 8372 (A500, A2000B)
    AGNUS_ECS_2MB           // Revision 8375 (A500+, A600)
};
typedef AGNUS_REVISION AgnusRevision;

#ifdef __cplusplus
struct AgnusRevisionEnum : util::Reflection<AgnusRevisionEnum, AgnusRevision>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = AGNUS_ECS_2MB;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "AGNUS"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {
                
            case AGNUS_OCS_OLD:  return "OCS_OLD";
            case AGNUS_OCS:      return "OCS";
            case AGNUS_ECS_1MB:  return "ECS_1MB";
            case AGNUS_ECS_2MB:  return "ECS_2MB";
        }
        return "???";
    }
};
#endif

enum_long(SLOT)
{
    // Primary slots
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
    SLOT_VBL,                       // Vertical blank
    SLOT_IRQ,                       // Interrupts
    SLOT_IPL,                       // CPU Interrupt Priority Lines
    SLOT_KBD,                       // Keyboard
    SLOT_TXD,                       // Serial data out (UART)
    SLOT_RXD,                       // Serial data in (UART)
    SLOT_POT,                       // Potentiometer
    SLOT_TER,                       // Enables tertiary slots
    
    // Tertiary slots
    SLOT_DC0,                       // Disk change (Df0)
    SLOT_DC1,                       // Disk change (Df1)
    SLOT_DC2,                       // Disk change (Df2)
    SLOT_DC3,                       // Disk change (Df3)
    SLOT_HD0,                       // Hard drive Hd0
    SLOT_HD1,                       // Hard drive Hd1
    SLOT_HD2,                       // Hard drive Hd2
    SLOT_HD3,                       // Hard drive Hd3
    SLOT_MSE1,                      // Port 1 mouse
    SLOT_MSE2,                      // Port 2 mouse
    SLOT_KEY,                       // Auto-typing
    SLOT_SRV,                       // Remote server manager
    SLOT_SER,                       // Serial remote server
    SLOT_INS,                       // Handles periodic calls to inspect()

    SLOT_COUNT
};
typedef SLOT EventSlot;

#ifdef __cplusplus
struct EventSlotEnum : util::Reflection<EventSlotEnum, EventSlot>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SLOT_COUNT - 1;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "SLOT"; }
    static const char *key(EventSlot value)
    {
        switch (value) {
                
            case SLOT_REG:   return "REG";
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
            case SLOT_VBL:   return "VBL";
            case SLOT_IRQ:   return "IRQ";
            case SLOT_IPL:   return "IPL";
            case SLOT_KBD:   return "KBD";
            case SLOT_TXD:   return "TXD";
            case SLOT_RXD:   return "RXD";
            case SLOT_POT:   return "POT";
            case SLOT_TER:   return "TER";
                
            case SLOT_DC0:   return "DC0";
            case SLOT_DC1:   return "DC1";
            case SLOT_DC2:   return "DC2";
            case SLOT_DC3:   return "DC3";
            case SLOT_HD0:   return "HD0";
            case SLOT_HD1:   return "HD1";
            case SLOT_HD2:   return "HD2";
            case SLOT_HD3:   return "HD3";
            case SLOT_MSE1:  return "MSE1";
            case SLOT_MSE2:  return "MSE2";
            case SLOT_KEY:   return "KEY";
            case SLOT_SRV:   return "SRV";
            case SLOT_SER:   return "SER";
            case SLOT_INS:   return "INS";

            case SLOT_COUNT: return "???";
        }
        return "???";
    }
};
#endif

enum_i8(EventID)
{
    EVENT_NONE          = 0,
    
    //
    // Events in the primary event table
    //

    // REG slot
    REG_CHANGE          = 1,
    REG_EVENT_COUNT,

    // CIA slots
    CIA_EXECUTE         = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // BPL slot
    BPL_L1              = 0x04,
    BPL_L1_MOD          = 0x08,
    BPL_L2              = 0x0C,
    BPL_L2_MOD          = 0x10,
    BPL_L3              = 0x14,
    BPL_L3_MOD          = 0x18,
    BPL_L4              = 0x1C,
    BPL_L4_MOD          = 0x20,
    BPL_L5              = 0x24,
    BPL_L5_MOD          = 0x28,
    BPL_L6              = 0x2C,
    BPL_L6_MOD          = 0x30,
    BPL_H1              = 0x34,
    BPL_H1_MOD          = 0x38,
    BPL_H2              = 0x3C,
    BPL_H2_MOD          = 0x40,
    BPL_H3              = 0x44,
    BPL_H3_MOD          = 0x48,
    BPL_H4              = 0x4C,
    BPL_H4_MOD          = 0x50,
    BPL_EVENT_COUNT     = 0x54,

    // DAS slot
    DAS_REFRESH         = 1,
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
    DAS_TICK,
    DAS_HSYNC,
    DAS_EOL,
    DAS_EVENT_COUNT,

    // Copper slot
    COP_REQ_DMA         = 1,
    COP_WAKEUP,
    COP_WAKEUP_BLIT,
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
    BLT_STRT1           = 1,
    BLT_STRT2,
    BLT_COPY_SLOW,
    BLT_COPY_FAKE,
    BLT_LINE_SLOW,
    BLT_LINE_FAKE,
    BLT_EVENT_COUNT,
        
    // SEC slot
    SEC_TRIGGER         = 1,
    SEC_EVENT_COUNT,
    
    //
    // Events in secondary event table
    //

    // Audio channels
    CHX_PERFIN          = 1,
    CHX_EVENT_COUNT,

    // Disk controller slot
    DSK_ROTATE          = 1,
    DSK_EVENT_COUNT,

    // Strobe slot
    VBL_STROBE0         = 1,
    VBL_STROBE1,
    VBL_STROBE2,
    VBL_EVENT_COUNT,
    
    // IRQ slot
    IRQ_CHECK           = 1,
    IRQ_EVENT_COUNT,

    // IPL slot
    IPL_CHANGE          = 1,
    IPL_EVENT_COUNT,

    // Keyboard
    KBD_TIMEOUT         = 1,
    KBD_DAT,
    KBD_CLK0,
    KBD_CLK1,
    KBD_SYNC_DAT0,
    KBD_SYNC_CLK0,
    KBD_SYNC_DAT1,
    KBD_SYNC_CLK1,
    KBD_EVENT_COUNT,

    // Serial data out (UART)
    TXD_BIT             = 1,
    TXD_EVENT_COUNT,

    // Serial data out (UART)
    RXD_BIT             = 1,
    RXD_EVENT_COUT,

    // Potentiometer
    POT_DISCHARGE       = 1,
    POT_CHARGE,
    POT_EVENT_COUNT,
    
    // Screenshots
    SCR_TAKE            = 1,
    SCR_EVENT_COUNT,
        
    // SEC slot
    TER_TRIGGER         = 1,
    TER_EVENT_COUNT,
    
    //
    // Events in tertiary event table
    //
    
    // Disk change slot
    DCH_INSERT          = 1,
    DCH_EJECT,
    DCH_EVENT_COUNT,

    // Hard drive slot
    HDR_IDLE            = 1,
    HDR_EVENT_COUNT,
    
    // Mouse
    MSE_PUSH_LEFT       = 1,
    MSE_RELEASE_LEFT,
    MSE_PUSH_RIGHT,
    MSE_RELEASE_RIGHT,
    MSE_EVENT_COUNT,

    // Auto typing
    KEY_PRESS           = 1,
    KEY_RELEASE,
    KEY_EVENT_COUNT,
    
    // Remote server manager
    SRV_LAUNCH_DAEMON   = 1,
    SRV_EVENT_COUNT,
    
    // Serial remote server
    SER_RECEIVE         = 1,
    SER_EVENT_COUNT,
    
    // Inspector slot
    INS_AMIGA           = 1,
    INS_CPU,
    INS_MEM,
    INS_CIA,
    INS_AGNUS,
    INS_PAULA,
    INS_DENISE,
    INS_PORTS,
    INS_EVENTS,
    INS_EVENT_COUNT
};

static inline bool isRegEvent(EventID id) { return id < REG_EVENT_COUNT; }
static inline bool isCiaEvent(EventID id) { return id < CIA_EVENT_COUNT; }
static inline bool isBplEvent(EventID id) { return id < BPL_EVENT_COUNT; }
static inline bool isDasEvent(EventID id) { return id < DAS_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id < COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id < BLT_EVENT_COUNT; }

static inline bool isBplxEvent(EventID id, int x)
{
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

enum_long(SPR_DMA_STATE)
{
    SPR_DMA_IDLE,
    SPR_DMA_ACTIVE
};
typedef SPR_DMA_STATE SprDMAState;

#ifdef __cplusplus
struct SprDmaStateEnum : util::Reflection<SprDmaStateEnum, SprDMAState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = SPR_DMA_ACTIVE;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "SPR_DMA"; }
    static const char *key(SprDMAState value)
    {
        switch (value) {
                
            case SPR_DMA_IDLE:   return "IDLE";
            case SPR_DMA_ACTIVE: return "ACTIVE";
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
    AgnusRevision revision;
    bool slowRamMirror;
    bool ptrDrops;
}
AgnusConfig;

typedef struct
{
    isize vpos;
    isize hpos;

    u16 dmacon;
    u16 bplcon0;
    u16 ddfstrt;
    u16 ddfstop;
    u16 diwstrt;
    u16 diwstop;

    u16 bpl1mod;
    u16 bpl2mod;
    u16 bltamod;
    u16 bltbmod;
    u16 bltcmod;
    u16 bltdmod;
    u16 bltcon0;
    
    u32 coppc0;
    u32 dskpt;
    u32 bplpt[6];
    u32 audpt[4];
    u32 audlc[4];
    u32 bltpt[4];
    u32 sprpt[8];

    bool bls;
}
AgnusInfo;

typedef struct
{
    EventSlot slot;
    EventID eventId;
    const char *eventName;

    // Trigger cycle of the event
    Cycle trigger;
    Cycle triggerRel;

    // Trigger frame relative to the current frame
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
    i64 frame;
    long vpos;
    long hpos;
}
EventInfo;

typedef struct
{
    isize usage[BUS_COUNT];
    
    double copperActivity;
    double blitterActivity;
    double diskActivity;
    double audioActivity;
    double spriteActivity;
    double bitplaneActivity;
}
AgnusStats;
