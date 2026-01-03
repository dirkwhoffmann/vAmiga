// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

namespace utl::debug {

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

}

//
// Forced error conditions
//

namespace utl::force {

constexpr long LAUNCH_ERROR           = 0;
constexpr long ROM_MISSING            = 0;
constexpr long CHIP_RAM_MISSING       = 0;
constexpr long AROS_NO_EXTROM         = 0;
constexpr long AROS_RAM_LIMIT         = 0;
constexpr long CHIP_RAM_LIMIT         = 0;
constexpr long SNAP_TOO_OLD           = 0;
constexpr long SNAP_TOO_NEW           = 0;
constexpr long SNAP_IS_BETA           = 0;
constexpr long SNAP_CORRUPTED         = 0;
constexpr long DISK_INVALID_LAYOUT    = 0;
constexpr long DISK_MODIFIED          = 0;
constexpr long HDR_TOO_LARGE          = 0;
constexpr long HDR_UNSUPPORTED_C      = 0;
constexpr long HDR_UNSUPPORTED_H      = 0;
constexpr long HDR_UNSUPPORTED_S      = 0;
constexpr long HDR_UNSUPPORTED_B      = 0;
constexpr long HDR_UNKNOWN_GEOMETRY   = 0;
constexpr long HDR_MODIFIED           = 0;
constexpr long FS_WRONG_BSIZE         = 0;
constexpr long FS_WRONG_CAPACITY      = 0;
constexpr long FS_WRONG_DOS_TYPE      = 0;
constexpr long DMS_CANT_CREATE        = 0;

}

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

namespace utl::channel {

// Default IO channels

extern long DEVNULL;
extern long STDERR;

// General
extern long XFILES;
extern long CNF_DEBUG;
extern long OBJ_DEBUG;
extern long DEF_DEBUG;
extern long MIMIC_UAE;

// Emulator
extern long RUN_DEBUG;
extern long TIM_DEBUG;
extern long WARP_DEBUG;
extern long CMD_DEBUG;
extern long MSG_DEBUG;
extern long SNP_DEBUG;

// Run ahead
extern long RUA_DEBUG;
extern long RUA_CHECKSUM;
extern long RUA_ON_STEROIDS;

// CPU
extern long CPU_DEBUG;

// Memory access
extern long OCSREG_DEBUG;
extern long ECSREG_DEBUG;
extern long INVREG_DEBUG;
extern long MEM_DEBUG;

// Agnus
extern long DMA_DEBUG;
extern long DDF_DEBUG;
extern long SEQ_DEBUG;
extern long SEQ_ON_STEROIDS;
extern long NTSC_DEBUG;

// Copper
extern long COP_CHECKSUM;
extern long COPREG_DEBUG;
extern long COP_DEBUG;

// Blitter
extern long BLT_CHECKSUM;
extern long BLTREG_DEBUG;
extern long BLT_REG_GUARD;
extern long BLT_MEM_GUARD;
extern long BLT_DEBUG;
extern long BLTTIM_DEBUG;
extern long SLOW_BLT_DEBUG;

// Denise
extern long BPLREG_DEBUG;
extern long BPLDAT_DEBUG;
extern long BPLMOD_DEBUG;
extern long SPRREG_DEBUG;
extern long COLREG_DEBUG;
extern long CLXREG_DEBUG;
extern long BPL_ON_STEROIDS;
extern long DIW_DEBUG;
extern long SPR_DEBUG;
extern long CLX_DEBUG;
extern long BORDER_DEBUG;
extern long LINE_DEBUG;
extern long DENISE_ON_STEROIDS;

// Paula
extern long INTREG_DEBUG;
extern long INT_DEBUG;

// CIAs
extern long CIAREG_DEBUG;
extern long CIASER_DEBUG;
extern long CIA_DEBUG;
extern long TOD_DEBUG;

// Floppy Drives
extern long ALIGN_HEAD;
extern long DSK_CHECKSUM;
extern long DSKREG_DEBUG;
extern long DSK_DEBUG;
extern long MFM_DEBUG;
extern long FS_DEBUG;

// Hard Drives
extern long HDR_ACCEPT_ALL;
extern long HDR_FS_LOAD_ALL;
extern long WT_DEBUG;

// Audio
extern long AUDREG_DEBUG;
extern long AUD_DEBUG;
extern long AUDBUF_DEBUG;
extern long AUDVOL_DEBUG;
extern long DISABLE_AUDIRQ;

// Ports
extern long POSREG_DEBUG;
extern long JOYREG_DEBUG;
extern long POTREG_DEBUG;
extern long VID_DEBUG;
extern long PRT_DEBUG;
extern long SER_DEBUG;
extern long POT_DEBUG;
extern long HOLD_MOUSE_L;
extern long HOLD_MOUSE_M;
extern long HOLD_MOUSE_R;

// Expansion boards
extern long ZOR_DEBUG;
extern long ACF_DEBUG;
extern long FAS_DEBUG;
extern long HDR_DEBUG;
extern long DBD_DEBUG;

// Media types
extern long ADF_DEBUG;
extern long HDF_DEBUG;
extern long DMS_DEBUG;
extern long IMG_DEBUG;

// Real-time clock
extern long RTC_DEBUG;

// Keyboard
extern long KBD_DEBUG;
extern long KEY_DEBUG;

// Misc
extern long RSH_DEBUG;
extern long REC_DEBUG;
extern long SCK_DEBUG;
extern long SRV_DEBUG;
extern long GDB_DEBUG;

}

//
// Logging macros
//

#if NDEBUG

#define logGeneric(key, level, format, ...) \
    do { \
        if constexpr (debug::key) \
            log(channel::key, level, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

#else

#define logGeneric(key, level, format, ...)                           \
do { \
    log(channel::key, level, std::source_location::current(), \
        format __VA_OPT__(,) __VA_ARGS__); \
} while (0)

#endif

#define logEmergency(format, ...) \
    logGeneric(STDERR, LogLevel::LOG_EMERG, format __VA_OPT__(,) __VA_ARGS__)

#define logAlert(format, ...) \
    logGeneric(STDERR, LogLevel::LOG_ALERT, format __VA_OPT__(,) __VA_ARGS__)

#define logCritical(format, ...) \
    logGeneric(STDERR, LogLevel::LOG_CRIT, format __VA_OPT__(,) __VA_ARGS__)

#define logError(format, ...) \
    logGeneric(STDERR, LogLevel::LOG_ERR, format __VA_OPT__(,) __VA_ARGS__)

#define logWarning(format, ...) \
    logGeneric(STDERR, LogLevel::LOG_WARNING, format __VA_OPT__(,) __VA_ARGS__)

#define logNotice(channel, format, ...) \
    logGeneric(channel, LogLevel::LOG_NOTICE, format __VA_OPT__(,) __VA_ARGS__)

#define logInfo(channel, format, ...) \
    logGeneric(channel, LogLevel::LOG_INFO, format __VA_OPT__(,) __VA_ARGS__)

#define logDebug(channel, format, ...) \
    logGeneric(channel, LogLevel::LOG_DEBUG, format __VA_OPT__(,) __VA_ARGS__)

//
// Deprecated backward compatibility macros
//

// #define logmsg(format, ...) \
// logGeneric(STDERR, LogLevel::LOG_NOTICE, format __VA_OPT__(,) __VA_ARGS__)

#define warn(format, ...) \
logGeneric(STDERR, LogLevel::LOG_WARNING, format __VA_OPT__(,) __VA_ARGS__)

#define fatal(format, ...) \
logGeneric(STDERR, LogLevel::LOG_EMERG, format __VA_OPT__(,) __VA_ARGS__); \
fatalError

#define xfiles(format, ...) \
logGeneric(XFILES, LogLevel::LOG_INFO, format __VA_OPT__(,) __VA_ARGS__)

#define debug(ch, format, ...) \
logGeneric(ch, LogLevel::LOG_INFO, format __VA_OPT__(,) __VA_ARGS__)
