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
#define V_MINOR 1
#define V_SUBMINOR 0

// Assertion checking (uncomment in a release build)
// #define NDEBUG

//
// Debug settings
//

// Custom debug levels (set to 1 to generate debug output)

static const int RUNLOOP_DEBUG = 2;  // Run loop of the emulator thread
static const int CIA_DEBUG     = 2;  // CIAs
static const int TOD_DEBUG     = 2;  // TODs (CIA 24-bit counters)
static const int RTC_DEBUG     = 2;  // Real-time clock
static const int DMA_DEBUG     = 2;  // DMA registers
static const int BPL_DEBUG     = 2;  // Bitplane DMA
static const int BPLREG_DEBUG  = 2;  // Bitplane registers
static const int DIW_DEBUG     = 2;  // Display window logic
static const int DDF_DEBUG     = 2;  // Display data fetch logic
static const int COL_DEBUG     = 2;  // Color registers
static const int SPR_DEBUG     = 2;  // Sprite registers
static const int CLX_DEBUG     = 2;  // Collision detection (CLXDAT, CLXCON)
static const int DSK_DEBUG     = 2;  // Disk controller
static const int DSK_CHECKSUM  = 1;  // Disk checksums
static const int AUD_DEBUG     = 2;  // Audio
static const int AUDBUF_DEBUG  = 2;  // Audio buffers
static const int AUDREG_DEBUG  = 2;  // Audio registers
static const int PORT_DEBUG    = 2;  // Control ports and connected devices
static const int COP_DEBUG     = 2;  // Copper
static const int COPREG_DEBUG  = 2;  // Copper registers
static const int BLT_DEBUG     = 2;  // Blitter
static const int BLITREG_DEBUG = 2;  // Blitter registers
static const int BLIT_CHECKSUM = 1;  // Blitter checksums
static const int SER_DEBUG     = 2;  // Serial interface
static const int POT_DEBUG     = 2;  // Potentiometer inputs
static const int KB_DEBUG      = 1;  // Keyboard
static const int REGACC_DEBUG  = 2;  // Register access violations
static const int SNAP_DEBUG    = 2;  // Snapshot debugging (serialization)

// Additional debugging aids (uncomment to enable)
// #define BORDER_DEBUG     // Draws the border in debug colors
// #define PIXEL_DEBUG      // Highlight first pixel in each 16-bit pixel chunk
// #define LINE_DEBUG       // Colorizes certain rasterlines
#define ALIGN_DRIVE_HEAD // Makes drive operations deterministic

#endif
