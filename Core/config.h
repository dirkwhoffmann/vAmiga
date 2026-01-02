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
static constexpr int VER_MINOR      = 4;
static constexpr int VER_SUBMINOR   = 0;
static constexpr int VER_BETA       = 5;

// Snapshot version number
static constexpr int SNP_MAJOR      = 4;
static constexpr int SNP_MINOR      = 4;
static constexpr int SNP_SUBMINOR   = 0;
static constexpr int SNP_BETA       = 5;


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

static constexpr int        DIAG_BOARD       = 0;           // Plug in the diagnose board
static constexpr int        ALLOW_ALL_ROMS   = 1;           // Disable the magic bytes check


//
// Debug settings
//

static constexpr bool betaRelease = VER_BETA != 0;

#ifdef NDEBUG
static constexpr bool releaseBuild = 1;
static constexpr bool debugBuild = 0;
typedef const long debugflag;
#else
static constexpr bool releaseBuild = 0;
static constexpr bool debugBuild = 1;
typedef long debugflag;
#endif

#ifdef __EMSCRIPTEN__
static constexpr bool wasmBuild = 1;
#else
static constexpr bool wasmBuild = 0;
#endif

#ifdef VAMIGA_DOS
static constexpr bool vAmigaDOS = 1;
#else
static constexpr bool vAmigaDOS = 0;
#endif

namespace vamiga {

// General
constexpr long XFILES             = 0;
constexpr long CNF_DEBUG          = 0;
constexpr long OBJ_DEBUG          = 0;
constexpr long DEF_DEBUG          = 0;
constexpr long MIMIC_UAE          = 0;

// Emulator
constexpr long RUN_DEBUG          = 0;
constexpr long TIM_DEBUG          = 0;
constexpr long WARP_DEBUG         = 0;
constexpr long CMD_DEBUG          = 0;
constexpr long MSG_DEBUG          = 0;
constexpr long SNP_DEBUG          = 0;

// Run ahead
constexpr long RUA_DEBUG          = 0;
constexpr long RUA_CHECKSUM       = 0;
constexpr long RUA_ON_STEROIDS    = 0;

// CPU
constexpr long CPU_DEBUG          = 0;

// Memory access
constexpr long OCSREG_DEBUG       = 0;
constexpr long ECSREG_DEBUG       = 0;
constexpr long INVREG_DEBUG       = 0;
constexpr long MEM_DEBUG          = 0;

// Agnus
constexpr long DMA_DEBUG          = 0;
constexpr long DDF_DEBUG          = 0;
constexpr long SEQ_DEBUG          = 0;
constexpr long SEQ_ON_STEROIDS    = 0;
constexpr long NTSC_DEBUG         = 0;

// Copper
constexpr long COP_CHECKSUM       = 0;
constexpr long COPREG_DEBUG       = 0;
constexpr long COP_DEBUG          = 0;

// Blitter
constexpr long BLT_CHECKSUM       = 0;
constexpr long BLTREG_DEBUG       = 0;
constexpr long BLT_REG_GUARD      = 0;
constexpr long BLT_MEM_GUARD      = 0;
constexpr long BLT_DEBUG          = 0;
constexpr long BLTTIM_DEBUG       = 0;
constexpr long SLOW_BLT_DEBUG     = 0;

// Denise
constexpr long BPLREG_DEBUG       = 0;
constexpr long BPLDAT_DEBUG       = 0;
constexpr long BPLMOD_DEBUG       = 0;
constexpr long SPRREG_DEBUG       = 0;
constexpr long COLREG_DEBUG       = 0;
constexpr long CLXREG_DEBUG       = 0;
constexpr long BPL_ON_STEROIDS    = 0;
constexpr long DIW_DEBUG          = 0;
constexpr long SPR_DEBUG          = 0;
constexpr long CLX_DEBUG          = 0;
constexpr long BORDER_DEBUG       = 0;
constexpr long LINE_DEBUG         = 0;
constexpr long DENISE_ON_STEROIDS = 0;

// Paula
constexpr long INTREG_DEBUG       = 0;
constexpr long INT_DEBUG          = 0;

// CIAs
constexpr long CIAREG_DEBUG       = 0;
constexpr long CIASER_DEBUG       = 0;
constexpr long CIA_DEBUG          = 0;
constexpr long TOD_DEBUG          = 0;

// Floppy Drives
constexpr long ALIGN_HEAD         = 0;
constexpr long DSK_CHECKSUM       = 0;
constexpr long DSKREG_DEBUG       = 0;
constexpr long DSK_DEBUG          = 0;
constexpr long MFM_DEBUG          = 0;
constexpr long FS_DEBUG           = 0;

// Hard Drives
constexpr long HDR_ACCEPT_ALL     = 0;
constexpr long HDR_FS_LOAD_ALL    = 0;
constexpr long WT_DEBUG           = 0;

// Audio
constexpr long AUDREG_DEBUG       = 0;
constexpr long AUD_DEBUG          = 0;
constexpr long AUDBUF_DEBUG       = 0;
constexpr long AUDVOL_DEBUG       = 0;
constexpr long DISABLE_AUDIRQ     = 0;

// Ports
constexpr long POSREG_DEBUG       = 0;
constexpr long JOYREG_DEBUG       = 0;
constexpr long POTREG_DEBUG       = 0;
constexpr long VID_DEBUG          = 0;
constexpr long PRT_DEBUG          = 0;
constexpr long SER_DEBUG          = 0;
constexpr long POT_DEBUG          = 0;
constexpr long HOLD_MOUSE_L       = 0;
constexpr long HOLD_MOUSE_M       = 0;
constexpr long HOLD_MOUSE_R       = 0;

// Expansion boards
constexpr long ZOR_DEBUG          = 0;
constexpr long ACF_DEBUG          = 0;
constexpr long FAS_DEBUG          = 0;
constexpr long HDR_DEBUG          = 0;
constexpr long DBD_DEBUG          = 0;

// Media types
constexpr long ADF_DEBUG          = 0;
constexpr long HDF_DEBUG          = 0;
constexpr long DMS_DEBUG          = 0;
constexpr long IMG_DEBUG          = 0;

// Real-time clock
constexpr long RTC_DEBUG          = 0;

// Keyboard
constexpr long KBD_DEBUG          = 0;
constexpr long KEY_DEBUG          = 0;

// Misc
constexpr long RSH_DEBUG          = 0;
constexpr long REC_DEBUG          = 0;
constexpr long SCK_DEBUG          = 0;
constexpr long SRV_DEBUG          = 0;
constexpr long GDB_DEBUG          = 0;


//
// Forced error conditions
//

constexpr long FORCE_LAUNCH_ERROR           = 0;
constexpr long FORCE_ROM_MISSING            = 0;
constexpr long FORCE_CHIP_RAM_MISSING       = 0;
constexpr long FORCE_AROS_NO_EXTROM         = 0;
constexpr long FORCE_AROS_RAM_LIMIT         = 0;
constexpr long FORCE_CHIP_RAM_LIMIT         = 0;
constexpr long FORCE_SNAP_TOO_OLD           = 0;
constexpr long FORCE_SNAP_TOO_NEW           = 0;
constexpr long FORCE_SNAP_IS_BETA           = 0;
constexpr long FORCE_SNAP_CORRUPTED         = 0;
constexpr long FORCE_DISK_INVALID_LAYOUT    = 0;
constexpr long FORCE_DISK_MODIFIED          = 0;
constexpr long FORCE_HDR_TOO_LARGE          = 0;
constexpr long FORCE_HDR_UNSUPPORTED_C      = 0;
constexpr long FORCE_HDR_UNSUPPORTED_H      = 0;
constexpr long FORCE_HDR_UNSUPPORTED_S      = 0;
constexpr long FORCE_HDR_UNSUPPORTED_B      = 0;
constexpr long FORCE_HDR_UNKNOWN_GEOMETRY   = 0;
constexpr long FORCE_HDR_MODIFIED           = 0;
constexpr long FORCE_FS_WRONG_BSIZE         = 0;
constexpr long FORCE_FS_WRONG_CAPACITY      = 0;
constexpr long FORCE_FS_WRONG_DOS_TYPE      = 0;
constexpr long FORCE_DMS_CANT_CREATE        = 0;

}

#include <assert.h>
