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

// Clock rate of the master clock in Hz
#define CLK_FREQUENCY_PAL   28375160                    // 28.375160 MHz
#define CLK_FREQUENCY_NTSC  28636360                    // 28.636360 MHz

// Clock rate of the CPU in Hz
#define CPU_FREQUENY_PAL    (MAS_FREQUENCY_PAL / 4)     //  7.093790 MHz
#define CPU_FREQUENY_NTSC   (MAS_FREQUENCY_NTSC / 4)    //  7.159090 MHz

// Clock rate of the DMA bus in Hz
#define DMA_FREQUENY_PAL    (MAS_FREQUENCY_PAL / 8)     //  3.546895 MHz
#define DMA_FREQUENY_NTSC   (MAS_FREQUENCY_NTSC / 8)    //  3.579545 MHz


//
// Screen parameters
//

/* Beam positions
 *
 * Vertical coordinates are measured in scanlines
 * Horizontal coordinates are measured in DMA cycles
 */

#define VPOS_CNT_PAL_LF     313
#define VPOS_CNT_PAL_SF     312
#define VPOS_MAX_PAL_LF     312
#define VPOS_MAX_PAL_SF     311
#define VPOS_CNT_PAL        VPOS_CNT_PAL_LF
#define VPOS_MAX_PAL        VPOS_MAX_PAL_LF

#define VPOS_CNT_NTSC_LF    263
#define VPOS_CNT_NTSC_SF    262
#define VPOS_MAX_NTSC_LF    262
#define VPOS_MAX_NTSC_SF    261
#define VPOS_CNT_NTSC       VPOS_CNT_NTSC_LF
#define VPOS_MAX_NTSC       VPOS_MAX_NTSC_LF

#define VPOS_CNT            VPOS_CNT_PAL
#define VPOS_MAX            VPOS_MAX_PAL

#define HPOS_CNT_PAL        227
#define HPOS_MAX_PAL        226

#define HPOS_CNT_NTSC_LL    228
#define HPOS_CNT_NTSC_SL    227
#define HPOS_MAX_NTSC_LL    227
#define HPOS_MAX_NTSC_SL    226
#define HPOS_CNT_NTSC       HPOS_CNT_NTSC_LL
#define HPOS_MAX_NTSC       HPOS_MAX_NTSC_LL

#define HPOS_MAX            HPOS_MAX_NTSC_LL
#define HPOS_CNT            HPOS_CNT_NTSC_LL


/* Screen buffer dimensions
 *
 * All values are measured in pixels. One DMA cycle corresponds to 4 pixels.
 * Hence, HPIXELS equals 4 * HPOS_CNT.
 */
// #define TPC         sizeof(Texel)       // Texels per DMA cycle (4 or 8)
#define VPIXELS     313                 // VPOS_CNT
#define HPIXELS     912                 // 4 * HPOS_CNT
#define PIXELS      (VPIXELS * HPIXELS)


/* Blanking area
 *
 * To understand the horizontal alignment of the Amiga screen, it is important
 * to note that the HBLANK area does *not* start at DMA cycle 0. According to
 * "Amiga Intern", DMA cycle $0F is the first and $35 the last cycles inside
 * the HBLANK area. However, these values seem to be wrong. According to the
 * Agnus schematics, the first HBLANK cycle is $12 and the last cycle is $23.
 * As a result, the early DMA cycles do not appear on the left side of the
 * screen.
 */

#define HBLANK_MIN    0x12      // First HBLANK cycle
#define HBLANK_MAX    0x23      // Last HBLANK cycle
#define HBLANK_CNT    0x12      // HBLANK_MAX - HBLANK_MIN + 1

#define VBLANK_MIN    0x00      // First VBLANK line
#define VBLANK_MAX    0x19      // Last VBLANK line
#define VBLANK_CNT    0x1A      // VBLANK_MAX - VBLANK_MIN + 1


//
// Hard drive geometry limits
//

#define HDR_C_MIN 16
#define HDR_C_MAX 16384
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
