// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ThreadTypes.h"
#include "Aliases.h"

namespace vamiga {

//
// Enumerations
//

enum class Compressor : long
{
    NONE,
    GZIP,
    LZ4,
    RLE2,
    RLE3
};

struct CompressorEnum : Reflection<CompressorEnum, Compressor>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Compressor::RLE3);

    static const char *_key(Compressor value)
    {
        switch (value) {

            case Compressor::NONE:  return "NONE";
            case Compressor::GZIP:  return "GZIP";
            case Compressor::RLE2:  return "RLE2";
            case Compressor::RLE3:  return "RLE3";
            case Compressor::LZ4:   return "LZ4";
        }
        return "???";
    }
    static const char *help(Compressor value)
    {
        switch (value) {

            case Compressor::NONE:  return "No compression";
            case Compressor::GZIP:  return "Gzip compression";
            case Compressor::RLE2:  return "Run-length encoding (2)";
            case Compressor::RLE3:  return "Run-length encoding (3)";
            case Compressor::LZ4:   return "LZ4 compression";
        }
        return "???";
    }
};

enum class TV : long
{
    PAL,
    NTSC
};

struct TVEnum : Reflection<TVEnum, TV>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(TV::NTSC);

    static const char *_key(TV value)
    {
        switch (value) {

            case TV::PAL:   return "PAL";
            case TV::NTSC:  return "NTSC";
        }
        return "???";
    }
    static const char *help(TV value)
    {
        switch (value) {

            case TV::PAL:   return "PAL Video Format";
            case TV::NTSC:  return "NTSC Video Format";
        }
        return "???";
    }
};

enum class Resolution : long
{
    LORES,      // Lores mode
    HIRES,      // Hires mode
    SHRES       // SuperHires mode (ECS only)
};

struct ResolutionEnum : Reflection<ResolutionEnum, Resolution>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Resolution::SHRES);

    static const char *_key(Resolution value)
    {
        switch (value) {

            case Resolution::LORES:          return "LORES";
            case Resolution::HIRES:          return "HIRES";
            case Resolution::SHRES:          return "SHRES";
        }
        return "???";
    }
    static const char *help(Resolution value)
    {
        switch (value) {

            case Resolution::LORES:          return "Lores Graphics";
            case Resolution::HIRES:          return "Hires Graphics";
            case Resolution::SHRES:          return "Super-Hires Graphics";
        }
        return "???";
    }
};

enum class Warp : long
{
    AUTO,
    NEVER,
    ALWAYS
};

struct WarpEnum : Reflection<WarpEnum, Warp>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Warp::ALWAYS);

    static const char *_key(Warp value)
    {
        switch (value) {

            case Warp::AUTO:     return "AUTO";
            case Warp::NEVER:    return "NEVER";
            case Warp::ALWAYS:   return "ALWAYS";
        }
        return "???";
    }
    static const char *help(Warp value)
    {
        return "";
    }
};

enum class ConfigScheme : long
{
    A1000_OCS_1MB,
    A500_OCS_1MB,
    A500_ECS_1MB,
    A500_PLUS_1MB
};

struct ConfigSchemeEnum : Reflection<ConfigSchemeEnum, ConfigScheme>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(ConfigScheme::A500_PLUS_1MB);

    static const char *_key(ConfigScheme value)
    {
        switch (value) {

            case ConfigScheme::A1000_OCS_1MB:  return "A1000_OCS_1MB";
            case ConfigScheme::A500_OCS_1MB:   return "A500_OCS_1MB";
            case ConfigScheme::A500_ECS_1MB:   return "A500_ECS_1MB";
            case ConfigScheme::A500_PLUS_1MB:  return "A500_PLUS_1MB";
        }
        return "???";
    }
    static const char *help(ConfigScheme value)
    {
        switch (value) {

            case ConfigScheme::A1000_OCS_1MB:  return "Amiga 1000, OCS Chipset, 1MB RAM";
            case ConfigScheme::A500_OCS_1MB:   return "Amiga 500, OCS Chipset, 1MB RAM";
            case ConfigScheme::A500_ECS_1MB:   return "Amiga 500, ECS Chipset, 1MB RAM";
            case ConfigScheme::A500_PLUS_1MB:  return "Amiga 500+, ECS Chipset, 1MB RAM";
        }
        return "???";
    }
};

enum class Reg : long
{
    BLTDDAT,    DMACONR,    VPOSR,      VHPOSR,     DSKDATR,
    JOY0DAT,    JOY1DAT,    CLXDAT,     ADKCONR,    POT0DAT,
    POT1DAT,    POTGOR,     SERDATR,    DSKBYTR,    INTENAR,
    INTREQR,    DSKPTH,     DSKPTL,     DSKLEN,     DSKDAT,
    REFPTR,     VPOSW,      VHPOSW,     COPCON,     SERDAT,
    SERPER,     POTGO,      JOYTEST,    STREQU,     STRVBL,
    STRHOR,     STRLONG,    BLTCON0,    BLTCON1,    BLTAFWM,
    BLTALWM,    BLTCPTH,    BLTCPTL,    BLTBPTH,    BLTBPTL,
    BLTAPTH,    BLTAPTL,    BLTDPTH,    BLTDPTL,    BLTSIZE,
    BLTCON0L,   BLTSIZV,    BLTSIZH,    BLTCMOD,    BLTBMOD,
    BLTAMOD,    BLTDMOD,    REG_0x68,   REG_0x6A,   REG_0x6C,
    REG_0x6E,   BLTCDAT,    BLTBDAT,    BLTADAT,    REG_0x76,
    SPRHDAT,    BPLHDAT,    DENISEID,   DSKSYNC,    COP1LCH,
    COP1LCL,    COP2LCH,    COP2LCL,    COPJMP1,    COPJMP2,
    COPINS,     DIWSTRT,    DIWSTOP,    DDFSTRT,    DDFSTOP,
    DMACON,     CLXCON,     INTENA,     INTREQ,     ADKCON,
    AUD0LCH,    AUD0LCL,    AUD0LEN,    AUD0PER,    AUD0VOL,
    AUD0DAT,    REG_0xAC,   REG_0xAE,   AUD1LCH,    AUD1LCL,
    AUD1LEN,    AUD1PER,    AUD1VOL,    AUD1DAT,    REG_0xBC,
    REG_0xBE,   AUD2LCH,    AUD2LCL,    AUD2LEN,    AUD2PER,
    AUD2VOL,    AUD2DAT,    REG_0xCC,   REG_0xCE,   AUD3LCH,
    AUD3LCL,    AUD3LEN,    AUD3PER,    AUD3VOL,    AUD3DAT,
    REG_0xDC,   REG_0xDE,   BPL1PTH,    BPL1PTL,    BPL2PTH,
    BPL2PTL,    BPL3PTH,    BPL3PTL,    BPL4PTH,    BPL4PTL,
    BPL5PTH,    BPL5PTL,    BPL6PTH,    BPL6PTL,    BPL7PTH,
    BPL7PTL,    BPL8PTH,    BPL8PTL,    BPLCON0,    BPLCON1,
    BPLCON2,    BPLCON3,    BPL1MOD,    BPL2MOD,    BPLCON4,
    CLXCON2,    BPL1DAT,    BPL2DAT,    BPL3DAT,    BPL4DAT,
    BPL5DAT,    BPL6DAT,    BPL7DAT,    BPL8DAT,    SPR0PTH,
    SPR0PTL,    SPR1PTH,    SPR1PTL,    SPR2PTH,    SPR2PTL,
    SPR3PTH,    SPR3PTL,    SPR4PTH,    SPR4PTL,    SPR5PTH,
    SPR5PTL,    SPR6PTH,    SPR6PTL,    SPR7PTH,    SPR7PTL,
    SPR0POS,    SPR0CTL,    SPR0DATA,   SPR0DATB,   SPR1POS,
    SPR1CTL,    SPR1DATA,   SPR1DATB,   SPR2POS,    SPR2CTL,
    SPR2DATA,   SPR2DATB,   SPR3POS,    SPR3CTL,    SPR3DATA,
    SPR3DATB,   SPR4POS,    SPR4CTL,    SPR4DATA,   SPR4DATB,
    SPR5POS,    SPR5CTL,    SPR5DATA,   SPR5DATB,   SPR6POS,
    SPR6CTL,    SPR6DATA,   SPR6DATB,   SPR7POS,    SPR7CTL,
    SPR7DATA,   SPR7DATB,   COLOR00,    COLOR01,    COLOR02,
    COLOR03,    COLOR04,    COLOR05,    COLOR06,    COLOR07,
    COLOR08,    COLOR09,    COLOR10,    COLOR11,    COLOR12,
    COLOR13,    COLOR14,    COLOR15,    COLOR16,    COLOR17,
    COLOR18,    COLOR19,    COLOR20,    COLOR21,    COLOR22,
    COLOR23,    COLOR24,    COLOR25,    COLOR26,    COLOR27,
    COLOR28,    COLOR29,    COLOR30,    COLOR31,    HTOTAL,
    HSSTOP,     HBSTRT,     HBSTOP,     VTOTAL,     VSSTOP,
    VBSTRT,     VBSTOP,     SPRHSTRT,   SPRHSTOP,   BPLHSTRT,
    BPLHSTOP,   HHPOSW,     HHPOSR,     BEAMCON0,   HSSTRT,
    VSSTRT,     HCENTER,    DIWHIGH,    BPLHMOD,    SPRHPTH,
    SPRHPTL,    BPLHPTH,    BPLHPTL,    REG_0x1F0,  REG_0x1F2,
    REG_0x1F4,  REG_0x1F6,  REG_0x1F8,  REG_0x1FA,  FMODE,
    NO_OP
};

static_assert(Reg::NO_OP == Reg(0x1FE >> 1));
struct RegEnum : Reflection<RegEnum, Reg>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Reg::NO_OP);

    static const char *_key(Reg value)
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

        return isValid(value) ? name[isize(value)] : "???";
    }
    static const char *help(Reg value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    //! Machine type (PAL or NTSC)
    TV type;

    //! After a reset, the emulator runs in warp mode for this amout of seconds
    isize warpBoot;

    //! Warp mode
    Warp warpMode;

    //! Emulator speed in percent (100 is native speed)
    isize speedBoost;

    //! Vertical Synchronization
    bool vsync;

    //! Number of run-ahead frames (0 = run-ahead is disabled)
    isize runAhead;

    //! Enable auto-snapshots
    bool autoSnapshots;

    //! Delay between two auto-snapshots in seconds
    isize snapshotDelay;

    //! Selects the snapshot compression method
    Compressor snapshotCompressor;

    //! Indicates whether workspace media files should be compressed
    bool compressWorkspaces;
}
AmigaConfig;

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

typedef u32 RunLoopFlags;

namespace RL
{
constexpr u32 STOP               = (1 << 0);
constexpr u32 SOFTSTOP_REACHED   = (1 << 1);
constexpr u32 EOL_REACHED        = (1 << 2);
constexpr u32 EOF_REACHED        = (1 << 3);
constexpr u32 BREAKPOINT_REACHED = (1 << 4);
constexpr u32 WATCHPOINT_REACHED = (1 << 5);
constexpr u32 CATCHPOINT_REACHED = (1 << 6);
constexpr u32 SWTRAP_REACHED     = (1 << 7);
constexpr u32 BEAMTRAP_REACHED   = (1 << 8);
constexpr u32 COPPERBP_REACHED   = (1 << 9);
constexpr u32 COPPERWP_REACHED   = (1 << 10);
constexpr u32 AUTO_SNAPSHOT      = (1 << 11);
constexpr u32 USER_SNAPSHOT      = (1 << 12);
constexpr u32 SYNC_THREAD        = (1 << 13);
};

}
