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

static const int DB_RUNLOOP = 2;  // Run loop of the emulator thread
static const int DB_CIA     = 2;  // CIAs
static const int DB_TOD     = 2;  // TODs (CIA 24-bit counters)
static const int DB_RTC     = 2;  // Real-time clock
static const int DB_DMA     = 2;  // DMA registers
static const int DB_BPL     = 2;  // Bitplane registers
static const int DB_COL     = 2;  // Color registers
static const int DB_SPR     = 2;  // Sprite registers
static const int DB_CLX     = 2;  // Collision detection (CLXDAT, CLXCON)
static const int DB_DSK     = 2;  // Disk registers
static const int DB_AUD     = 2;  // Audio 
static const int DB_AUDBUF  = 2;  // Audio buffers
static const int DB_AUDREG  = 2;  // Audio registers
static const int DB_PORTS   = 2;  // Control ports and connected devices
static const int DB_COP     = 2;  // Copper
static const int DB_COPREG  = 2;  // Copper registers
static const int DB_BLT     = 2;  // Blitter
static const int DB_BLT_CHK = 2;  // Blitter checksums
static const int DB_SER     = 2;  // Serial interface
static const int DB_POT     = 2;  // Potentiometer inputs
static const int DB_KB      = 1;  // Keyboard
static const int DB_ACCESS  = 2;  // Register access violations
static const int DB_SNAP    = 2;  // Snapshot debugging (serialization)

// Additional debugging aids (uncomment to enable)
// #define BORDER_DEBUG     // Draws the border in debug colors
// #define PIXEL_DEBUG      // Highlight first pixel in each 16-bit pixel chunk
// #define LINE_DEBUG       // Colorizes certain rasterlines
// #define ALIGN_DRIVE_HEAD // Makes drive operations deterministic

#endif
