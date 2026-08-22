// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "utl/abilities/Loggable.h"

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
 *
 * A flag's value doubles as the severity a log call is issued with: -1
 * disables the call, 0...7 enables it at the corresponding LogLevel (see
 * LV_EMERGENCY...LV_DEBUG below, mirroring the BSD syslog levels). E.g.,
 * 'CPU_DEBUG = LV_DEBUG' enables every logme() call gated by CPU_DEBUG at
 * severity LV_DEBUG.
 */

//
// Logging macro
//

#define logme(key, format, ...) \
    do { \
        if CONSTEXPR (debug::key != -1) \
            log((LogLevel)debug::key, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)


namespace utl::debug {

//
// Fixed severities (always active, never -1)
//

inline constexpr long LV_EMERGENCY = (long)utl::LogLevel::LOG_EMERG;
inline constexpr long LV_ALERT     = (long)utl::LogLevel::LOG_ALERT;
inline constexpr long LV_CRITICAL  = (long)utl::LogLevel::LOG_CRIT;
inline constexpr long LV_ERROR     = (long)utl::LogLevel::LOG_ERR;
inline constexpr long LV_WARNING   = (long)utl::LogLevel::LOG_WARNING;
inline constexpr long LV_NOTICE    = (long)utl::LogLevel::LOG_NOTICE;
inline constexpr long LV_INFO      = (long)utl::LogLevel::LOG_INFO;
inline constexpr long LV_DEBUG     = (long)utl::LogLevel::LOG_DEBUG;

//
// Debug settings
//

// Always-off placeholder, used to permanently silence a log call
inline constexpr long NULLDEV            = -1;

// General
inline CONSTEXPR long XFILES             = -1;
inline CONSTEXPR long CNF_DEBUG          = -1;
inline CONSTEXPR long OBJ_DEBUG          = -1;
inline CONSTEXPR long DEF_DEBUG          = -1;
inline CONSTEXPR long MIMIC_UAE          = -1;

// Emulator
inline CONSTEXPR long RUN_DEBUG          = -1;
inline CONSTEXPR long TIM_DEBUG          = -1;
inline CONSTEXPR long WARP_DEBUG         = -1;
inline CONSTEXPR long CMD_DEBUG          = -1;
inline CONSTEXPR long MSG_DEBUG          = -1;
inline CONSTEXPR long SNP_DEBUG          = -1;

// Run ahead
inline CONSTEXPR long RUA_DEBUG          = -1;
inline CONSTEXPR long RUA_CHECKSUM       = -1;
inline CONSTEXPR long RUA_ON_STEROIDS    = -1;

// CPU
inline CONSTEXPR long CPU_DEBUG          = -1;

// Memory access
inline CONSTEXPR long OCSREG_DEBUG       = -1;
inline CONSTEXPR long ECSREG_DEBUG       = -1;
inline CONSTEXPR long INVREG_DEBUG       = -1;
inline CONSTEXPR long MEM_DEBUG          = -1;

// Agnus
inline CONSTEXPR long DMA_DEBUG          = -1;
inline CONSTEXPR long DDF_DEBUG          = -1;
inline CONSTEXPR long SEQ_DEBUG          = -1;
inline CONSTEXPR long SEQ_ON_STEROIDS    = -1;
inline CONSTEXPR long NTSC_DEBUG         = -1;

// Copper
inline CONSTEXPR long COP_CHECKSUM       = -1;
inline CONSTEXPR long COPREG_DEBUG       = -1;
inline CONSTEXPR long COP_DEBUG          = -1;

// Blitter
inline CONSTEXPR long BLT_CHECKSUM       = -1;
inline CONSTEXPR long BLTREG_DEBUG       = -1;
inline CONSTEXPR long BLT_REG_GUARD      = -1;
inline CONSTEXPR long BLT_MEM_GUARD      = -1;
inline CONSTEXPR long BLT_DEBUG          = -1;
inline CONSTEXPR long BLTTIM_DEBUG       = -1;
inline CONSTEXPR long SLOW_BLT_DEBUG     = -1;

// Denise
inline CONSTEXPR long BPLREG_DEBUG       = -1;
inline CONSTEXPR long BPLDAT_DEBUG       = -1;
inline CONSTEXPR long BPLMOD_DEBUG       = -1;
inline CONSTEXPR long SPRREG_DEBUG       = -1;
inline CONSTEXPR long COLREG_DEBUG       = -1;
inline CONSTEXPR long CLXREG_DEBUG       = -1;
inline CONSTEXPR long BPL_ON_STEROIDS    = -1;
inline CONSTEXPR long DIW_DEBUG          = -1;
inline CONSTEXPR long SPR_DEBUG          = -1;
inline CONSTEXPR long CLX_DEBUG          = -1;
inline CONSTEXPR long BORDER_DEBUG       = -1;
inline CONSTEXPR long LINE_DEBUG         = -1;
inline CONSTEXPR long DENISE_ON_STEROIDS = -1;

// Paula
inline CONSTEXPR long INTREG_DEBUG       = -1;
inline CONSTEXPR long INT_DEBUG          = -1;

// CIAs
inline CONSTEXPR long CIAREG_DEBUG       = -1;
inline CONSTEXPR long CIASER_DEBUG       = -1;
inline CONSTEXPR long CIA_DEBUG          = -1;
inline CONSTEXPR long TOD_DEBUG          = -1;

// Floppy Drives
inline CONSTEXPR long ALIGN_HEAD         = -1;
inline CONSTEXPR long DSK_CHECKSUM       = -1;
inline CONSTEXPR long DSKREG_DEBUG       = -1;
inline CONSTEXPR long DSK_DEBUG          = -1;
inline CONSTEXPR long MFM_DEBUG          = -1;
inline CONSTEXPR long FS_DEBUG           = -1;

// Hard Drives
inline CONSTEXPR long HDR_ACCEPT_ALL     = -1;
inline CONSTEXPR long HDR_FS_LOAD_ALL    = -1;
inline CONSTEXPR long WT_DEBUG           = -1;

// Audio
inline CONSTEXPR long AUDREG_DEBUG       = -1;
inline CONSTEXPR long AUD_DEBUG          = -1;
inline CONSTEXPR long AUDBUF_DEBUG       = -1;
inline CONSTEXPR long AUDVOL_DEBUG       = -1;
inline CONSTEXPR long DISABLE_AUDIRQ     = -1;

// Ports
inline CONSTEXPR long POSREG_DEBUG       = -1;
inline CONSTEXPR long JOYREG_DEBUG       = -1;
inline CONSTEXPR long POTREG_DEBUG       = -1;
inline CONSTEXPR long VID_DEBUG          = -1;
inline CONSTEXPR long PRT_DEBUG          = -1;
inline CONSTEXPR long SER_DEBUG          = -1;
inline CONSTEXPR long POT_DEBUG          = -1;
inline CONSTEXPR long HOLD_MOUSE_L       = -1;
inline CONSTEXPR long HOLD_MOUSE_M       = -1;
inline CONSTEXPR long HOLD_MOUSE_R       = -1;

// Expansion boards
inline CONSTEXPR long ZOR_DEBUG          = -1;
inline CONSTEXPR long ACF_DEBUG          = -1;
inline CONSTEXPR long FAS_DEBUG          = -1;
inline CONSTEXPR long HDR_DEBUG          = -1;
inline CONSTEXPR long DBD_DEBUG          = -1;

// Image files
inline CONSTEXPR long HDF_DEBUG          = -1;
inline CONSTEXPR long DMS_DEBUG          = -1;
inline CONSTEXPR long IMG_DEBUG          = -1;

// Real-time clock
inline CONSTEXPR long RTC_DEBUG          = -1;

// Keyboard
inline CONSTEXPR long KBD_DEBUG          = -1;
inline CONSTEXPR long KEY_DEBUG          = -1;

// Misc
inline CONSTEXPR long RSH_DEBUG          = -1;
inline CONSTEXPR long REC_DEBUG          = -1;
inline CONSTEXPR long SCK_DEBUG          = -1;
inline CONSTEXPR long SRV_DEBUG          = -1;
inline CONSTEXPR long GDB_DEBUG          = -1;

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
// Convenience wrappers
//

#define fatal(format, ...) \
    do { \
        logme(LV_EMERGENCY, format __VA_OPT__(,) __VA_ARGS__); \
        assert(false); \
        std::terminate(); \
    } while(0)

#define xfiles(format, ...) \
    logme(XFILES, format __VA_OPT__(,) __VA_ARGS__)
