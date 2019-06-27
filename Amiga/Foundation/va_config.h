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
// Release options (Uncomment these options for a release build!)
//

// Disables assertion checking
// #define NDEBUG


//
// Constants
//

// Snapshot version number of this release
#define V_MAJOR 0
#define V_MINOR 1
#define V_SUBMINOR 0


//
// Debug settings
//

// Default debug level (used for statements with no custom level) DEPRECATED
#define DEBUG_LEVEL 1

//
// Custom debug levels (set to 1 to enable debug output)
//

// Debugging the run loop
#define RUN_DEBUG 2

// Debugging the Complex Interface Adapters (CIAs)
#define CIA_DEBUG 2

// Debugging DMA registers
#define DMA_DEBUG 2

// Debugging bitplane registers
#define BPL_DEBUG 2

// Debugging color registers
#define COL_DEBUG 2

// Debugging sprite registers
#define SPR_DEBUG 2

// Debugging disk registers
#define DSK_DEBUG 2

// Debugging audio registers
#define AUD_DEBUG 2

// Debugging the control ports and connected devices
#define CPT_DEBUG 2

// Debugging the Copper
#define COPREG_DEBUG 2
#define COP_DEBUG 2

// Debugging the Blitter
#define BLT_DEBUG 2
#define BLT_CHECKSUM 2

// Debugging the serial interface
#define SER_DEBUG 2

// Debugging the potentiometer inputs
#define POT_DEBUG 2

// Debugging the keyboard
#define KB_DEBUG 1

// Debugging custom register access violations
#define OCS_DEBUG 2

//
//
//

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
