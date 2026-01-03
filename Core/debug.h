// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

/* Debug settings
 *
 * For each debug setting, the emulator registers a corresponding log channel
 * with the Loggable interface. Log channels initialized with a debug level
 * of 0 are disabled by default.
 *
 * In debug builds, log channels can be enabled, disabled, or reconfigured
 * dynamically via the Loggable interface. In release builds, the initial
 * configuration is fixed and cannot be modified, allowing the compiler
 * to apply performance optimization.
 */

namespace utl {

// General
constexpr long XFILES             = 0;      extern long CH_XFILES;
constexpr long CNF_DEBUG          = 0;      extern long CH_CNF_DEBUG;
constexpr long OBJ_DEBUG          = 0;      extern long CH_OBJ_DEBUG;
constexpr long DEF_DEBUG          = 0;      extern long CH_DEF_DEBUG;
constexpr long MIMIC_UAE          = 0;      extern long CH_MIMIC_UAE;

// Emulator
constexpr long RUN_DEBUG          = 0;      extern long CH_RUN_DEBUG;
constexpr long TIM_DEBUG          = 0;      extern long CH_TIM_DEBUG;
constexpr long WARP_DEBUG         = 0;      extern long CH_WARP_DEBUG;
constexpr long CMD_DEBUG          = 0;      extern long CH_CMD_DEBUG;
constexpr long MSG_DEBUG          = 0;      extern long CH_MSG_DEBUG;
constexpr long SNP_DEBUG          = 0;      extern long CH_SNP_DEBUG;

// Run ahead
constexpr long RUA_DEBUG          = 0;      extern long CH_RUA_DEBUG;
constexpr long RUA_CHECKSUM       = 0;      extern long CH_RUA_CHECKSUM;
constexpr long RUA_ON_STEROIDS    = 0;      extern long CH_RUA_ON_STEROIDS;

// CPU
constexpr long CPU_DEBUG          = 0;      extern long CH_CPU_DEBUG;

// Memory access
constexpr long OCSREG_DEBUG       = 0;      extern long CH_OCSREG_DEBUG;
constexpr long ECSREG_DEBUG       = 0;      extern long CH_ECSREG_DEBUG;
constexpr long INVREG_DEBUG       = 0;      extern long CH_INVREG_DEBUG;
constexpr long MEM_DEBUG          = 0;      extern long CH_MEM_DEBUG;

// Agnus
constexpr long DMA_DEBUG          = 0;      extern long CH_DMA_DEBUG;
constexpr long DDF_DEBUG          = 0;      extern long CH_DDF_DEBUG;
constexpr long SEQ_DEBUG          = 0;      extern long CH_SEQ_DEBUG;
constexpr long SEQ_ON_STEROIDS    = 0;      extern long CH_SEQ_ON_STEROIDS;
constexpr long NTSC_DEBUG         = 0;      extern long CH_NTSC_DEBUG;

// Copper
constexpr long COP_CHECKSUM       = 0;      extern long CH_COP_CHECKSUM;
constexpr long COPREG_DEBUG       = 0;      extern long CH_COPREG_DEBUG;
constexpr long COP_DEBUG          = 0;      extern long CH_COP_DEBUG;

// Blitter
constexpr long BLT_CHECKSUM       = 0;      extern long CH_BLT_CHECKSUM;
constexpr long BLTREG_DEBUG       = 0;      extern long CH_BLTREG_DEBUG;
constexpr long BLT_REG_GUARD      = 0;      extern long CH_BLT_REG_GUARD;
constexpr long BLT_MEM_GUARD      = 0;      extern long CH_BLT_MEM_GUARD;
constexpr long BLT_DEBUG          = 0;      extern long CH_BLT_DEBUG;
constexpr long BLTTIM_DEBUG       = 0;      extern long CH_BLTTIM_DEBUG;
constexpr long SLOW_BLT_DEBUG     = 0;      extern long CH_SLOW_BLT_DEBUG;

// Denise
constexpr long BPLREG_DEBUG       = 0;      extern long CH_BPLREG_DEBUG;
constexpr long BPLDAT_DEBUG       = 0;      extern long CH_BPLDAT_DEBUG;
constexpr long BPLMOD_DEBUG       = 0;      extern long CH_BPLMOD_DEBUG;
constexpr long SPRREG_DEBUG       = 0;      extern long CH_SPRREG_DEBUG;
constexpr long COLREG_DEBUG       = 0;      extern long CH_COLREG_DEBUG;
constexpr long CLXREG_DEBUG       = 0;      extern long CH_CLXREG_DEBUG;
constexpr long BPL_ON_STEROIDS    = 0;      extern long CH_BPL_ON_STEROIDS;
constexpr long DIW_DEBUG          = 0;      extern long CH_DIW_DEBUG;
constexpr long SPR_DEBUG          = 0;      extern long CH_SPR_DEBUG;
constexpr long CLX_DEBUG          = 0;      extern long CH_CLX_DEBUG;
constexpr long BORDER_DEBUG       = 0;      extern long CH_BORDER_DEBUG;
constexpr long LINE_DEBUG         = 0;      extern long CH_LINE_DEBUG;
constexpr long DENISE_ON_STEROIDS = 0;      extern long CH_DENISE_ON_STEROIDS;

// Paula
constexpr long INTREG_DEBUG       = 0;      extern long CH_INTREG_DEBUG;
constexpr long INT_DEBUG          = 0;      extern long CH_INT_DEBUG;

// CIAs
constexpr long CIAREG_DEBUG       = 0;      extern long CH_CIAREG_DEBUG;
constexpr long CIASER_DEBUG       = 0;      extern long CH_CIASER_DEBUG;
constexpr long CIA_DEBUG          = 0;      extern long CH_CIA_DEBUG;
constexpr long TOD_DEBUG          = 0;      extern long CH_TOD_DEBUG;

// Floppy Drives
constexpr long ALIGN_HEAD         = 0;      extern long CH_ALIGN_HEAD;
constexpr long DSK_CHECKSUM       = 0;      extern long CH_DSK_CHECKSUM;
constexpr long DSKREG_DEBUG       = 0;      extern long CH_DSKREG_DEBUG;
constexpr long DSK_DEBUG          = 0;      extern long CH_DSK_DEBUG;
constexpr long MFM_DEBUG          = 0;      extern long CH_MFM_DEBUG;
constexpr long FS_DEBUG           = 0;      extern long CH_FS_DEBUG;

// Hard Drives
constexpr long HDR_ACCEPT_ALL     = 0;      extern long CH_HDR_ACCEPT_ALL;
constexpr long HDR_FS_LOAD_ALL    = 0;      extern long CH_HDR_FS_LOAD_ALL;
constexpr long WT_DEBUG           = 0;      extern long CH_WT_DEBUG;

// Audio
constexpr long AUDREG_DEBUG       = 0;      extern long CH_AUDREG_DEBUG;
constexpr long AUD_DEBUG          = 0;      extern long CH_AUD_DEBUG;
constexpr long AUDBUF_DEBUG       = 0;      extern long CH_AUDBUF_DEBUG;
constexpr long AUDVOL_DEBUG       = 0;      extern long CH_AUDVOL_DEBUG;
constexpr long DISABLE_AUDIRQ     = 0;      extern long CH_DISABLE_AUDIRQ;

// Ports
constexpr long POSREG_DEBUG       = 0;      extern long CH_POSREG_DEBUG;
constexpr long JOYREG_DEBUG       = 0;      extern long CH_JOYREG_DEBUG;
constexpr long POTREG_DEBUG       = 0;      extern long CH_POTREG_DEBUG;
constexpr long VID_DEBUG          = 0;      extern long CH_VID_DEBUG;
constexpr long PRT_DEBUG          = 0;      extern long CH_PRT_DEBUG;
constexpr long SER_DEBUG          = 0;      extern long CH_SER_DEBUG;
constexpr long POT_DEBUG          = 0;      extern long CH_POT_DEBUG;
constexpr long HOLD_MOUSE_L       = 0;      extern long CH_HOLD_MOUSE_L;
constexpr long HOLD_MOUSE_M       = 0;      extern long CH_HOLD_MOUSE_M;
constexpr long HOLD_MOUSE_R       = 0;      extern long CH_HOLD_MOUSE_R;

// Expansion boards
constexpr long ZOR_DEBUG          = 0;      extern long CH_ZOR_DEBUG;
constexpr long ACF_DEBUG          = 0;      extern long CH_ACF_DEBUG;
constexpr long FAS_DEBUG          = 0;      extern long CH_FAS_DEBUG;
constexpr long HDR_DEBUG          = 0;      extern long CH_HDR_DEBUG;
constexpr long DBD_DEBUG          = 0;      extern long CH_DBD_DEBUG;

// Media types
constexpr long ADF_DEBUG          = 0;     extern long CH_ADF_DEBUG;
constexpr long HDF_DEBUG          = 0;     extern long CH_HDF_DEBUG;
constexpr long DMS_DEBUG          = 0;     extern long CH_DMS_DEBUG;
constexpr long IMG_DEBUG          = 0;     extern long CH_IMG_DEBUG;

// Real-time clock
constexpr long RTC_DEBUG          = 0;     extern long CH_RTC_DEBUG;

// Keyboard
constexpr long KBD_DEBUG          = 0;     extern long CH_KBD_DEBUG;
constexpr long KEY_DEBUG          = 0;     extern long CH_KEY_DEBUG;

// Misc
constexpr long RSH_DEBUG          = 0;     extern long CH_RSH_DEBUG;
constexpr long REC_DEBUG          = 0;     extern long CH_REC_DEBUG;
constexpr long SCK_DEBUG          = 0;     extern long CH_SCK_DEBUG;
constexpr long SRV_DEBUG          = 0;     extern long CH_SRV_DEBUG;
constexpr long GDB_DEBUG          = 0;     extern long CH_GDB_DEBUG;


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
