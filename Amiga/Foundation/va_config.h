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

// Default debug level (used for statements with no custom level)
#define DEBUG_LEVEL 1

// Set to 1 to debug the CIAs
#define CIA_DEBUG 2

// Set to 1 to debug DMA relevant registers
#define DMA_DEBUG 2

// Set to 1 to debug bitplane relevant registers
#define BPL_DEBUG 2

// Set to 1 to debug color registers
#define COL_DEBUG 2

// Set to 1 to debug sprite relevant registers
#define SPR_DEBUG 2

// Set to 1 to debug disk relevant registers
#define DSK_DEBUG 2

// Set to 1 to debug audio relevant registers
#define AUD_DEBUG 2

// Set to 1 to debug the control ports and connected devices
#define CPT_DEBUG 2

// Set to 1 to debug Copper registers
#define COPREG_DEBUG 2

// Set to 1 to debug the Copper
#define COP_DEBUG 2

// Set to 1 to debug the Blitter
#define BLT_DEBUG 2

// Set to 1 to debug the serial interface
#define SER_DEBUG 2

// Enable to draw the display window border in debug colors
// #define BORDER_DEBUG

// Enable to debug the biplane shift registers
// #define SHIFTREG_DEBUG

// Enable to highlight the first pixel in a 16-bit lores or hires pixel chunk
// #define PIXEL_DEBUG

// Enable to colorize certain rasterlines
// #define LINE_DEBUG

// Enable to make drive operations deterministic
// #define ALIGN_DRIVE_HEAD

#endif
