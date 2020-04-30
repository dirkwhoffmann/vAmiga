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

// Uncomment to fallback to a simpler Agnus execution function
// #define AGNUS_EXEC_DEBUG

// Uncomment to lauch the emulator with a disk in df0
// #define BOOT_DISK "/Downloads/Cabal.adf"
// #define BOOT_DISK "/Users/hoff/Dropbox/Amiga/Games/Pinball.adf"
// #define BOOT_DISK "/Users/hoff/Downloads/Ghostsngoblins.adf"

// Uncomment to colorize certain rasterlines
// #define LINE_DEBUG (agnus.pos.v == 260 || agnus.pos.v == 300)

// Optimizations
static const int NO_SSE          = 0;  // Don't use SSE extensions

// Runloop and CPU
static const int RUN_DEBUG       = 2;  // Run loop, component states
static const int SNP_DEBUG       = 2;  // Serialization (snapshots)
static const int CPU_DEBUG       = 2;  // CPU
static const int HARD_RESET      = 0;  // Restores the power up state in reset()

// Memory access
static const int OCSREG_DEBUG    = 2;  // General OCS register debugging
static const int ECSREG_DEBUG    = 2;  // Special ECS register debugging
static const int INVREG_DEBUG    = 2;  // Ivalid register accesses
static const int RAM_DEBUG       = 2;  // Fast RAM

// Agnus
static const int DMA_DEBUG       = 2;  // DMA registers
static const int DDF_DEBUG       = 2;  // Display data fetch

// Denise
static const int BPLREG_DEBUG    = 2;  // Bitplane registers
static const int SPRREG_DEBUG    = 2;  // Sprite registers
static const int COLREG_DEBUG    = 2;  // Color registers
static const int BPL_DEBUG       = 2;  // Bitplane DMA
static const int DIW_DEBUG       = 2;  // Display window
static const int SPR_DEBUG       = 2;  // Sprites
static const int CLX_DEBUG       = 2;  // Collision detection (CLXDAT, CLXCON)
static const int BORDER_DEBUG    = 0;  // Draw the border in debug colors

// Paula
static const int INTREG_DEBUG    = 2;  // Interrupt registers
static const int INT_DEBUG       = 2;  // Interrupt logic

// CIAs
static const int CIA_ON_STEROIDS = 0;  // Keep the CIAs awake all the time
static const int CIAREG_DEBUG    = 2;  // CIA registers
static const int CIA_DEBUG       = 2;  // CIA execution
static const int TOD_DEBUG       = 2;  // TODs (CIA 24-bit counters)

// Blitter
static const int BLT_CHECKSUM    = 0;  // Compute Blitter checksums
static const int BLTREG_DEBUG    = 2;  // Blitter registers
static const int BLT_DEBUG       = 2;  // Blitter execution
static const int BLTTIM_DEBUG    = 2;  // Blitter Timing
static const int FORCE_FASTBLT   = 0;  // Always use the fast Blitter
static const int SLOW_BLT_DEBUG  = 0;  // Execute micro-instruction in one chunk

// Copper
static const int COP_CHECKSUM    = 0;  // Compute Copper checksums
static const int COPREG_DEBUG    = 2;  // Copper registers
static const int COP_DEBUG       = 2;  // Copper execution

// Drive
static const int DSK_CHECKSUM    = 0;  // Compute disk checksums
static const int DSKREG_DEBUG    = 2;  // Disk controller registers
static const int DSK_DEBUG       = 2;  // Disk controller execution
static const int DRIVE_DEBUG     = 0;  // Fallback to a simple turbo drive
static const int MFM_DEBUG       = 2;  // Disk encoder / decoder

// Audio
static const int AUDREG_DEBUG    = 2;  // Audio registers
static const int AUD_DEBUG       = 2;  // Audio execution
static const int AUDBUF_DEBUG    = 2;  // Audio buffers

// Ports
static const int POSREG_DEBUG    = 2;  // POSxxx registers
static const int JOYREG_DEBUG    = 2;  // JOYxxx registers
static const int PORT_DEBUG      = 2;  // Control ports and connected devices
static const int SER_DEBUG       = 2;  // Serial interface
static const int POT_DEBUG       = 2;  // Potentiometer inputs

// Other components
static const int RTC_DEBUG       = 2;  // Real-time clock
static const int KBD_DEBUG       = 2;  // Keyboard

#endif
