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

// Custom register addresses

#define DMACON 0x96L


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
// Screen parameters
//

// Vertical screen buffer size
static const long VPIXELS = 288;

// Horizontal screen buffer size
static const long HPIXELS = 768;

// Screen buffer size
static const long BUFSIZE = VPIXELS * HPIXELS * 4;

#endif 
