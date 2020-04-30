// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef VACONFIG_H
#define VACONFIG_H

//
// Release settings
//

// Snapshot version number
#define V_MAJOR 0
#define V_MINOR 9
#define V_SUBMINOR 2

// Assertion checking (uncomment in release build)
// #define NDEBUG


//
// Debug settings
//

// Uncomment to set a breakpoint on startup
// #define INITIAL_BREAKPOINT 0xFE4510

// Set to false to disable SSE optimizations
static const bool USE_SSE = true;

// Uncomment to colorize certain rasterlines
// #define LINE_DEBUG (agnus.pos.v == 260)

// Register debugging (set to 1 to generate debug output)

static const int OCSREG_DEBUG  = 2;  // General OCS register debugging
static const int ECSREG_DEBUG  = 2;  // Special ECS register debugging
static const int BLTREG_DEBUG  = 2;  // Blitter registers
static const int INTREG_DEBUG  = 2;  // Interrupt registers
static const int DSKREG_DEBUG  = 2;  // Disk controller registers
static const int CIAREG_DEBUG  = 2;  // CIA registers
static const int BPLREG_DEBUG  = 2;  // Bitplane registers
static const int SPRREG_DEBUG  = 2;  // Sprite registers
static const int AUDREG_DEBUG  = 2;  // Audio registers
static const int COPREG_DEBUG  = 2;  // Copper registers
static const int COLREG_DEBUG  = 2;  // Color registers
static const int POSREG_DEBUG  = 2;  // POSxxx registers
static const int JOYREG_DEBUG  = 2;  // JOYxxx registers
static const int INVREG_DEBUG  = 2;  // Ivalid register accesses

// Component debugging (set to 1 to generate debug output)

static const int RUN_DEBUG     = 2;  // Run loop, component states
static const int CPU_DEBUG     = 2;  // CPU
static const int INT_DEBUG     = 2;  // Interrupts
static const int CIA_DEBUG     = 2;  // CIAs
static const int TOD_DEBUG     = 2;  // TODs (CIA 24-bit counters)
static const int RTC_DEBUG     = 2;  // Real-time clock
static const int RAM_DEBUG     = 2;  // RAM
static const int DMA_DEBUG     = 2;  // DMA registers
static const int BPL_DEBUG     = 2;  // Bitplane DMA
static const int DIW_DEBUG     = 2;  // Display window
static const int DDF_DEBUG     = 2;  // Display data fetch
static const int SPR_DEBUG     = 2;  // Sprites
static const int CLX_DEBUG     = 2;  // Collision detection (CLXDAT, CLXCON)
static const int DSK_DEBUG     = 2;  // Disk controller
static const int AUD_DEBUG     = 2;  // Audio
static const int AUDBUF_DEBUG  = 2;  // Audio buffers
static const int PORT_DEBUG    = 2;  // Control ports and connected devices
static const int COP_DEBUG     = 2;  // Copper
static const int BLT_DEBUG     = 2;  // Blitter
static const int BLTTIM_DEBUG  = 2;  // Blitter Timing
static const int SER_DEBUG     = 2;  // Serial interface
static const int POT_DEBUG     = 2;  // Potentiometer inputs
static const int KBD_DEBUG     = 2;  // Keyboard
static const int SNP_DEBUG     = 2;  // Snapshot debugging (Serialization)
static const int MFM_DEBUG     = 2;  // Disc encoder / decoder


// Checksum computation (set to true to compute checksums)

static const int DSK_CHECKSUM  = false;  // Disk checksums
static const int BLT_CHECKSUM  = false;  // Blitter checksums
static const int COP_CHECKSUM  = false;  // Copper checksums

// Drive debugging (set to true to enable debugging)

static const bool DRIVE_DEBUG = false;   // Fallback to a simple turbo drive
static const bool FORCE_FASTBLT = false; // Always use the fast Blitter

// Uncomment to lauch the emulator with a preloaed disk in df0
// #define BOOT_DISK "/Downloads/Cabal.adf"
// #define BOOT_DISK "/Users/hoff/Dropbox/Amiga/Games/Pinball.adf"
// #define BOOT_DISK "/Users/hoff/Downloads/Ghostsngoblins.adf"

// Additional debug settings (uncomment to enable)
#define HARD_RESET        // Restores the initial power up state in reset()
// #define BORDER_DEBUG      // Draws the border in debug colors
// #define PIXEL_DEBUG       // Highlights the first pixel in each word
// #define SLOW_BLT_DEBUG    // Executes SlowBlitter instructions in one chunk
// #define AGNUS_EXEC_DEBUG  // Falls back to a simpler Agnus execution function
// #define CIA_ON_STEROIDS   // Disables the CIA sleep logic

#endif
