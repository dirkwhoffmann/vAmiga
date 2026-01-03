// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include <optional>

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
// typedef const long debugflag;
#else
static constexpr bool releaseBuild = 0;
static constexpr bool debugBuild = 1;
// typedef long debugflag;
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

/* Each debug option is represented as an optional Syslog severity level.
 * An empty optional denotes a disabled option. For every debug option,
 * the emulator registers a corresponding log channel with the Loggable
 * interface. Log channels initialized with an empty optional are disabled.
 *
 * If a debug level is set to, for example, 4, all messages with a Syslog
 * severity of 4 or lower are emitted. Setting the level to 7 (the maximum
 * Syslog severity) enables all messages.
 *
 * In debug builds, log channels can be enabled, disabled, or reconfigured
 * dynamically via the Loggable interface.
 */

namespace utl {

// General
constexpr std::optional<long> XFILES             = {};      extern long CH_XFILES;
constexpr std::optional<long> CNF_DEBUG          = {};      extern long CH_CNF_DEBUG;
constexpr std::optional<long> OBJ_DEBUG          = {};      extern long CH_OBJ_DEBUG;
constexpr std::optional<long> DEF_DEBUG          = {};      extern long CH_DEF_DEBUG;
constexpr std::optional<long> MIMIC_UAE          = {};      extern long CH_MIMIC_UAE;

// Emulator
constexpr std::optional<long> RUN_DEBUG          = {};      extern long CH_RUN_DEBUG;
constexpr std::optional<long> TIM_DEBUG          = {};      extern long CH_TIM_DEBUG;
constexpr std::optional<long> WARP_DEBUG         = {};      extern long CH_WARP_DEBUG;
constexpr std::optional<long> CMD_DEBUG          = {};      extern long CH_CMD_DEBUG;
constexpr std::optional<long> MSG_DEBUG          = {};      extern long CH_MSG_DEBUG;
constexpr std::optional<long> SNP_DEBUG          = {};      extern long CH_SNP_DEBUG;

// Run ahead
constexpr std::optional<long> RUA_DEBUG          = {};      extern long CH_RUA_DEBUG;
constexpr std::optional<long> RUA_CHECKSUM       = {};      extern long CH_RUA_CHECKSUM;
constexpr std::optional<long> RUA_ON_STEROIDS    = {};      extern long CH_RUA_ON_STEROIDS;

// CPU
constexpr std::optional<long> CPU_DEBUG          = {};      extern long CH_CPU_DEBUG;

// Memory access
constexpr std::optional<long> OCSREG_DEBUG       = {};      extern long CH_OCSREG_DEBUG;
constexpr std::optional<long> ECSREG_DEBUG       = {};      extern long CH_ECSREG_DEBUG;
constexpr std::optional<long> INVREG_DEBUG       = {};      extern long CH_INVREG_DEBUG;
constexpr std::optional<long> MEM_DEBUG          = {};      extern long CH_MEM_DEBUG;

// Agnus
constexpr std::optional<long> DMA_DEBUG          = {};      extern long CH_DMA_DEBUG;
constexpr std::optional<long> DDF_DEBUG          = {};      extern long CH_DDF_DEBUG;
constexpr std::optional<long> SEQ_DEBUG          = {};      extern long CH_SEQ_DEBUG;
constexpr std::optional<long> SEQ_ON_STEROIDS    = {};      extern long CH_SEQ_ON_STEROIDS;
constexpr std::optional<long> NTSC_DEBUG         = {};      extern long CH_NTSC_DEBUG;

// Copper
constexpr std::optional<long> COP_CHECKSUM       = {};      extern long CH_COP_CHECKSUM;
constexpr std::optional<long> COPREG_DEBUG       = {};      extern long CH_COPREG_DEBUG;
constexpr std::optional<long> COP_DEBUG          = {};      extern long CH_COP_DEBUG;

// Blitter
constexpr std::optional<long> BLT_CHECKSUM       = {};      extern long CH_BLT_CHECKSUM;
constexpr std::optional<long> BLTREG_DEBUG       = {};      extern long CH_BLTREG_DEBUG;
constexpr std::optional<long> BLT_REG_GUARD      = {};      extern long CH_BLT_REG_GUARD;
constexpr std::optional<long> BLT_MEM_GUARD      = {};      extern long CH_BLT_MEM_GUARD;
constexpr std::optional<long> BLT_DEBUG          = {};      extern long CH_BLT_DEBUG;
constexpr std::optional<long> BLTTIM_DEBUG       = {};      extern long CH_BLTTIM_DEBUG;
constexpr std::optional<long> SLOW_BLT_DEBUG     = {};      extern long CH_SLOW_BLT_DEBUG;

// Denise
constexpr std::optional<long> BPLREG_DEBUG       = {};      extern long CH_BPLREG_DEBUG;
constexpr std::optional<long> BPLDAT_DEBUG       = {};      extern long CH_BPLDAT_DEBUG;
constexpr std::optional<long> BPLMOD_DEBUG       = {};      extern long CH_BPLMOD_DEBUG;
constexpr std::optional<long> SPRREG_DEBUG       = {};      extern long CH_SPRREG_DEBUG;
constexpr std::optional<long> COLREG_DEBUG       = {};      extern long CH_COLREG_DEBUG;
constexpr std::optional<long> CLXREG_DEBUG       = {};      extern long CH_CLXREG_DEBUG;
constexpr std::optional<long> BPL_ON_STEROIDS    = {};      extern long CH_BPL_ON_STEROIDS;
constexpr std::optional<long> DIW_DEBUG          = {};      extern long CH_DIW_DEBUG;
constexpr std::optional<long> SPR_DEBUG          = {};      extern long CH_SPR_DEBUG;
constexpr std::optional<long> CLX_DEBUG          = {};      extern long CH_CLX_DEBUG;
constexpr std::optional<long> BORDER_DEBUG       = {};      extern long CH_BORDER_DEBUG;
constexpr std::optional<long> LINE_DEBUG         = {};      extern long CH_LINE_DEBUG;
constexpr std::optional<long> DENISE_ON_STEROIDS = {};      extern long CH_DENISE_ON_STEROIDS;

// Paula
constexpr std::optional<long> INTREG_DEBUG       = {};      extern long CH_INTREG_DEBUG;
constexpr std::optional<long> INT_DEBUG          = {};      extern long CH_INT_DEBUG;

// CIAs
constexpr std::optional<long> CIAREG_DEBUG       = {};      extern long CH_CIAREG_DEBUG;
constexpr std::optional<long> CIASER_DEBUG       = {};      extern long CH_CIASER_DEBUG;
constexpr std::optional<long> CIA_DEBUG          = {};      extern long CH_CIA_DEBUG;
constexpr std::optional<long> TOD_DEBUG          = {};      extern long CH_TOD_DEBUG;

// Floppy Drives
constexpr std::optional<long> ALIGN_HEAD         = {};      extern long CH_ALIGN_HEAD;
constexpr std::optional<long> DSK_CHECKSUM       = {};      extern long CH_DSK_CHECKSUM;
constexpr std::optional<long> DSKREG_DEBUG       = {};      extern long CH_DSKREG_DEBUG;
constexpr std::optional<long> DSK_DEBUG          = {};      extern long CH_DSK_DEBUG;
constexpr std::optional<long> MFM_DEBUG          = {};      extern long CH_MFM_DEBUG;
constexpr std::optional<long> FS_DEBUG           = {};      extern long CH_FS_DEBUG;

// Hard Drives
constexpr std::optional<long> HDR_ACCEPT_ALL     = {};      extern long CH_HDR_ACCEPT_ALL;
constexpr std::optional<long> HDR_FS_LOAD_ALL    = {};      extern long CH_HDR_FS_LOAD_ALL;
constexpr std::optional<long> WT_DEBUG           = {};      extern long CH_WT_DEBUG;

// Audio
constexpr std::optional<long> AUDREG_DEBUG       = {};      extern long CH_AUDREG_DEBUG;
constexpr std::optional<long> AUD_DEBUG          = {};      extern long CH_AUD_DEBUG;
constexpr std::optional<long> AUDBUF_DEBUG       = {};      extern long CH_AUDBUF_DEBUG;
constexpr std::optional<long> AUDVOL_DEBUG       = {};      extern long CH_AUDVOL_DEBUG;
constexpr std::optional<long> DISABLE_AUDIRQ     = {};      extern long CH_DISABLE_AUDIRQ;

// Ports
constexpr std::optional<long> POSREG_DEBUG       = {};      extern long CH_POSREG_DEBUG;
constexpr std::optional<long> JOYREG_DEBUG       = {};      extern long CH_JOYREG_DEBUG;
constexpr std::optional<long> POTREG_DEBUG       = {};      extern long CH_POTREG_DEBUG;
constexpr std::optional<long> VID_DEBUG          = {};      extern long CH_VID_DEBUG;
constexpr std::optional<long> PRT_DEBUG          = {};      extern long CH_PRT_DEBUG;
constexpr std::optional<long> SER_DEBUG          = {};      extern long CH_SER_DEBUG;
constexpr std::optional<long> POT_DEBUG          = {};      extern long CH_POT_DEBUG;
constexpr std::optional<long> HOLD_MOUSE_L       = {};      extern long CH_HOLD_MOUSE_L;
constexpr std::optional<long> HOLD_MOUSE_M       = {};      extern long CH_HOLD_MOUSE_M;
constexpr std::optional<long> HOLD_MOUSE_R       = {};      extern long CH_HOLD_MOUSE_R;

// Expansion boards
constexpr std::optional<long> ZOR_DEBUG          = {};      extern long CH_ZOR_DEBUG;
constexpr std::optional<long> ACF_DEBUG          = {};      extern long CH_ACF_DEBUG;
constexpr std::optional<long> FAS_DEBUG          = {};      extern long CH_FAS_DEBUG;
constexpr std::optional<long> HDR_DEBUG          = {};      extern long CH_HDR_DEBUG;
constexpr std::optional<long> DBD_DEBUG          = {};      extern long CH_DBD_DEBUG;

// Media types
constexpr std::optional<long> ADF_DEBUG          = {};     extern long CH_ADF_DEBUG;
constexpr std::optional<long> HDF_DEBUG          = {};     extern long CH_HDF_DEBUG;
constexpr std::optional<long> DMS_DEBUG          = {};     extern long CH_DMS_DEBUG;
constexpr std::optional<long> IMG_DEBUG          = {};     extern long CH_IMG_DEBUG;

// Real-time clock
constexpr std::optional<long> RTC_DEBUG          = {};     extern long CH_RTC_DEBUG;

// Keyboard
constexpr std::optional<long> KBD_DEBUG          = {};     extern long CH_KBD_DEBUG;
constexpr std::optional<long> KEY_DEBUG          = {};     extern long CH_KEY_DEBUG;

// Misc
constexpr std::optional<long> RSH_DEBUG          = {};     extern long CH_RSH_DEBUG;
constexpr std::optional<long> REC_DEBUG          = {};     extern long CH_REC_DEBUG;
constexpr std::optional<long> SCK_DEBUG          = {};     extern long CH_SCK_DEBUG;
constexpr std::optional<long> SRV_DEBUG          = {};     extern long CH_SRV_DEBUG;
constexpr std::optional<long> GDB_DEBUG          = {};     extern long CH_GDB_DEBUG;


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
