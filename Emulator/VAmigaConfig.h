// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

//
// Release settings
//

// Version number
static constexpr int VER_MAJOR      = 4;
static constexpr int VER_MINOR      = 0;
static constexpr int VER_SUBMINOR   = 0;
static constexpr int VER_BETA       = 0;

// Snapshot version number
static constexpr int SNP_MAJOR      = 4;
static constexpr int SNP_MINOR      = 0;
static constexpr int SNP_SUBMINOR   = 0;
static constexpr int SNP_BETA       = 0;


//
// Video settings
//

/* Texels per pixel. Set to 1 to create a texture in hires resolution where
 * every hires pixel is represented by a single texel). Set to 2 to generate a
 * texture in super-hires resolution where every hires pixel is represented by
 * two texels.
 */
#define TPP 1


//
// Execution settings
//

static constexpr int DIAG_BOARD     = 0; // Plug in the diagnose board
static constexpr int ALLOW_ALL_ROMS = 0; // Disable the magic bytes check


//
// Debug settings
//

static constexpr bool betaRelease = VER_BETA != 0;

#ifdef NDEBUG
static constexpr bool releaseBuild = 1;
static constexpr bool debugBuild = 0;
typedef const int debugflag;
#else
static constexpr bool releaseBuild = 0;
static constexpr bool debugBuild = 1;
typedef int debugflag;
#endif

#ifdef __EMSCRIPTEN__
static constexpr bool emscripten = 1;
#else
static constexpr bool emscripten = 0;
#endif

// General
extern debugflag XFILES;
extern debugflag CNF_DEBUG;
extern debugflag OBJ_DEBUG;
extern debugflag DEF_DEBUG;
extern debugflag MIMIC_UAE;

// Emulator
extern debugflag RUN_DEBUG;
extern debugflag TIM_DEBUG;
extern debugflag WARP_DEBUG;
extern debugflag CMD_DEBUG;
extern debugflag MSG_DEBUG;
extern debugflag SNP_DEBUG;

// Run ahead
extern debugflag RUA_DEBUG;
extern debugflag RUA_CHECKSUM;
extern debugflag RUA_ON_STEROIDS;

// CPU
extern debugflag CPU_DEBUG;

// Memory access
extern debugflag OCSREG_DEBUG;
extern debugflag ECSREG_DEBUG;
extern debugflag INVREG_DEBUG;
extern debugflag MEM_DEBUG;

// Agnus
extern debugflag DMA_DEBUG;
extern debugflag DDF_DEBUG;
extern debugflag SEQ_DEBUG;
extern debugflag SEQ_ON_STEROIDS;
extern debugflag NTSC_DEBUG;

// Copper
extern debugflag COP_CHECKSUM;
extern debugflag COPREG_DEBUG;
extern debugflag COP_DEBUG;

// Blitter
extern debugflag BLT_CHECKSUM;
extern debugflag BLTREG_DEBUG;
extern debugflag BLT_REG_GUARD;
extern debugflag BLT_MEM_GUARD;
extern debugflag BLT_DEBUG;
extern debugflag BLTTIM_DEBUG;
extern debugflag SLOW_BLT_DEBUG;

// Denise
extern debugflag BPLREG_DEBUG;
extern debugflag BPLDAT_DEBUG;
extern debugflag BPLMOD_DEBUG;
extern debugflag SPRREG_DEBUG;
extern debugflag COLREG_DEBUG;
extern debugflag CLXREG_DEBUG;
extern debugflag BPL_ON_STEROIDS;
extern debugflag DIW_DEBUG;
extern debugflag SPR_DEBUG;
extern debugflag CLX_DEBUG;
extern debugflag BORDER_DEBUG;
extern debugflag LINE_DEBUG;
extern debugflag DENISE_ON_STEROIDS;

// Paula
extern debugflag INTREG_DEBUG;
extern debugflag INT_DEBUG;

// CIAs
extern debugflag CIAREG_DEBUG;
extern debugflag CIASER_DEBUG;
extern debugflag CIA_DEBUG;
extern debugflag TOD_DEBUG;

// Floppy Drives
extern debugflag ALIGN_HEAD;
extern debugflag DSK_CHECKSUM;
extern debugflag DSKREG_DEBUG;
extern debugflag DSK_DEBUG;
extern debugflag MFM_DEBUG;
extern debugflag FS_DEBUG;

// Hard Drives
extern debugflag HDR_ACCEPT_ALL;
extern debugflag HDR_FS_LOAD_ALL;
extern debugflag WT_DEBUG;

// Audio
extern debugflag AUDREG_DEBUG;
extern debugflag AUD_DEBUG;
extern debugflag AUDBUF_DEBUG;
extern debugflag AUDVOL_DEBUG;
extern debugflag DISABLE_AUDIRQ;

// Ports
extern debugflag POSREG_DEBUG;
extern debugflag JOYREG_DEBUG;
extern debugflag POTREG_DEBUG;
extern debugflag VID_DEBUG;
extern debugflag PRT_DEBUG;
extern debugflag SER_DEBUG;
extern debugflag POT_DEBUG;
extern debugflag HOLD_MOUSE_L;
extern debugflag HOLD_MOUSE_M;
extern debugflag HOLD_MOUSE_R;

// Expansion boards
extern debugflag ZOR_DEBUG;
extern debugflag ACF_DEBUG;
extern debugflag FAS_DEBUG;
extern debugflag HDR_DEBUG;
extern debugflag DBD_DEBUG;

// Media types
extern debugflag ADF_DEBUG;
extern debugflag HDF_DEBUG;
extern debugflag DMS_DEBUG;
extern debugflag IMG_DEBUG;

// Real-time clock
extern debugflag RTC_DEBUG;

// Keyboard
extern debugflag KBD_DEBUG;
extern debugflag KEY_DEBUG;

// Misc
extern debugflag REC_DEBUG;
extern debugflag SCK_DEBUG;
extern debugflag SRV_DEBUG;
extern debugflag GDB_DEBUG;


//
// Forced error conditions
//

extern debugflag FORCE_ROM_MISSING;
extern debugflag FORCE_CHIP_RAM_MISSING;
extern debugflag FORCE_AROS_NO_EXTROM;
extern debugflag FORCE_AROS_RAM_LIMIT;
extern debugflag FORCE_CHIP_RAM_LIMIT;
extern debugflag FORCE_SNAP_TOO_OLD;
extern debugflag FORCE_SNAP_TOO_NEW;
extern debugflag FORCE_SNAP_IS_BETA;
extern debugflag FORCE_SNAP_CORRUPTED;
extern debugflag FORCE_DISK_INVALID_LAYOUT;
extern debugflag FORCE_DISK_MODIFIED;
extern debugflag FORCE_HDR_TOO_LARGE;
extern debugflag FORCE_HDR_UNSUPPORTED_C;
extern debugflag FORCE_HDR_UNSUPPORTED_H;
extern debugflag FORCE_HDR_UNSUPPORTED_S;
extern debugflag FORCE_HDR_UNSUPPORTED_B;
extern debugflag FORCE_HDR_UNKNOWN_GEOMETRY;
extern debugflag FORCE_HDR_MODIFIED;
extern debugflag FORCE_FS_WRONG_BSIZE;
extern debugflag FORCE_FS_WRONG_CAPACITY;
extern debugflag FORCE_FS_WRONG_DOS_TYPE;
extern debugflag FORCE_DMS_CANT_CREATE;
extern debugflag FORCE_RECORDING_ERROR;
extern debugflag FORCE_NO_FFMPEG;
extern debugflag FORCE_ZLIB_ERROR;

#include <assert.h>
