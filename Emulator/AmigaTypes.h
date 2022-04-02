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

//
// Enumerations
//

enum_long(CONFIG_SCHEME)
{
    CONFIG_A1000_OCS_1MB,
    CONFIG_A500_OCS_1MB,
    CONFIG_A500_ECS_1MB
};
typedef CONFIG_SCHEME ConfigScheme;

#ifdef __cplusplus
struct ConfigSchemeEnum : util::Reflection<ConfigSchemeEnum, ConfigScheme>
{
    static long minVal() { return 0; }
    static long maxVal() { return CONFIG_A500_ECS_1MB; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "CONFIG"; }
    static const char *key(ConfigScheme value)
    {
        switch (value) {
                
            case CONFIG_A1000_OCS_1MB:  return "A1000_OCS_1MB";
            case CONFIG_A500_OCS_1MB:   return "A500_OCS_1MB";
            case CONFIG_A500_ECS_1MB:   return "A500_ECS_1MB";
        }
        return "???";
    }
};
#endif

enum_long(INSPECTION_TARGET)
{
    INSPECTION_NONE,
    INSPECTION_AMIGA,
    INSPECTION_CPU,
    INSPECTION_CIA,
    INSPECTION_MEM,
    INSPECTION_AGNUS,
    INSPECTION_DENISE,
    INSPECTION_PAULA,
    INSPECTION_PORTS,
    INSPECTION_EVENTS,
};
typedef INSPECTION_TARGET InspectionTarget;

#ifdef __cplusplus
struct InspectionTargetEnum : util::Reflection<InspectionTargetEnum, InspectionTarget>
{
    static long minVal() { return 0; }
    static long maxVal() { return INSPECTION_EVENTS; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "INSPECTION"; }
    static const char *key(InspectionTarget value)
    {
        switch (value) {
                
            case INSPECTION_NONE:    return "NONE";
            case INSPECTION_AMIGA:   return "AMIGA";
            case INSPECTION_CPU:     return "CPU";
            case INSPECTION_CIA:     return "CIA";
            case INSPECTION_MEM:     return "MEM";
            case INSPECTION_AGNUS:   return "AGNUS";
            case INSPECTION_DENISE:  return "DENISE";
            case INSPECTION_PAULA:   return "PAULA";
            case INSPECTION_PORTS:   return "PORTS";
            case INSPECTION_EVENTS:  return "EVENTS";
        }
        return "???";
    }
};
#endif

enum_long(REG_CHIPSET)
{
    REG_BLTDDAT,  REG_DMACONR,  REG_VPOSR,    REG_VHPOSR,   REG_DSKDATR,
    REG_JOY0DAT,  REG_JOY1DAT,  REG_CLXDAT,   REG_ADKCONR,  REG_POT0DAT,
    REG_POT1DAT,  REG_POTGOR,   REG_SERDATR,  REG_DSKBYTR,  REG_INTENAR,
    REG_INTREQR,  REG_DSKPTH,   REG_DSKPTL,   REG_DSKLEN,   REG_DSKDAT,
    REG_REFPTR,   REG_VPOSW,    REG_VHPOSW,   REG_COPCON,   REG_SERDAT,
    REG_SERPER,   REG_POTGO,    REG_JOYTEST,  REG_STREQU,   REG_STRVBL,
    REG_STRHOR,   REG_STRLONG,  REG_BLTCON0,  REG_BLTCON1,  REG_BLTAFWM,
    REG_BLTALWM,  REG_BLTCPTH,  REG_BLTCPTL,  REG_BLTBPTH,  REG_BLTBPTL,
    REG_BLTAPTH,  REG_BLTAPTL,  REG_BLTDPTH,  REG_BLTDPTL,  REG_BLTSIZE,
    REG_BLTCON0L, REG_BLTSIZV,  REG_BLTSIZH,  REG_BLTCMOD,  REG_BLTBMOD,
    REG_BLTAMOD,  REG_BLTDMOD,  REG_0x68,     REG_0x6A,     REG_0x6C,
    REG_0x6E,     REG_BLTCDAT,  REG_BLTBDAT,  REG_BLTADAT,  REG_0x76,
    REG_SPRHDAT,  REG_BPLHDAT,  REG_DENISEID, REG_DSKSYNC,  REG_COP1LCH,
    REG_COP1LCL,  REG_COP2LCH,  REG_COP2LCL,  REG_COPJMP1,  REG_COPJMP2,
    REG_COPINS,   REG_DIWSTRT,  REG_DIWSTOP,  REG_DDFSTRT,  REG_DDFSTOP,
    REG_DMACON,   REG_CLXCON,   REG_INTENA,   REG_INTREQ,   REG_ADKCON,
    REG_AUD0LCH,  REG_AUD0LCL,  REG_AUD0LEN,  REG_AUD0PER,  REG_AUD0VOL,
    REG_AUD0DAT,  REG_0xAC,     REG_0xAE,     REG_AUD1LCH,  REG_AUD1LCL,
    REG_AUD1LEN,  REG_AUD1PER,  REG_AUD1VOL,  REG_AUD1DAT,  REG_0xBC,
    REG_0xBE,     REG_AUD2LCH,  REG_AUD2LCL,  REG_AUD2LEN,  REG_AUD2PER,
    REG_AUD2VOL,  REG_AUD2DAT,  REG_0xCC,     REG_0xCE,     REG_AUD3LCH,
    REG_AUD3LCL,  REG_AUD3LEN,  REG_AUD3PER,  REG_AUD3VOL,  REG_AUD3DAT,
    REG_0xDC,     REG_0xDE,     REG_BPL1PTH,  REG_BPL1PTL,  REG_BPL2PTH,
    REG_BPL2PTL,  REG_BPL3PTH,  REG_BPL3PTL,  REG_BPL4PTH,  REG_BPL4PTL,
    REG_BPL5PTH,  REG_BPL5PTL,  REG_BPL6PTH,  REG_BPL6PTL,  REG_BPL7PTH,
    REG_BPL7PTL,  REG_BPL8PTH,  REG_BPL8PTL,  REG_BPLCON0,  REG_BPLCON1,
    REG_BPLCON2,  REG_BPLCON3,  REG_BPL1MOD,  REG_BPL2MOD,  REG_BPLCON4,
    REG_CLXCON2,  REG_BPL1DAT,  REG_BPL2DAT,  REG_BPL3DAT,  REG_BPL4DAT,
    REG_BPL5DAT,  REG_BPL6DAT,  REG_BPL7DAT,  REG_BPL8DAT,  REG_SPR0PTH,
    REG_SPR0PTL,  REG_SPR1PTH,  REG_SPR1PTL,  REG_SPR2PTH,  REG_SPR2PTL,
    REG_SPR3PTH,  REG_SPR3PTL,  REG_SPR4PTH,  REG_SPR4PTL,  REG_SPR5PTH,
    REG_SPR5PTL,  REG_SPR6PTH,  REG_SPR6PTL,  REG_SPR7PTH,  REG_SPR7PTL,
    REG_SPR0POS,  REG_SPR0CTL,  REG_SPR0DATA, REG_SPR0DATB, REG_SPR1POS,
    REG_SPR1CTL,  REG_SPR1DATA, REG_SPR1DATB, REG_SPR2POS,  REG_SPR2CTL,
    REG_SPR2DATA, REG_SPR2DATB, REG_SPR3POS,  REG_SPR3CTL,  REG_SPR3DATA,
    REG_SPR3DATB, REG_SPR4POS,  REG_SPR4CTL,  REG_SPR4DATA, REG_SPR4DATB,
    REG_SPR5POS,  REG_PR5CTL,   REG_SPR5DATA, REG_SPR5DATB, REG_SPR6POS,
    REG_SPR6CTL,  REG_SPR6DATA, REG_SPR6DATB, REG_SPR7POS,  REG_SPR7CTL,
    REG_SPR7DATA, REG_SPR7DATB, REG_COLOR00,  REG_COLOR01,  REG_COLOR02,
    REG_COLOR03,  REG_COLOR04,  REG_COLOR05,  REG_COLOR06,  REG_COLOR07,
    REG_COLOR08,  REG_COLOR09,  REG_COLOR10,  REG_COLOR11,  REG_COLOR12,
    REG_COLOR13,  REG_COLOR14,  REG_COLOR15,  REG_COLOR16,  REG_COLOR17,
    REG_COLOR18,  REG_COLOR19,  REG_COLOR20,  REG_COLOR21,  REG_COLOR22,
    REG_COLOR23,  REG_COLOR24,  REG_COLOR25,  REG_COLOR26,  REG_COLOR27,
    REG_COLOR28,  REG_COLOR29,  REG_COLOR30,  REG_COLOR31,  REG_HTOTAL,
    REG_HSSTOP,   REG_HBSTRT,   REG_HBSTOP,   REG_VTOTAL,   REG_VSSTOP,
    REG_VBSTRT,   REG_VBSTOP,   REG_SPRHSTRT, REG_SPRHSTOP, REG_BPLHSTRT,
    REG_BPLHSTOP, REG_HHPOSW,   REG_HHPOSR,   REG_BEAMCON0, REG_HSSTRT,
    REG_VSSTRT,   REG_HCENTER,  REG_DIWHIGH,  REG_BPLHMOD,  REG_SPRHPTH,
    REG_SPRHPTL,  REG_BPLHPTH,  REG_BPLHPTL,  REG_0x1F0,    REG_0x1F2,
    REG_0x1F4,    REG_0x1F6,    REG_0x1F8,    REG_0x1FA,    REG_FMODE,
    REG_NO_OP
};
typedef REG_CHIPSET ChipsetReg;

#ifdef __cplusplus
static_assert(REG_NO_OP == (0x1FE >> 1));
struct ChipsetRegEnum : util::Reflection<ChipsetRegEnum, ChipsetReg>
{
    static long minVal() { return 0; }
    static long maxVal() { return REG_NO_OP; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "REG"; }
    static const char *key(ConfigScheme value)
    {
        static const char *name[] = {

            "BLTDDAT",        "DMACONR",        "VPOSR",
            "VHPOSR",         "DSKDATR",        "JOY0DAT",
            "JOY1DAT",        "CLXDAT",         "ADKCONR",
            "POT0DAT",        "POT1DAT",        "POTGOR",
            "SERDATR",        "DSKBYTR",        "INTENAR",
            "INTREQR",        "DSKPTH",         "DSKPTL",
            "DSKLEN",         "DSKDAT",         "REFPTR",
            "VPOSW",          "VHPOSW",         "COPCON",
            "SERDAT",         "SERPER",         "POTGO",
            "JOYTEST",        "STREQU",         "STRVBL",
            "STRHOR",         "STRLONG",        "BLTCON0",
            "BLTCON1",        "BLTAFWM",        "BLTALWM",
            "BLTCPTH",        "BLTCPTL",        "BLTBPTH",
            "BLTBPTL",        "BLTAPTH",        "BLTAPTL",
            "BLTDPTH",        "BLTDPTL",        "BLTSIZE",
            "BLTCON0L (ECS)", "BLTSIZV (ECS)",  "BLTSIZH (ECS)",
            "BLTCMOD",        "BLTBMOD",        "BLTAMOD",
            "BLTDMOD",        "unused",         "unused",
            "unused",         "unused",         "BLTCDAT",
            "BLTBDAT",        "BLTADAT",        "unused",
            "SPRHDAT (ECS)",  "BPLHDAT (AGA)",  "DENISEID (ECS)",
            "DSKSYNC",        "COP1LCH",        "COP1LCL",
            "COP2LCH",        "COP2LCL",        "COPJMP1",
            "COPJMP2",        "COPINS",         "DIWSTRT",
            "DIWSTOP",        "DDFSTRT",        "DDFSTOP",
            "DMACON",         "CLXCON",         "INTENA",
            "INTREQ",         "ADKCON",         "AUD0PTH",
            "AUD0PTL",        "AUD0LEN",        "AUD0PER",
            "AUD0VOL",        "AUD0DAT",        "unused",
            "UNUSED",         "AUD1PTH",        "AUD1PTL",
            "AUD1LEN",        "AUD1PER",        "AUD1VOL",
            "AUD1DAT",        "unused",         "unused",
            "AUD2PTH",        "AUD2PTL",        "AUD2LEN",
            "AUD2PER",        "AUD2VOL",        "AUD2DAT",
            "unused",         "unused",         "AUD3PTH",
            "AUD3PTL",        "AUD3LEN",        "AUD3PER",
            "AUD3VOL",        "AUD3DAT",        "unused",
            "UNUSED",         "BPL1PTH",        "BPL1PTL",
            "BPL2PTH",        "BPL2PTL",        "BPL3PTH",
            "BPL3PTL",        "BPL4PTH",        "BPL4PTL",
            "BPL5PTH",        "BPL5PTL",        "BPL6PTH",
            "BPL6PTL",        "BPL7PTH (AGA)",  "BPL7PTL (AGA)",
            "BPL8PTH (AGA)",  "BPL8PTL (AGA)",  "BPLCON0",
            "BPLCON1",        "BPLCON2",        "BPLCON3 (ECS)",
            "BPL1MOD",        "BPL2MOD",        "BPLCON4 (AGA)",
            "BPLCON4 (AGA)",  "BPL1DAT",        "BPL2DAT",
            "BPL3DAT",        "BPL4DAT",        "BPL5DAT",
            "BPL6DAT",        "BPL7DAT (AGA)",  "BPL8DAT (AGA)",
            "SPR0PTH",        "SPR0PTL",        "SPR1PTH",
            "SPR1PTL",        "SPR2PTH",        "SPR2PTL",
            "SPR3PTH",        "SPR3PTL",        "SPR4PTH",
            "SPR4PTL",        "SPR5PTH",        "SPR5PTL",
            "SPR6PTH",        "SPR6PTL",        "SPR7PTH",
            "SPR7PTL",        "SPR0POS",        "SPR0CTL",
            "SPR0DATA",       "SPR0DATB",       "SPR1POS",
            "SPR1CTL",        "SPR1DATA",       "SPR1DATB",
            "SPR2POS",        "SPR2CTL",        "SPR2DATA",
            "SPR2DATB",       "SPR3POS",        "SPR3CTL",
            "SPR3DATA",       "SPR3DATB",       "SPR4POS",
            "SPR4CTL",        "SPR4DATA",       "SPR4DATB",
            "SPR5POS",        "SPR5CTL",        "SPR5DATA",
            "SPR5DATB",       "SPR6POS",        "SPR6CTL",
            "SPR6DATA",       "SPR6DATB",       "SPR7POS",
            "SPR7CTL",        "SPR7DATA",       "SPR7DATB",
            "COLOR00",        "COLOR01",        "COLOR02",
            "COLOR03",        "COLOR04",        "COLOR05",
            "COLOR06",        "COLOR07",        "COLOR08",
            "COLOR09",        "COLOR10",        "COLOR11",
            "COLOR12",        "COLOR13",        "COLOR14",
            "COLOR15",        "COLOR16",        "COLOR17",
            "COLOR18",        "COLOR19",        "COLOR20",
            "COLOR21",        "COLOR22",        "COLOR23",
            "COLOR24",        "COLOR25",        "COLOR26",
            "COLOR27",        "COLOR28",        "COLOR29",
            "COLOR30",        "COLOR31",        "HTOTAL (ECS)",
            "HSSTOP (ECS)",   "HBSTRT (ECS)",   "HBSTOP (ECS)",
            "VTOTAL (ECS)",   "VSSTOP (ECS)",   "VBSTRT (ECS)",
            "VBSTOP (ECS)",   "SPRHSTRT (AGA)", "SPRHSTOP (AGA)",
            "BPLHSTRT (AGA)", "BPLHSTOP (AGA)", "HHPOSW (AGA)",
            "HHPOSR (AGA)",   "BEAMCON0 (ECS)", "HSSTRT (ECS)",
            "VSSTRT (ECS)",   "HCENTER (ECS)",  "DIWHIGH (ECS)",
            "BPLHMOD (AGA)",  "SPRHPTH (AGA)",  "SPRHPTL (AGA)",
            "BPLHPTH (AGA)",  "BPLHPTL (AGA)",  "unused",
            "unused",         "unused",         "unused",
            "unused",         "unused",         "FMODE (AGA)",
            "NO-OP"
        };

        return isValid(value) ? name[value] : "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    Cycle cpuClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    i64 frame;
    long vpos;
    long hpos;
}
AmigaInfo;


//
// Private data types
//

#ifdef __cplusplus

typedef u32 RunLoopFlags;

namespace RL
{
constexpr u32 STOP               = (1 << 0);
constexpr u32 WARP_ON            = (1 << 1);
constexpr u32 WARP_OFF           = (1 << 2);
constexpr u32 SOFTSTOP_REACHED   = (1 << 3);
constexpr u32 BREAKPOINT_REACHED = (1 << 4);
constexpr u32 WATCHPOINT_REACHED = (1 << 5);
constexpr u32 CATCHPOINT_REACHED = (1 << 6);
constexpr u32 SWTRAP_REACHED     = (1 << 7);
constexpr u32 COPPERBP_REACHED   = (1 << 8);
constexpr u32 COPPERWP_REACHED   = (1 << 9);
constexpr u32 AUTO_SNAPSHOT      = (1 << 10);
constexpr u32 USER_SNAPSHOT      = (1 << 11);
constexpr u32 SYNC_THREAD        = (1 << 12);
};

#endif
