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


//
// Logging flags
//

//        name              default    description
#define LOG_FLAGS(E)                                                          \
                                                                              \
    /* General */                                                             \
    E(LOG_XFILES,         LV_OFF,  "Report paranormal activity")            \
    E(LOG_CNF,            LV_OFF,  "Configuration options")                 \
    E(LOG_OBJ,            LV_OFF,  "Object life-times")                     \
    E(LOG_DEF,            LV_OFF,  "User defaults")                         \
                                                                              \
    /* Emulator */                                                            \
    E(LOG_RUN,            LV_OFF,  "Run loop, component states")            \
    E(LOG_TIM,            LV_OFF,  "Thread synchronization")                \
    E(LOG_WARP,           LV_OFF,  "Warp mode")                             \
    E(LOG_CMD,            LV_OFF,  "Command queue")                         \
    E(LOG_MSG,            LV_OFF,  "Message queue")                         \
                                                                              \
    /* Run ahead */                                                           \
    E(LOG_RUA,            LV_OFF,  "Run-ahead activity")                    \
                                                                              \
    /* CPU */                                                                 \
    E(LOG_CPU,            LV_OFF,  "CPU")                                   \
                                                                              \
    /* Memory access */                                                       \
    E(LOG_OCSREG,         LV_OFF,  "General OCS register debugging")        \
    E(LOG_ECSREG,         LV_OFF,  "Special ECS register debugging")        \
    E(LOG_INVREG,         LV_OFF,  "Invalid register accesses")             \
    E(LOG_MEM,            LV_OFF,  "Memory")                                \
                                                                              \
    /* Agnus */                                                               \
    E(LOG_DMA,            LV_OFF,  "DMA registers")                         \
    E(LOG_DDF,            LV_OFF,  "Display data fetch")                    \
    E(LOG_SEQ,            LV_OFF,  "Bitplane sequencer")                    \
    E(LOG_NTSC,           LV_OFF,  "NTSC mode")                             \
                                                                              \
    /* Copper */                                                              \
    E(LOG_COPREG,         LV_OFF,  "Copper registers")                      \
    E(LOG_COP,            LV_OFF,  "Copper execution")                      \
                                                                              \
    /* Blitter */                                                             \
    E(LOG_BLTREG,         LV_OFF,  "Blitter registers")                     \
    E(LOG_BLT_REG_GUARD,  LV_OFF,  "Register writes while Blitter runs")    \
    E(LOG_BLT,            LV_OFF,  "Blitter execution")                     \
    E(LOG_BLTTIM,         LV_OFF,  "Blitter timing")                        \
                                                                              \
    /* Denise */                                                              \
    E(LOG_BPLREG,         LV_OFF,  "Bitplane registers")                    \
    E(LOG_BPLDAT,         LV_OFF,  "BPLxDAT registers")                     \
    E(LOG_BPLMOD,         LV_OFF,  "BPLxMOD registers")                     \
    E(LOG_SPRREG,         LV_OFF,  "Sprite registers")                      \
    E(LOG_COLREG,         LV_OFF,  "Color registers")                       \
    E(LOG_CLXREG,         LV_OFF,  "Collision detection registers")         \
    E(LOG_DIW,            LV_OFF,  "Display window")                        \
    E(LOG_SPR,            LV_OFF,  "Sprites")                               \
    E(LOG_CLX,            LV_OFF,  "Collision detection")                   \
                                                                              \
    /* Paula */                                                               \
    E(LOG_INTREG,         LV_OFF,  "Interrupt registers")                   \
    E(LOG_INT,            LV_OFF,  "Interrupt logic")                       \
                                                                              \
    /* CIAs */                                                                \
    E(LOG_CIAREG,         LV_OFF,  "CIA registers")                         \
    E(LOG_CIASER,         LV_OFF,  "CIA serial register")                   \
    E(LOG_CIA,            LV_OFF,  "CIA execution")                         \
    E(LOG_TOD,            LV_OFF,  "TODs (CIA 24-bit counters)")            \
                                                                              \
    /* Floppy drives */                                                       \
    E(LOG_DSKREG,         LV_OFF,  "Disk controller registers")             \
    E(LOG_DSK,            LV_OFF,  "Disk controller execution")             \
    E(LOG_MFM,            LV_OFF,  "Disk encoder / decoder")                \
    E(LOG_IMG,            LV_OFF,  "Disk images")                          \
                                                                              \
    /* Hard drives */                                                         \
    E(LOG_WT,             LV_OFF,  "Write-through mode")                    \
                                                                              \
    /* Audio */                                                               \
    E(LOG_AUDREG,         LV_OFF,  "Audio registers")                       \
    E(LOG_AUD,            LV_OFF,  "Audio execution")                       \
    E(LOG_AUDBUF,         LV_OFF,  "Audio buffers")                         \
    E(LOG_AUDVOL,         LV_OFF,  "Audio volume")                          \
                                                                              \
    /* Ports */                                                               \
    E(LOG_POSREG,         LV_OFF,  "Beam position registers")               \
    E(LOG_JOYREG,         LV_OFF,  "Joystick registers")                    \
    E(LOG_POTREG,         LV_OFF,  "Potentiometer registers")               \
    E(LOG_VID,            LV_OFF,  "Video port")                            \
    E(LOG_PRT,            LV_OFF,  "Control ports")                         \
    E(LOG_SER,            LV_OFF,  "Serial port")                           \
    E(LOG_POT,            LV_OFF,  "Potentiometer inputs")                  \
                                                                              \
    /* Expansion boards */                                                    \
    E(LOG_ZOR,            LV_OFF,  "Zorro space")                           \
    E(LOG_ACF,            LV_OFF,  "Autoconfig")                            \
    E(LOG_FAS,            LV_OFF,  "FastRam")                               \
    E(LOG_HDR,            LV_OFF,  "Hard drive")                            \
    E(LOG_DBD,            LV_OFF,  "Debug board")                           \
                                                                              \
    /* Real-time clock */                                                     \
    E(LOG_RTC,            LV_OFF,  "Real-time clock")                       \
                                                                              \
    /* Keyboard */                                                            \
    E(LOG_KBD,            LV_OFF,  "Keyboard")                              \
    E(LOG_KEY,            LV_OFF,  "Keyboard key events")                   \
                                                                              \
    /* Misc */                                                                \
    E(LOG_RSH,            LV_OFF,  "RetroShell")                            \
    E(LOG_REC,            LV_OFF,  "Screen recorder")                       \
    E(LOG_SCK,            LV_OFF,  "Sockets")                               \
    E(LOG_SRV,            LV_OFF,  "Remote server")                         \
    E(LOG_GDB,            LV_OFF,  "GDB server")


//
// Debug flags
//

//        type  name              default  description
#define DEBUG_FLAGS(E)                                                          \
                                                                                \
    /* General */                                                               \
    E(MIMIC_UAE,          false, "Mimic UAE quirks")                      \
                                                                                \
    /* Emulator */                                                              \
    E(SNP_DEBUG,          false, "Serialization (snapshots)")             \
                                                                                \
    /* Run ahead */                                                             \
    E(RUA_CHECKSUM,       false, "Run-ahead instance integrity")          \
    E(RUA_ON_STEROIDS,    false, "Update RUA instance every frame")       \
                                                                                \
    /* Agnus */                                                                 \
    E(SEQ_ON_STEROIDS,    false, "Disable sequencer fast-paths")          \
                                                                                \
    /* Copper */                                                                \
    E(COP_CHECKSUM,       false, "Compute Copper checksums")              \
                                                                                \
    /* Blitter */                                                               \
    E(BLT_CHECKSUM,       false, "Compute Blitter checksums")             \
    E(BLT_MEM_GUARD,      false, "Guard memory while Blitter runs")       \
    E(BLT_MINTERM_CHECK,  false, "Verify the Blitter minterm logic")      \
    E(SLOW_BLT_DEBUG,     false, "Execute micro-instructions in a chunk") \
                                                                                \
    /* Denise */                                                                \
    E(BPL_ON_STEROIDS,    false, "Disable drawing fast-paths")            \
    E(BORDER_DEBUG,       false, "Draw the border in debug colors")       \
    E(DENISE_ON_STEROIDS, false, "Disable Denise fast-paths")             \
                                                                                \
    /* Floppy drives */                                                         \
    E(ALIGN_HEAD,         false, "Make head movement deterministic")      \
    E(DSK_CHECKSUM,       false, "Compute disk checksums")                \
    E(FS_VERIFY,          false, "Verify file system integrity")          \
                                                                                \
    /* Hard drives */                                                           \
    E(HDR_FS_LOAD_ALL,    false, "Don't filter out unneeded file systems")\
                                                                                \
    /* Audio */                                                                 \
    E(DISABLE_AUDIRQ,     false, "Disable audio interrupts")              \
                                                                                \
    /* Ports */                                                                 \
    E(HOLD_MOUSE_L,       false, "Hold down the left mouse button")       \
    E(HOLD_MOUSE_M,       false, "Hold down the middle mouse button")     \
    E(HOLD_MOUSE_R,       false, "Hold down the right mouse button")      \
                                                                                \
    /* Forced error conditions */                                              \
    E(LAUNCH_ERROR,       false, "Force a launch error")                 \
    E(ROM_MISSING,        false, "Force a missing-ROM error")            \
    E(CHIP_RAM_MISSING,   false, "Force a missing chip-RAM error")       \
    E(AROS_NO_EXTROM,     false, "Force a missing AROS ext-ROM error")   \
    E(AROS_RAM_LIMIT,     false, "Force an AROS RAM-limit error")        \
    E(CHIP_RAM_LIMIT,     false, "Force a chip-RAM limit error")         \
    E(SNAP_TOO_OLD,       false, "Force a 'snapshot too old' error")     \
    E(SNAP_TOO_NEW,       false, "Force a 'snapshot too new' error")     \
    E(SNAP_IS_BETA,       false, "Force a 'beta snapshot' error")        \
    E(SNAP_CORRUPTED,     false, "Force a snapshot corruption error")    \
    E(DISK_INVALID_LAYOUT,false, "Force a disk layout error")            \
    E(DISK_MODIFIED,      false, "Force the disk-modified flag")         \
    E(HDR_MODIFIED,       false, "Force the drive-modified flag")


//
// Logging macro
//

#define logme(key, format, ...) \
    do { \
        if CONSTEXPR (key != LogLevel::LV_OFF) \
            log(key, std::source_location::current(), \
                format __VA_OPT__(,) __VA_ARGS__); \
    } while (0)


namespace vamiga {

using utl::LogLevel;
using utl::FlagInfo;

//
// Fixed severities (always active, never OFF)
//

inline constexpr LogLevel LV_FATAL   = LogLevel::LV_FATAL;
inline constexpr LogLevel LV_ERROR   = LogLevel::LV_ERROR;
inline constexpr LogLevel LV_WARNING = LogLevel::LV_WARN;
inline constexpr LogLevel LV_INFO    = LogLevel::LV_INFO;
inline constexpr LogLevel LV_DEBUG   = LogLevel::LV_DEBUG;
inline constexpr LogLevel LV_TRACE   = LogLevel::LV_TRACE;

// Always-off placeholder, used to permanently silence a log call
inline constexpr LogLevel LOG_NULLDEV = LogLevel::LV_OFF;


//
// Flag declarations (generated from the tables above)
//

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

/* RetroShell uses these tables to list the flags and to register a setter
 * for each of them. See utl::FlagInfo (Loggable.h) for the descriptor type.
 */

#ifndef NDEBUG

extern const std::vector<FlagInfo> logFlags;
extern const std::vector<FlagInfo> debugFlags;

#endif

}


//
// Convenience wrappers
//

#define fatal(format, ...) \
    do { \
        logme(LV_FATAL, format __VA_OPT__(,) __VA_ARGS__); \
        assert(false); \
        std::terminate(); \
    } while(0)

#define xfiles(format, ...) \
    logme(LOG_XFILES, format __VA_OPT__(,) __VA_ARGS__)
