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
 * becomes 'if constexpr', so the compiler removes each guarded log call (or
 * action) entirely when its flag is off.
 *
 * In debug builds, CONSTEXPR expands to nothing. The same flags become
 * ordinary (inline) variables that can be switched on and off at runtime,
 * and 'if CONSTEXPR' becomes a plain 'if', evaluated every time.
 *
 * Logging flags are typed 'LogLevel': their value doubles as the severity
 * a logme() call is issued with. LOG_NONE disables the call, any other
 * LogLevel enables it at that severity. E.g., 'LOG_CPU = LogLevel::LOG_DEBUG'
 * enables every logme() call gated by LOG_CPU at severity LOG_DEBUG. All
 * logging flags are prefixed 'LOG_' to set them apart from action flags.
 *
 * Action flags are typed 'bool' - see the comment above their section.
 */

//
// Logging macro
//

#define logme(key, format, ...) \
    do { \
        if CONSTEXPR (debug::key != LogLevel::LOG_NONE) \
            log(debug::key, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)


namespace utl::debug {

//
// Fixed severities (always active, never LOG_NONE)
//

inline constexpr LogLevel LV_EMERGENCY = LogLevel::LOG_EMERG;
inline constexpr LogLevel LV_ALERT     = LogLevel::LOG_ALERT;
inline constexpr LogLevel LV_CRITICAL  = LogLevel::LOG_CRIT;
inline constexpr LogLevel LV_ERROR     = LogLevel::LOG_ERR;
inline constexpr LogLevel LV_WARNING   = LogLevel::LOG_WARNING;
inline constexpr LogLevel LV_NOTICE    = LogLevel::LOG_NOTICE;
inline constexpr LogLevel LV_INFO      = LogLevel::LOG_INFO;
inline constexpr LogLevel LV_DEBUG     = LogLevel::LOG_DEBUG;

// Always-off placeholder, used to permanently silence a log call
inline constexpr LogLevel LOG_NULLDEV = LogLevel::LOG_NONE;

/* Debug flags come in two kinds:
 *
 * - Logging flags gate a logme() call and nothing else. Disabling one
 *   removes the call (release builds) or simply keeps it silent (debug
 *   builds); the emulator behaves identically either way.
 *
 * - Action flags enable extra debug behavior with a real side effect
 *   (an integrity check, a redundant computation compared against the
 *   fast path, a memory guard, forcing a specific code path, ...). Some
 *   of these also log a message as part of that action, but disabling
 *   the flag changes what the emulator *does*, not just what it prints.
 */

//
// Flags that control logging
//

// General
inline CONSTEXPR LogLevel LOG_XFILES  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CNF     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_OBJ     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DEF     = LogLevel::LOG_NONE;

// Emulator
inline CONSTEXPR LogLevel LOG_RUN     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_TIM     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_WARP    = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CMD     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_MSG     = LogLevel::LOG_NONE;

// Run ahead
inline CONSTEXPR LogLevel LOG_RUA     = LogLevel::LOG_NONE;

// CPU
inline CONSTEXPR LogLevel LOG_CPU     = LogLevel::LOG_NONE;

// Memory access
inline CONSTEXPR LogLevel LOG_OCSREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_ECSREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_INVREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_MEM     = LogLevel::LOG_NONE;

// Agnus
inline CONSTEXPR LogLevel LOG_DMA     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DDF     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SEQ     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_NTSC    = LogLevel::LOG_NONE;

// Copper
inline CONSTEXPR LogLevel LOG_COPREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_COP     = LogLevel::LOG_NONE;

// Blitter
inline CONSTEXPR LogLevel LOG_BLTREG        = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_BLT_REG_GUARD = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_BLT           = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_BLTTIM        = LogLevel::LOG_NONE;

// Denise
inline CONSTEXPR LogLevel LOG_BPLREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_BPLDAT  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_BPLMOD  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SPRREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_COLREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CLXREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DIW     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SPR     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CLX     = LogLevel::LOG_NONE;

// Paula
inline CONSTEXPR LogLevel LOG_INTREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_INT     = LogLevel::LOG_NONE;

// CIAs
inline CONSTEXPR LogLevel LOG_CIAREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CIASER  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_CIA     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_TOD     = LogLevel::LOG_NONE;

// Floppy Drives
inline CONSTEXPR LogLevel LOG_DSKREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DSK     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_MFM     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_FS      = LogLevel::LOG_NONE;

// Hard Drives
inline CONSTEXPR LogLevel LOG_WT      = LogLevel::LOG_NONE;

// Audio
inline CONSTEXPR LogLevel LOG_AUDREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_AUD     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_AUDBUF  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_AUDVOL  = LogLevel::LOG_NONE;

// Ports
inline CONSTEXPR LogLevel LOG_POSREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_JOYREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_POTREG  = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_VID     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_PRT     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SER     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_POT     = LogLevel::LOG_NONE;

// Expansion boards
inline CONSTEXPR LogLevel LOG_ZOR     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_ACF     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_FAS     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_HDR     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DBD     = LogLevel::LOG_NONE;

// Image files
inline CONSTEXPR LogLevel LOG_HDF     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_DMS     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_IMG     = LogLevel::LOG_NONE;

// Real-time clock
inline CONSTEXPR LogLevel LOG_RTC     = LogLevel::LOG_NONE;

// Keyboard
inline CONSTEXPR LogLevel LOG_KBD     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_KEY     = LogLevel::LOG_NONE;

// Misc
inline CONSTEXPR LogLevel LOG_RSH     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_REC     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SCK     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_SRV     = LogLevel::LOG_NONE;
inline CONSTEXPR LogLevel LOG_GDB     = LogLevel::LOG_NONE;

/* Action flags are plain bools: unlike logging flags, "how loud" never
 * applies to them, only "on or off". The exception is LINE_DEBUG, which
 * holds a scanline number rather than an on/off switch (-1 = disabled).
 *
 * A few of these also print a message reporting on the action they
 * triggered (e.g. a computed checksum). Rather than force that message
 * through a severity encoded in a now-boolean flag, those call sites
 * gate a logme() call with a fixed LV_* severity on the action flag
 * directly - see e.g. Copper::eofHandler() or Blitter::beginLineBlit().
 */

//
// Flags that enable a debug action
//

// General
inline CONSTEXPR bool MIMIC_UAE          = false;

// Emulator
inline CONSTEXPR bool SNP_DEBUG          = false;

// Run ahead
inline CONSTEXPR bool RUA_CHECKSUM       = false;
inline CONSTEXPR bool RUA_ON_STEROIDS    = false;

// Agnus
inline CONSTEXPR bool SEQ_ON_STEROIDS    = false;

// Copper
inline CONSTEXPR bool COP_CHECKSUM       = false;

// Blitter
inline CONSTEXPR bool BLT_CHECKSUM       = false;
inline CONSTEXPR bool BLT_MEM_GUARD      = false;
inline CONSTEXPR bool BLT_MINTERM_CHECK  = false;
inline CONSTEXPR bool SLOW_BLT_DEBUG     = false;

// Denise
inline CONSTEXPR bool BPL_ON_STEROIDS    = false;
inline CONSTEXPR bool BORDER_DEBUG       = false;
inline CONSTEXPR long LINE_DEBUG         = -1;
inline CONSTEXPR bool DENISE_ON_STEROIDS = false;

// Floppy Drives
inline CONSTEXPR bool ALIGN_HEAD         = false;
inline CONSTEXPR bool DSK_CHECKSUM       = false;
inline CONSTEXPR bool FS_VERIFY          = false;

// Hard Drives
inline CONSTEXPR bool HDR_ACCEPT_ALL     = false;
inline CONSTEXPR bool HDR_FS_LOAD_ALL    = false;

// Audio
inline CONSTEXPR bool DISABLE_AUDIRQ     = false;

// Ports
inline CONSTEXPR bool HOLD_MOUSE_L       = false;
inline CONSTEXPR bool HOLD_MOUSE_M       = false;
inline CONSTEXPR bool HOLD_MOUSE_R       = false;

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
    logme(LOG_XFILES, format __VA_OPT__(,) __VA_ARGS__)
