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

/* How the debug system works:
 *
 * In release builds (NDEBUG), CONSTEXPR expands to 'constexpr'. Every debug
 * flag below then becomes a compile-time constant, and 'if CONSTEXPR' below
 * becomes 'if constexpr', so the compiler removes each guarded log call (or
 * debug action) entirely when its flag is off.
 *
 * In debug builds, CONSTEXPR expands to nothing. The same flags become
 * ordinary (inline) variables that can be switched on and off at runtime,
 * either by editing the tables below or via RetroShell ('log' and 'debug').
 *
 * Debug flags come in two kinds, declared in the two tables below:
 *
 * - Logging flags gate a logme() call and nothing else. Disabling one
 *   removes the call (release builds) or simply keeps it silent (debug
 *   builds); the emulator behaves identically either way. They are typed
 *   'LogLevel', because their value doubles as the severity the message is
 *   issued with. OFF disables the call, any other LogLevel enables it
 *   at that severity.
 *
 * - Debug flags enable extra behavior with a real side effect (an
 *   integrity check, a redundant computation compared against the fast
 *   path, forcing a specific code path, simulating an error condition,
 *   ...). Some of these also log a message as part of that action, but
 *   disabling the flag changes what the emulator *does*, not just what
 *   it prints. They are all typed 'bool'.
 *
 * Both tables are X-macro lists: each entry names a flag exactly once, and
 * is expanded both into the variable declaration and (in debug builds) into
 * a descriptor table used by RetroShell. To add a flag, add one line here.
 *
 * Note that rvlib maintains its own, independent set of flags (rvdebug.h).
 */

namespace vamiga {

using utl::LogLevel;
using utl::FlagInfo;

//
// Fixed severities (always active, never OFF)
//

inline constexpr LogLevel LV_OFF     = LogLevel::Off;
inline constexpr LogLevel LV_FATAL   = LogLevel::Fatal;
inline constexpr LogLevel LV_ERROR   = LogLevel::Error;
inline constexpr LogLevel LV_WARNING = LogLevel::Warn;
inline constexpr LogLevel LV_INFO    = LogLevel::Info;
inline constexpr LogLevel LV_DEBUG   = LogLevel::Debug;
inline constexpr LogLevel LV_TRACE   = LogLevel::Trace;


//
// Logging flags
//

#define LOG_FLAGS(E)                                                          \
                                                                              \
    /* General */                                                             \
    E(LOG_XFILES,         Off,  "Report paranormal activity")                 \
    E(LOG_CNF,            Off,  "Configuration options")                      \
    E(LOG_OBJ,            Off,  "Object life-times")                          \
    E(LOG_DEF,            Off,  "User defaults")                              \
                                                                              \
    /* Emulator */                                                            \
    E(LOG_RUN,            Off,  "Run loop, component states")                 \
    E(LOG_TIM,            Off,  "Thread synchronization")                     \
    E(LOG_WARP,           Off,  "Warp mode")                                  \
    E(LOG_CMD,            Off,  "Command queue")                              \
    E(LOG_MSG,            Off,  "Message queue")                              \
                                                                              \
    /* Run ahead */                                                           \
    E(LOG_RUA,            Off,  "Run-ahead activity")                         \
                                                                              \
    /* CPU */                                                                 \
    E(LOG_CPU,            Off,  "CPU")                                        \
                                                                              \
    /* Memory access */                                                       \
    E(LOG_OCSREG,         Off,  "General OCS register debugging")             \
    E(LOG_ECSREG,         Off,  "Special ECS register debugging")             \
    E(LOG_INVREG,         Off,  "Invalid register accesses")                  \
    E(LOG_MEM,            Off,  "Memory")                                     \
                                                                              \
    /* Agnus */                                                               \
    E(LOG_DMA,            Off,  "DMA registers")                              \
    E(LOG_DDF,            Off,  "Display data fetch")                         \
    E(LOG_SEQ,            Off,  "Bitplane sequencer")                         \
    E(LOG_NTSC,           Off,  "NTSC mode")                                  \
                                                                              \
    /* Copper */                                                              \
    E(LOG_COPREG,         Off,  "Copper registers")                           \
    E(LOG_COP,            Off,  "Copper execution")                           \
                                                                              \
    /* Blitter */                                                             \
    E(LOG_BLTREG,         Off,  "Blitter registers")                          \
    E(LOG_BLT_REG_GUARD,  Off,  "Register writes while Blitter runs")         \
    E(LOG_BLT,            Off,  "Blitter execution")                          \
    E(LOG_BLTTIM,         Off,  "Blitter timing")                             \
                                                                              \
    /* Denise */                                                              \
    E(LOG_BPLREG,         Off,  "Bitplane registers")                         \
    E(LOG_BPLDAT,         Off,  "BPLxDAT registers")                          \
    E(LOG_BPLMOD,         Off,  "BPLxMOD registers")                          \
    E(LOG_SPRREG,         Off,  "Sprite registers")                           \
    E(LOG_COLREG,         Off,  "Color registers")                            \
    E(LOG_CLXREG,         Off,  "Collision detection registers")              \
    E(LOG_DIW,            Off,  "Display window")                             \
    E(LOG_SPR,            Off,  "Sprites")                                    \
    E(LOG_CLX,            Off,  "Collision detection")                        \
                                                                              \
    /* Paula */                                                               \
    E(LOG_INTREG,         Off,  "Interrupt registers")                        \
    E(LOG_INT,            Off,  "Interrupt logic")                            \
                                                                              \
    /* CIAs */                                                                \
    E(LOG_CIAREG,         Off,  "CIA registers")                              \
    E(LOG_CIASER,         Off,  "CIA serial register")                        \
    E(LOG_CIA,            Off,  "CIA execution")                              \
    E(LOG_TOD,            Off,  "TODs (CIA 24-bit counters)")                 \
                                                                              \
    /* Floppy drives */                                                       \
    E(LOG_DSKREG,         Off,  "Disk controller registers")                  \
    E(LOG_DSK,            Off,  "Disk controller execution")                  \
    E(LOG_MFM,            Off,  "Disk encoder / decoder")                     \
    E(LOG_IMG,            Off,  "Disk images")                                \
                                                                              \
    /* Hard drives */                                                         \
    E(LOG_WT,             Off,  "Write-through mode")                         \
                                                                              \
    /* Audio */                                                               \
    E(LOG_AUDREG,         Off,  "Audio registers")                            \
    E(LOG_AUD,            Off,  "Audio execution")                            \
    E(LOG_AUDBUF,         Off,  "Audio buffers")                              \
    E(LOG_AUDVOL,         Off,  "Audio volume")                               \
                                                                              \
    /* Ports */                                                               \
    E(LOG_POSREG,         Off,  "Beam position registers")                    \
    E(LOG_JOYREG,         Off,  "Joystick registers")                         \
    E(LOG_POTREG,         Off,  "Potentiometer registers")                    \
    E(LOG_VID,            Off,  "Video port")                                 \
    E(LOG_PRT,            Off,  "Control ports")                              \
    E(LOG_SER,            Off,  "Serial port")                                \
    E(LOG_POT,            Off,  "Potentiometer inputs")                       \
                                                                              \
    /* Expansion boards */                                                    \
    E(LOG_ZOR,            Off,  "Zorro space")                                \
    E(LOG_ACF,            Off,  "Autoconfig")                                 \
    E(LOG_FAS,            Off,  "FastRam")                                    \
    E(LOG_HDR,            Off,  "Hard drive")                                 \
    E(LOG_DBD,            Off,  "Debug board")                                \
                                                                              \
    /* Real-time clock */                                                     \
    E(LOG_RTC,            Off,  "Real-time clock")                            \
                                                                              \
    /* Keyboard */                                                            \
    E(LOG_KBD,            Off,  "Keyboard")                                   \
    E(LOG_KEY,            Off,  "Keyboard key events")                        \
                                                                              \
    /* Misc */                                                                \
    E(LOG_RSH,            Off,  "RetroShell")                                 \
    E(LOG_REC,            Off,  "Screen recorder")                            \
    E(LOG_SCK,            Off,  "Sockets")                                    \
    E(LOG_SRV,            Off,  "Remote server")                              \
    E(LOG_GDB,            Off,  "GDB server")


//
// Debug flags
//

#define DEBUG_FLAGS(E)                                                        \
                                                                              \
    /* General */                                                             \
    E(MIMIC_UAE,            false, "Mimic UAE quirks")                        \
                                                                              \
    /* Emulator */                                                            \
    E(SNP_DEBUG,            false, "Serialization (snapshots)")               \
                                                                              \
    /* Run ahead */                                                           \
    E(RUA_CHECKSUM,         false, "Run-ahead instance integrity")            \
    E(RUA_ON_STEROIDS,      false, "Update RUA instance every frame")         \
                                                                              \
    /* Agnus */                                                               \
    E(SEQ_ON_STEROIDS,      false, "Disable sequencer fast-paths")            \
                                                                              \
    /* Copper */                                                              \
    E(COP_CHECKSUM,         false, "Compute Copper checksums")                \
                                                                              \
    /* Blitter */                                                             \
    E(BLT_CHECKSUM,         false, "Compute Blitter checksums")               \
    E(BLT_MEM_GUARD,        false, "Guard memory while Blitter runs")         \
    E(BLT_MINTERM_CHECK,    false, "Verify the Blitter minterm logic")        \
    E(SLOW_BLT_DEBUG,       false, "Execute micro-instructions in a chunk")   \
                                                                              \
    /* Denise */                                                              \
    E(BPL_ON_STEROIDS,      false, "Disable drawing fast-paths")              \
    E(BORDER_DEBUG,         false, "Draw the border in debug colors")         \
    E(DENISE_ON_STEROIDS,   false, "Disable Denise fast-paths")               \
                                                                              \
    /* Floppy drives */                                                       \
    E(ALIGN_HEAD,           false, "Make head movement deterministic")        \
    E(DSK_CHECKSUM,         false, "Compute disk checksums")                  \
    E(FS_VERIFY,            false, "Verify file system integrity")            \
                                                                              \
    /* Hard drives */                                                         \
    E(HDR_FS_LOAD_ALL,      false, "Don't filter out unneeded file systems")  \
                                                                              \
    /* Audio */                                                               \
    E(DISABLE_AUDIRQ,       false, "Disable audio interrupts")                \
                                                                              \
    /* Ports */                                                               \
    E(HOLD_MOUSE_L,         false, "Hold down the left mouse button")         \
    E(HOLD_MOUSE_M,         false, "Hold down the middle mouse button")       \
    E(HOLD_MOUSE_R,         false, "Hold down the right mouse button")        \
                                                                              \
    /* Forced error conditions */                                             \
    E(LAUNCH_ERROR,         false, "Force a launch error")                    \
    E(ROM_MISSING,          false, "Force a missing-ROM error")               \
    E(CHIP_RAM_MISSING,     false, "Force a missing chip-RAM error")          \
    E(AROS_NO_EXTROM,       false, "Force a missing AROS ext-ROM error")      \
    E(AROS_RAM_LIMIT,       false, "Force an AROS RAM-limit error")           \
    E(CHIP_RAM_LIMIT,       false, "Force a chip-RAM limit error")            \
    E(SNAP_TOO_OLD,         false, "Force a 'snapshot too old' error")        \
    E(SNAP_TOO_NEW,         false, "Force a 'snapshot too new' error")        \
    E(SNAP_IS_BETA,         false, "Force a 'beta snapshot' error")           \
    E(SNAP_CORRUPTED,       false, "Force a snapshot corruption error")       \
    E(DISK_INVALID_LAYOUT,  false, "Force a disk layout error")               \
    E(DISK_MODIFIED,        false, "Force the disk-modified flag")            \
    E(HDR_MODIFIED,         false, "Force the drive-modified flag")


//
// Logging macro and flag declarations
//

#define logme(key, format, ...) \
    do { \
        if CONSTEXPR (key != LogLevel::Off) \
            log(key, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

#define DECLARE_LOG_FLAG(name, dflt, help) \
    inline CONSTEXPR LogLevel name = LogLevel::dflt;
LOG_FLAGS(DECLARE_LOG_FLAG)
#undef DECLARE_LOG_FLAG

#define DECLARE_DEBUG_FLAG(name, dflt, help) \
    inline CONSTEXPR bool name = dflt;
DEBUG_FLAGS(DECLARE_DEBUG_FLAG)
#undef DECLARE_DEBUG_FLAG


//
// Flag descriptors (debug builds only)
//

#ifndef NDEBUG

extern const std::vector<FlagInfo> logFlags;
extern const std::vector<FlagInfo> debugFlags;

#endif

}


//
// Convenience wrappers
//

/*
#define fatal(format, ...) \
    do { \
        logme(LV_FATAL, format __VA_OPT__(,) __VA_ARGS__); \
        assert(false); \
        std::terminate(); \
    } while(0)
*/

#define xfiles(format, ...) \
    logme(LOG_XFILES, format __VA_OPT__(,) __VA_ARGS__)
