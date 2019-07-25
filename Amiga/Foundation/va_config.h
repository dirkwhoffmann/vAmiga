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
#define RUN_DEBUG 2         // Emulator thread run loop
#define CIA_DEBUG 2         // CIAs
#define TOD_DEBUG 2         // TODs (24-bit counters inside the CIAs)
#define RTC_DEBUG 2         // Real-time clock
#define DMA_DEBUG 2         // DMA registers
#define BPL_DEBUG 2         // Bitplane registers
#define COL_DEBUG 2         // Color registers
#define SPR_DEBUG 2         // Sprite registers
#define DSK_DEBUG 2         // Disk registers
#define AUD_DEBUG 2         // Audio registers
#define CPT_DEBUG 2         // Control ports and connected devices
#define COPREG_DEBUG 2      // Copper registers
#define COP_DEBUG 2         // Copper internals
#define BLT_DEBUG 2         // Blitter registers
#define BLT_CHECKSUM 2      // Blitter checksum computation
#define SER_DEBUG 2         // Serial interface
#define POT_DEBUG 2         // Potentiometer inputs
#define KB_DEBUG 1          // Keyboard
#define OCS_DEBUG 2         // Custom register access violations
#define SNAP_DEBUG 2        // Snapshot debugging (state serialization)


// Additional debugging aids (uncomment to enable)
// #define BORDER_DEBUG     // Draws the border in debug colors
// #define PIXEL_DEBUG      // Highlight first pixel in each 16-bit pixel chunk
// #define LINE_DEBUG       // Colorizes certain rasterlines
// #define ALIGN_DRIVE_HEAD // Makes drive operations deterministic

#endif
