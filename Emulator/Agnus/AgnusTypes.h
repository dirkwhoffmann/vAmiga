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
#include "BusTypes.h"
#include "SchedulerTypes.h"
#include "SequencerTypes.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(AGNUS_REVISION)
{
    AGNUS_OCS_DIP,          // Revision 8367 (A1000, A2000A)
    AGNUS_OCS_PLCC,         // Revision 8371 (A500, A2000B)
    AGNUS_ECS_1MB,          // Revision 8372 (A500, A2000B)
    AGNUS_ECS_2MB           // Revision 8375 (A500+, A600)
};
typedef AGNUS_REVISION AgnusRevision;

#ifdef __cplusplus
struct AgnusRevisionEnum : util::Reflection<AgnusRevisionEnum, AgnusRevision>
{
    static long minVal() { return 0; }
    static long maxVal() { return AGNUS_ECS_2MB; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "AGNUS"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {
                
            case AGNUS_OCS_DIP:  return "OCS_DIP";
            case AGNUS_OCS_PLCC: return "OCS_PLCC";
            case AGNUS_ECS_1MB:  return "ECS_1MB";
            case AGNUS_ECS_2MB:  return "ECS_2MB";
        }
        return "???";
    }
};
#endif

enum_long(SPR_DMA_STATE)
{
    SPR_DMA_IDLE,
    SPR_DMA_ACTIVE
};
typedef SPR_DMA_STATE SprDMAState;

#ifdef __cplusplus
struct SprDmaStateEnum : util::Reflection<SprDmaStateEnum, SprDMAState>
{
    static long minVal() { return 0; }
    static long maxVal() { return SPR_DMA_ACTIVE; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

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

// DEPRECATED
enum_long(DDF_STATE)
{
    DDF_OFF,
    DDF_READY,
    DDF_ON
};
typedef DDF_STATE DDFState;

#ifdef __cplusplus
struct DDFStateEnum : util::Reflection<DDFStateEnum, DDFState>
{
    static long minVal() { return 0; }
    static long maxVal() { return DDF_ON; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "DDF"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {
                
            case DDF_OFF:   return "OFF";
            case DDF_READY: return "READY";
            case DDF_ON:    return "ON";
        }
        return "???";
    }
};
#endif

/*
enum_long(DISPLAY_SIGNAL)
{
    // Upper four bits of BPLCON0
    SIG_CON_L0,
    SIG_CON_L1,
    SIG_CON_L2,
    SIG_CON_L3,
    SIG_CON_L4,
    SIG_CON_L5,
    SIG_CON_L6,
    SIG_CON_L7,
    SIG_CON_H0,
    SIG_CON_H1,
    SIG_CON_H2,
    SIG_CON_H3,
    SIG_CON_H4,
    SIG_CON_H5,
    SIG_CON_H6,
    SIG_CON_H7,

    // Bitplane enable bit
    SIG_BMAPEN_CLR,
    SIG_BMAPEN_SET,

    // Vertical position flipflop
    SIG_VFLOP_SET,
    SIG_VFLOP_CLR,
    
    // Vertical start and stop
    // SIG_BPVSTART0,
    // SIG_BPVSTART1,
    // SIG_BPVSTOP0,
    // SIG_BPVSTOP1,
    
    // Horizontal start and stop
    SIG_BPHSTART,
    SIG_BPHSTOP,
        
    // Left and right hardware stops
    SIG_SHW,
    SIG_RHW,
    
    // Dummy event
    SIG_NONE
};
typedef DDF_STATE DisplaySignal;

#ifdef __cplusplus
struct DisplaySignalEnum : util::Reflection<DisplaySignalEnum, DisplaySignal>
{
    static long minVal() { return 0; }
    static long maxVal() { return SIG_NONE; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "SIG"; }
    static const char *key(DisplaySignal value)
    {
        switch (value) {
                
            case SIG_CON_L0:        return "CON_L0";
            case SIG_CON_L1:        return "CON_L1";
            case SIG_CON_L2:        return "CON_L2";
            case SIG_CON_L3:        return "CON_L3";
            case SIG_CON_L4:        return "CON_L4";
            case SIG_CON_L5:        return "CON_L5";
            case SIG_CON_L6:        return "CON_L6";
            case SIG_CON_L7:        return "CON_L7";
            case SIG_CON_H0:        return "CON_H0";
            case SIG_CON_H1:        return "CON_H1";
            case SIG_CON_H2:        return "CON_H2";
            case SIG_CON_H3:        return "CON_H3";
            case SIG_CON_H4:        return "CON_H4";
            case SIG_CON_H5:        return "CON_H5";
            case SIG_CON_H6:        return "CON_H6";
            case SIG_CON_H7:        return "CON_H7";
            case SIG_BMAPEN_CLR:    return "BMAPEN_CLR";
            case SIG_BMAPEN_SET:    return "BMAPEN_SET";
            case SIG_VFLOP_SET:     return "VFLOP_SET";
            case SIG_VFLOP_CLR:     return "VFLOP_CLR";
            case SIG_BPHSTART:      return "BPHSTART";
            case SIG_BPHSTOP:       return "BPHSTOP";
            case SIG_SHW:           return "SHW";
            case SIG_RHW:           return "RHW";
            case SIG_NONE:          return "NONE";
        }
        return "???";
    }
};
#endif
*/

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;

//
// Structures
//

typedef struct
{
    AgnusRevision revision;
    bool slowRamMirror;
}
AgnusConfig;

typedef struct
{
    isize vpos;
    isize hpos;

    u16 dmacon;
    u16 bplcon0;
    u8  bpu;
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
    
    // u32 coppc;
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
    isize usage[BUS_COUNT];
    
    double copperActivity;
    double blitterActivity;
    double diskActivity;
    double audioActivity;
    double spriteActivity;
    double bitplaneActivity;
}
AgnusStats;

#ifdef __cplusplus
struct DDFFlipflops
{
    // Flipflops (derived from the OCS schematics)
    bool ff1 = false;
    bool ff2 = false;
    bool ff3 = false;
    bool ff4 = false;
    bool ff5 = false;
    
    bool operator!=(const DDFFlipflops &rhs) const
    {
        return
        this->ff1 != rhs.ff1 ||
        this->ff2 != rhs.ff2 ||
        this->ff3 != rhs.ff3 ||
        this->ff4 != rhs.ff4 ||
        this->ff5 != rhs.ff5;
    }
    
    template <class W>
    void operator<<(W& worker)
    {
        worker << ff1 << ff2 << ff3 << ff4 << ff5;
    }
};
#endif
