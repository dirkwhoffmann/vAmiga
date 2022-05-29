// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Frequencies
//

// Clock rate of the master clock in Hz (PAL Amiga, 28.37516 MHz)
#define MASTER_FREQUENCY 28375160

// Clock rate of the Motorola 68000 CPU in Hz (7.09379 MHz)
#define CPU_FREQUENY (28375160 / 4)

// Clock rate of the DMA bus in Hz (3.546895 MHz)
#define DMA_FREQUENY (28375160 / 8)


//
// Screen parameters
//

/* Beam positions
 *
 * Vertical coordinates are measured in scanlines.
 * Horizontal coordinates are measured in DMA cycles.
 */

#define VPOS_MAX      312
#define VPOS_CNT      313

#define HPOS_MAX_PAL  226
#define HPOS_CNT_PAL  227

#define HPOS_MAX_NTSC 227
#define HPOS_CNT_NTSC 228

#define HPOS_MAX      HPOS_MAX_NTSC
#define HPOS_CNT      HPOS_CNT_NTSC


/* Screen buffer dimensions
 *
 * All values are measured in pixels. One DMA cycle corresponds to 4 pixels.
 * Hence, HPIXELS equals 4 * HPOS_CNT_PAL. VPIXELS is one greater than VPOS_CNT,
 * because of the misalignment offset applied to the screen buffer start
 * address (see below).
 */

#define VPIXELS       314                    // VPOS_CNT + 1 line
#define HPIXELS       908                    // 4 * HPOS_CNT_PAL
#define PIXELS        (VPIXELS * HPIXELS)


/* Blanking area
 *
 * To understand the horizontal position of the Amiga screen, it is important
 * to note that the HBLANK area does *not* start at DMA cycle 0. According to
 * "Amiga Intern", DMA cycle $0F (15) is the first and $35 (53) the last cycles
 * inside the HBLANK area. However, these values seem to be wrong and I am
 * using different values instead.
 *
 * As a result, the early DMA cycles do not appear on the left side of the
 * screen, but on the right side in the previous scanline. To mimic this
 * behaviour, a misalignment offset is added to the start address of the screen
 * buffer before it is written into the GPU texture. The offset is chosen such
 * that the HBLANK area starts at the first pixel of each line in the texture.
 * As a side effect of adding this offset, constant VPIXELS needs to be greater
 * than VPOS_CNT. Otherwise, we would access unallocated memory at the end of
 * the last scanline.
 */

#define HBLANK_MIN    0x12 // 0x0A
#define HBLANK_MAX    0x23 // 0x30
#define HBLANK_CNT    0x12 // 0x27 // equals HBLANK_MAX - HBLANK_MIN + 1

#define VBLANK_MIN    0x00
#define VBLANK_MAX    0x19
#define VBLANK_CNT    0x1A // equals VBLANK_MAX - VBLANK_MIN + 1


//
// Hard drive geometry limits
//

#define HDR_C_MIN 16
#define HDR_C_MAX 1024
#define HDR_H_MIN 1
#define HDR_H_MAX 16
#define HDR_S_MIN 16
#define HDR_S_MAX 63


//
// Custom registers
//

// DMACON register bits
#define BBUSY  0x4000
#define BZERO  0x2000
#define BLTPRI 0x0400
#define DMAEN  0x0200
#define BPLEN  0x0100
#define COPEN  0x0080
#define BLTEN  0x0040
#define SPREN  0x0020
#define DSKEN  0x0010
#define AUD3EN 0x0008
#define AUD2EN 0x0004
#define AUD1EN 0x0002
#define AUD0EN 0x0001

#define AUDEN  0x000F
