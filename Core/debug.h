// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#ifdef NDEBUG
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

/* In release builds (NDEBUG), CONSTEXPR expands to 'constexpr'. Every debug
 * flag below then becomes a compile-time constant, and 'if CONSTEXPR' below
 * becomes 'if constexpr', so the compiler removes each guarded log call
 * entirely when its flag is off.
 *
 * In debug builds, CONSTEXPR expands to nothing. The same flags become
 * ordinary (inline) variables that can be switched on and off at runtime,
 * and 'if CONSTEXPR' becomes a plain 'if', evaluated every time.
 */

//
// Logging macros
//

#define logRaw(level, format, ...) \
    do { \
        log(1, level, std::source_location::current(), \
            format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

#define logMsg(key, level, format, ...) \
    do { \
        if CONSTEXPR (debug::key) \
            log(1, level, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)


namespace utl::debug {

//
// Debug settings
//

// Always-off placeholder, used to permanently silence a log call
inline constexpr long NULLDEV            = 0;

// General
inline CONSTEXPR long XFILES             = 0;
inline CONSTEXPR long CNF_DEBUG          = 0;
inline CONSTEXPR long OBJ_DEBUG          = 0;
inline CONSTEXPR long DEF_DEBUG          = 0;
inline CONSTEXPR long MIMIC_UAE          = 0;

// Emulator
inline CONSTEXPR long RUN_DEBUG          = 0;
inline CONSTEXPR long TIM_DEBUG          = 0;
inline CONSTEXPR long WARP_DEBUG         = 0;
inline CONSTEXPR long CMD_DEBUG          = 0;
inline CONSTEXPR long MSG_DEBUG          = 0;
inline CONSTEXPR long SNP_DEBUG          = 0;

// Run ahead
inline CONSTEXPR long RUA_DEBUG          = 0;
inline CONSTEXPR long RUA_CHECKSUM       = 0;
inline CONSTEXPR long RUA_ON_STEROIDS    = 0;

// CPU
inline CONSTEXPR long CPU_DEBUG          = 0;

// Memory access
inline CONSTEXPR long OCSREG_DEBUG       = 0;
inline CONSTEXPR long ECSREG_DEBUG       = 0;
inline CONSTEXPR long INVREG_DEBUG       = 0;
inline CONSTEXPR long MEM_DEBUG          = 0;

// Agnus
inline CONSTEXPR long DMA_DEBUG          = 0;
inline CONSTEXPR long DDF_DEBUG          = 0;
inline CONSTEXPR long SEQ_DEBUG          = 0;
inline CONSTEXPR long SEQ_ON_STEROIDS    = 0;
inline CONSTEXPR long NTSC_DEBUG         = 0;

// Copper
inline CONSTEXPR long COP_CHECKSUM       = 0;
inline CONSTEXPR long COPREG_DEBUG       = 0;
inline CONSTEXPR long COP_DEBUG          = 0;

// Blitter
inline CONSTEXPR long BLT_CHECKSUM       = 0;
inline CONSTEXPR long BLTREG_DEBUG       = 0;
inline CONSTEXPR long BLT_REG_GUARD      = 0;
inline CONSTEXPR long BLT_MEM_GUARD      = 0;
inline CONSTEXPR long BLT_DEBUG          = 0;
inline CONSTEXPR long BLTTIM_DEBUG       = 0;
inline CONSTEXPR long SLOW_BLT_DEBUG     = 0;

// Denise
inline CONSTEXPR long BPLREG_DEBUG       = 0;
inline CONSTEXPR long BPLDAT_DEBUG       = 0;
inline CONSTEXPR long BPLMOD_DEBUG       = 0;
inline CONSTEXPR long SPRREG_DEBUG       = 0;
inline CONSTEXPR long COLREG_DEBUG       = 0;
inline CONSTEXPR long CLXREG_DEBUG       = 0;
inline CONSTEXPR long BPL_ON_STEROIDS    = 0;
inline CONSTEXPR long DIW_DEBUG          = 0;
inline CONSTEXPR long SPR_DEBUG          = 0;
inline CONSTEXPR long CLX_DEBUG          = 0;
inline CONSTEXPR long BORDER_DEBUG       = 0;
inline CONSTEXPR long LINE_DEBUG         = 0;
inline CONSTEXPR long DENISE_ON_STEROIDS = 0;

// Paula
inline CONSTEXPR long INTREG_DEBUG       = 0;
inline CONSTEXPR long INT_DEBUG          = 0;

// CIAs
inline CONSTEXPR long CIAREG_DEBUG       = 0;
inline CONSTEXPR long CIASER_DEBUG       = 0;
inline CONSTEXPR long CIA_DEBUG          = 0;
inline CONSTEXPR long TOD_DEBUG          = 0;

// Floppy Drives
inline CONSTEXPR long ALIGN_HEAD         = 0;
inline CONSTEXPR long DSK_CHECKSUM       = 0;
inline CONSTEXPR long DSKREG_DEBUG       = 0;
inline CONSTEXPR long DSK_DEBUG          = 0;
inline CONSTEXPR long MFM_DEBUG          = 0;
inline CONSTEXPR long FS_DEBUG           = 0;

// Hard Drives
inline CONSTEXPR long HDR_ACCEPT_ALL     = 0;
inline CONSTEXPR long HDR_FS_LOAD_ALL    = 0;
inline CONSTEXPR long WT_DEBUG           = 0;

// Audio
inline CONSTEXPR long AUDREG_DEBUG       = 0;
inline CONSTEXPR long AUD_DEBUG          = 0;
inline CONSTEXPR long AUDBUF_DEBUG       = 0;
inline CONSTEXPR long AUDVOL_DEBUG       = 0;
inline CONSTEXPR long DISABLE_AUDIRQ     = 0;

// Ports
inline CONSTEXPR long POSREG_DEBUG       = 0;
inline CONSTEXPR long JOYREG_DEBUG       = 0;
inline CONSTEXPR long POTREG_DEBUG       = 0;
inline CONSTEXPR long VID_DEBUG          = 0;
inline CONSTEXPR long PRT_DEBUG          = 0;
inline CONSTEXPR long SER_DEBUG          = 0;
inline CONSTEXPR long POT_DEBUG          = 0;
inline CONSTEXPR long HOLD_MOUSE_L       = 0;
inline CONSTEXPR long HOLD_MOUSE_M       = 0;
inline CONSTEXPR long HOLD_MOUSE_R       = 0;

// Expansion boards
inline CONSTEXPR long ZOR_DEBUG          = 0;
inline CONSTEXPR long ACF_DEBUG          = 0;
inline CONSTEXPR long FAS_DEBUG          = 0;
inline CONSTEXPR long HDR_DEBUG          = 0;
inline CONSTEXPR long DBD_DEBUG          = 0;

// Image files
inline CONSTEXPR long HDF_DEBUG          = 0;
inline CONSTEXPR long DMS_DEBUG          = 0;
inline CONSTEXPR long IMG_DEBUG          = 0;

// Real-time clock
inline CONSTEXPR long RTC_DEBUG          = 0;

// Keyboard
inline CONSTEXPR long KBD_DEBUG          = 0;
inline CONSTEXPR long KEY_DEBUG          = 0;

// Misc
inline CONSTEXPR long RSH_DEBUG          = 0;
inline CONSTEXPR long REC_DEBUG          = 0;
inline CONSTEXPR long SCK_DEBUG          = 0;
inline CONSTEXPR long SRV_DEBUG          = 0;
inline CONSTEXPR long GDB_DEBUG          = 0;

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
// Wrappers for all syslog levels
//

#define logemergency(format, ...) \
    logRaw(LogLevel::LOG_EMERG, format __VA_OPT__(,) __VA_ARGS__)

#define logalert(format, ...) \
    logRaw(LogLevel::LOG_ALERT, format __VA_OPT__(,) __VA_ARGS__)

#define logcritical(format, ...) \
    logRaw(LogLevel::LOG_CRIT, format __VA_OPT__(,) __VA_ARGS__)

#define logerror(format, ...) \
    logRaw(LogLevel::LOG_ERR, format __VA_OPT__(,) __VA_ARGS__)

#define logwarn(format, ...) \
    logRaw(LogLevel::LOG_WARNING, format __VA_OPT__(,) __VA_ARGS__)

#define lognotice(channel, format, ...) \
    logMsg(channel, LogLevel::LOG_NOTICE, format __VA_OPT__(,) __VA_ARGS__)

#define loginfo(channel, format, ...) \
    logMsg(channel, LogLevel::LOG_INFO, format __VA_OPT__(,) __VA_ARGS__)

#define logdebug(channel, format, ...) \
    logMsg(channel, LogLevel::LOG_DEBUG, format __VA_OPT__(,) __VA_ARGS__)


//
// Convenience wrappers
//

#define fatal(format, ...) \
    do { \
        logemergency(format __VA_OPT__(,) __VA_ARGS__); \
        assert(false); \
        std::terminate(); \
    } while(0)

#define xfiles(format, ...) \
    logMsg(XFILES, LogLevel::LOG_INFO, format __VA_OPT__(,) __VA_ARGS__)
