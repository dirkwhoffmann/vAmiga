// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef VA_CONSTANTS_H
#define VA_CONSTANTS_H

// Clock rate of the master clock (PAL)
static const double masterClockFrequency = 28.37516;

// Clock rate of the Motorola 68000 CPU (7.09379 MHz)
static const double cpuClockFrequency = masterClockFrequency / 4.0;

// Frame rate (PAL)
// IS THIS 100% CORRECT? E.G., THE C64 DIFFERS SLIGHTLHY FROM 50
static const double frameRate = 50.0;

// Number of master clock cycles per frame
static const double masterCyclesPerFrame = (masterClockFrequency * 1000000.0) / frameRate;


// Printable names for all custom registers
static const char *customReg[256] = {
    
    "BLTDDAT",      "DMACONR",      "VPOSR",
    "VHPOSR",       "DSKDATR",      "JOY0DAT",
    "JOY1DAT",      "CLXDAT",       "ADKCONR",
    "POT0DAT",      "POT1DAT",      "POTINP",
    "SERDATR",      "DSKBYTR",      "INTENAR",
    "INTREQR",      "DSKPTH",       "DSKPTL",
    "DSKLEN",       "DSKDAT",       "REFPTR",
    "VPOSW",        "VHPOSW",       "COPCON",
    "SERDAT",       "SERPER",       "POTGO",
    "JOYTEST",      "STREQU",       "STRVBL",
    "STRHOR",       "STRLONG",      "BLTCON0",
    "BLTCON1",      "BLTAFWM",      "BLTALWM",
    "BLTCPTH",      "BLTCPTL",      "BLTBPTH",
    "BLTBPTL",      "BLTAPTH",      "BLTAPTL",
    "BLTDPTH",      "BLTDPTL",      "BLTSIZE",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "BLTCMOD",      "BLTBMOD",      "BLTAMOD",
    "BLTDMOD",      "unused",       "unused",
    "unused",       "unused",       "BLTCDAT",
    "BLTBDAT",      "BLTADAT",      "unused",
    "unused (ECS)", "unused",       "unused (ECS)",
    "DSKSYNC",      "COP1LCH",      "COP1LCL",
    "COP2LCH",      "COP2LCL",      "COPJMP1",
    "COPJMP2",      "COPINS",       "DIWSTRT",
    "DIWSTOP",      "DDFSTRT",      "DDFSTOP",
    "DMACON",       "CLXCON",       "INTENA",
    "INTREQ",       "ADKCON",       "AUD0PTH",
    "AUD0PTL",      "AUD0LEN",      "AUD0PER",
    "AUD0VOL",      "AUD0DAT",      "unused",
    "UNUSED",       "AUD1PTH",      "AUD1PTL",
    "AUD1LEN",      "AUD1PER",      "AUD1VOL",
    "AUD1DAT",      "unused",       "unused",
    "AUD2PTH",      "AUD2PTL",      "AUD2LEN",
    "AUD2PER",      "AUD2VOL",      "AUD2DAT",
    "unused",       "unused",       "AUD3PTH",
    "AUD3PTL",      "AUD3LEN",      "AUD3PER",
    "AUD3VOL",      "AUD3DAT",      "unused",
    "UNUSED",       "BPL0PTH",      "BPL0PTL",
    "BPL1PTH",      "BPL1PTL",      "BPL2PTH",
    "BPL2PTL",      "BPL3PTH",      "BPL3PTL",
    "BPL4PTH",      "BPL4PTL",      "BPL5PTH",
    "BPL5PTL",      "unused",       "unused",
    "unused",       "unused",       "BPLCON0",
    "BPLCON1",      "BPLCON2",      "unused (ECS)",
    "BPL1MOD",      "BPL2MOD",      "unused",
    "unused",       "BPL0DAT",      "BPL1DAT",
    "BPL2DAT",      "BPL3DAT",      "BPL4DAT",
    "BPL5DAT",      "unused",       "unused",
    "SPR0PTH",      "SPR0PTL",      "SPR1PTH",
    "SPR1PTL",      "SPR2PTH",      "SPR2PTL",
    "SPR3PTH",      "SPR3PTL",      "SPR4PTH",
    "SPR4PTL",      "SPR5PTH",      "SPR5PTL",
    "SPR6PTH",      "SPR6PTL",      "SPR7PTH",
    "SPR7PTL",      "SPR0POS",      "SPR0CTL",
    "SPR0DATA",     "SPR0DATB",     "SPR1POS",
    "SPR1CTL",      "SPR1DATA",     "SPR1DATB",
    "SPR2POS",      "SPR2CTL",      "SPR2DATA",
    "SPR2DATB",     "SPR3POS",      "SPR3CTL",
    "SPR3DATA",     "SPR3DATB",     "SPR4POS",
    "SPR4CTL",      "SPR4DATA",     "SPR4DATB",
    "SPR5POS",      "SPR5CTL",      "SPR5DATA",
    "SPR5DATB",     "SPR6POS",      "SPR6CTL",
    "SPR6DATA",     "SPR6DATB",     "SPR7POS",
    "SPR7CTL",      "SPR7DATA",     "SPR7DATB",
    "COLOR00",      "COLOR01",      "COLOR02",
    "COLOR03",      "COLOR04",      "COLOR05",
    "COLOR06",      "COLOR07",      "COLOR08",
    "COLOR09",      "COLOR10",      "COLOR11",
    "COLOR12",      "COLOR13",      "COLOR14",
    "COLOR15",      "COLOR16",      "COLOR17",
    "COLOR18",      "COLOR19",      "COLOR20",
    "COLOR21",      "COLOR22",      "COLOR23",
    "COLOR24",      "COLOR25",      "COLOR26",
    "COLOR27",      "COLOR28",      "COLOR29",
    "COLOR30",      "COLOR31",      "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused (ECS)",
    "unused (ECS)", "unused (ECS)", "unused",
    "unused",       "unused",       "unused",
    "unused",       "unused",       "unused",
    "NO-OP(NULL)"
};



//
// DEPRECATED (from VirtualC64)
//

// Horizontal screen parameters

//! @brief    Width of left VBLANK area in PAL mode
static const uint16_t PAL_LEFT_VBLANK = 76;

//! @brief    Width of left border in PAL mode
static const uint16_t PAL_LEFT_BORDER_WIDTH = 48;

//! @brief    Width of canvas area in PAL mode
static const uint16_t PAL_CANVAS_WIDTH = 320;

//! @brief    Width of right border in PAL mode
static const uint16_t PAL_RIGHT_BORDER_WIDTH = 37;

//! @brief    Width of right VBLANK area in PAL mode
static const uint16_t PAL_RIGHT_VBLANK = 23;

//! @brief    Total width of a rasterline (including VBLANK) in PAL mode
static const uint16_t PAL_WIDTH = 504; // 76 + 48 + 320 + 37 + 23


//! @brief    Number of viewable pixels per rasterline in PAL mode
// static const uint16_t PAL_VISIBLE_PIXELS = 403;


// Vertical screen parameters

//! @brief    Number of VBLANK lines at top in PAL mode
static const uint16_t PAL_UPPER_VBLANK = 16;

//! @brief    Heigt of upper boder in PAL mode
static const uint16_t PAL_UPPER_BORDER_HEIGHT = 35;

//! @brief    Height of canvas area in PAL mode
static const uint16_t PAL_CANVAS_HEIGHT = 200;

//! @brief    Lower border height in PAL mode
static const uint16_t PAL_LOWER_BORDER_HEIGHT = 49;

//! @brief    Number of VBLANK lines at bottom in PAL mode
static const uint16_t PAL_LOWER_VBLANK = 12;

//! @brief    Total height of a frame (including VBLANK) in PAL mode
static const uint16_t PAL_HEIGHT = 312; // 16 + 35 + 200 + 49 + 12

//! @brief    Number of drawn rasterlines per frame in PAL mode
static const uint16_t PAL_RASTERLINES = 284; // 35 + 200 + 49

//! @brief    Number of viewable rasterlines per frame in PAL mode
static const uint16_t PAL_VISIBLE_RASTERLINES = 284; // was 292

//! @brief    Number of drawn pixels per rasterline in PAL mode
static const uint16_t PAL_PIXELS = 405; // 48 + 320 + 37

//! @brief    Number of drawn pixels per rasterline in NTSC mode
static const uint16_t NTSC_PIXELS = 428; // 55 + 320 + 53


#endif 
