// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

namespace utl {

//
// Debug settings
//

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


//
// Logging channels
//

/* For each debug setting, the emulator registers a corresponding log channel
 * with the Loggable interface. Log channels initialized with a debug level
 * of 0 are disabled by default.
 *
 * In debug builds, log channels can be enabled, disabled, or reconfigured
 * dynamically via the Loggable interface. In release builds, the initial
 * configuration is fixed and cannot be modified, allowing the compiler
 * to apply performance optimization.
 */

// General
extern long CH_XFILES;
extern long CH_CNF_DEBUG;
extern long CH_OBJ_DEBUG;
extern long CH_DEF_DEBUG;
extern long CH_MIMIC_UAE;

// Emulator
extern long CH_RUN_DEBUG;
extern long CH_TIM_DEBUG;
extern long CH_WARP_DEBUG;
extern long CH_CMD_DEBUG;
extern long CH_MSG_DEBUG;
extern long CH_SNP_DEBUG;

// Run ahead
extern long CH_RUA_DEBUG;
extern long CH_RUA_CHECKSUM;
extern long CH_RUA_ON_STEROIDS;

// CPU
extern long CH_CPU_DEBUG;

// Memory access
extern long CH_OCSREG_DEBUG;
extern long CH_ECSREG_DEBUG;
extern long CH_INVREG_DEBUG;
extern long CH_MEM_DEBUG;

// Agnus
extern long CH_DMA_DEBUG;
extern long CH_DDF_DEBUG;
extern long CH_SEQ_DEBUG;
extern long CH_SEQ_ON_STEROIDS;
extern long CH_NTSC_DEBUG;

// Copper
extern long CH_COP_CHECKSUM;
extern long CH_COPREG_DEBUG;
extern long CH_COP_DEBUG;

// Blitter
extern long CH_BLT_CHECKSUM;
extern long CH_BLTREG_DEBUG;
extern long CH_BLT_REG_GUARD;
extern long CH_BLT_MEM_GUARD;
extern long CH_BLT_DEBUG;
extern long CH_BLTTIM_DEBUG;
extern long CH_SLOW_BLT_DEBUG;

// Denise
extern long CH_BPLREG_DEBUG;
extern long CH_BPLDAT_DEBUG;
extern long CH_BPLMOD_DEBUG;
extern long CH_SPRREG_DEBUG;
extern long CH_COLREG_DEBUG;
extern long CH_CLXREG_DEBUG;
extern long CH_BPL_ON_STEROIDS;
extern long CH_DIW_DEBUG;
extern long CH_SPR_DEBUG;
extern long CH_CLX_DEBUG;
extern long CH_BORDER_DEBUG;
extern long CH_LINE_DEBUG;
extern long CH_DENISE_ON_STEROIDS;

// Paula
extern long CH_INTREG_DEBUG;
extern long CH_INT_DEBUG;

// CIAs
extern long CH_CIAREG_DEBUG;
extern long CH_CIASER_DEBUG;
extern long CH_CIA_DEBUG;
extern long CH_TOD_DEBUG;

// Floppy Drives
extern long CH_ALIGN_HEAD;
extern long CH_DSK_CHECKSUM;
extern long CH_DSKREG_DEBUG;
extern long CH_DSK_DEBUG;
extern long CH_MFM_DEBUG;
extern long CH_FS_DEBUG;

// Hard Drives
extern long CH_HDR_ACCEPT_ALL;
extern long CH_HDR_FS_LOAD_ALL;
extern long CH_WT_DEBUG;

// Audio
extern long CH_AUDREG_DEBUG;
extern long CH_AUD_DEBUG;
extern long CH_AUDBUF_DEBUG;
extern long CH_AUDVOL_DEBUG;
extern long CH_DISABLE_AUDIRQ;

// Ports
extern long CH_POSREG_DEBUG;
extern long CH_JOYREG_DEBUG;
extern long CH_POTREG_DEBUG;
extern long CH_VID_DEBUG;
extern long CH_PRT_DEBUG;
extern long CH_SER_DEBUG;
extern long CH_POT_DEBUG;
extern long CH_HOLD_MOUSE_L;
extern long CH_HOLD_MOUSE_M;
extern long CH_HOLD_MOUSE_R;

// Expansion boards
extern long CH_ZOR_DEBUG;
extern long CH_ACF_DEBUG;
extern long CH_FAS_DEBUG;
extern long CH_HDR_DEBUG;
extern long CH_DBD_DEBUG;

// Media types
extern long CH_ADF_DEBUG;
extern long CH_HDF_DEBUG;
extern long CH_DMS_DEBUG;
extern long CH_IMG_DEBUG;

// Real-time clock
extern long CH_RTC_DEBUG;

// Keyboard
extern long CH_KBD_DEBUG;
extern long CH_KEY_DEBUG;

// Misc
extern long CH_RSH_DEBUG;
extern long CH_REC_DEBUG;
extern long CH_SCK_DEBUG;
extern long CH_SRV_DEBUG;
extern long CH_GDB_DEBUG;

}


//
// Experimental
//


//
// Wrapper macros (TODO: cleanup)
//

#define CONCAT(a,b) a##b
#define LOG_CHANNEL(a) CONCAT(CH_,a)

#define logmsg(format, ...) { \
log(1, LogLevel::LV_NOTICE, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); }

#define warn(format, ...) { \
log(1, LogLevel::LV_WARNING, std::source_location::current(), "WARNING: " format __VA_OPT__(,) __VA_ARGS__); }

#define fatal(format, ...) { \
log(1, LogLevel::LV_EMERGENCY, std::source_location::current(), "FATAL: " format __VA_OPT__(,) __VA_ARGS__); assert(false); exit(1); }

#define xfiles(format, ...) { \
log(CH_XFILES, LogLevel::LV_NOTICE, std::source_location::current(), "XFILES: " format __VA_OPT__(,) __VA_ARGS__); }

#ifdef NDEBUG

#define debug(channel, format, ...) \
do { if constexpr (channel) { \
log(LOG_CHANNEL(channel), LogLevel::LV_INFO, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

#define logtrace(channel, format, ...) \
do { if constexpr (channel) { \
log(LOG_CHANNEL(channel), LogLevel::LV_DEBUG, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__); \
}} while (0);

#else

#define debug(channel, format, ...) \
log(LOG_CHANNEL(channel), LogLevel::LV_INFO, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__);

#define logtrace(channel, format, ...) \
log(LOG_CHANNEL(channel), LogLevel::LV_DEBUG, std::source_location::current(), format __VA_OPT__(,) __VA_ARGS__);

#endif
