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

// Assertion checking
// #define NDEBUG   // Uncomment this option in a release build


//
// Debug settings
//

// Custom debug levels for certain components.
// Set to 1 to generate debug output.

#define RUN_DEBUG 2    // Emulator thread run loop
#define CIA_DEBUG 2    // CIAs
#define TOD_DEBUG 1    // TODs (24-bit counters inside the CIAs)
#define RTC_DEBUG 2    // Real-time clock
#define DMA_DEBUG 2    // DMA registers
#define BPL_DEBUG 2    // Bitplane registers
#define COL_DEBUG 2    // Color registers
#define SPR_DEBUG 2    // Sprite registers
#define DSK_DEBUG 2    // Disk registers
#define AUD_DEBUG 2    // Audio registers
#define CPT_DEBUG 2    // Control ports and connected devices
#define COPREG_DEBUG 2 // Copper registers
#define COP_DEBUG 2    // Copper internals
#define BLT_DEBUG 2    // Blitter registers
#define BLT_CHECKSUM 2 // Blitter checksum computation
#define SER_DEBUG 2    // Serial interface
#define POT_DEBUG 2    // Potentiometer inputs
#define KB_DEBUG 1     // Keyboard
#define OCS_DEBUG 2    // Custom register access violations


// Enable to draw the display window border in debug colors
// #define BORDER_DEBUG

// Enable to debug the biplane shift registers
// #define SHIFTREG_DEBUG

// Enable to highlight the first pixel in a 16-bit lores or hires pixel chunk
// #define PIXEL_DEBUG

// Enable to colorize certain rasterlines
// #define LINE_DEBUG

// Enable to make drive operations deterministic
#define ALIGN_DRIVE_HEAD

#endif
