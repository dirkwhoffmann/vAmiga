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
 * either by editing the tables below or via RetroShell ('log' and 'debug').
 *
 * Debug flags come in two kinds, declared in the two tables below:
 *
 * - Logging flags gate a logme() call and nothing else. Disabling one
 *   removes the call (release builds) or simply keeps it silent (debug
 *   builds); the emulator behaves identically either way. They are typed
 *   'LogLevel', because their value doubles as the severity the message is
 *   issued with. LOG_NONE disables the call, any other LogLevel enables it
 *   at that severity. All logging flags are prefixed 'LOG_'.
 *
 * - Action flags enable extra debug behavior with a real side effect
 *   (an integrity check, a redundant computation compared against the
 *   fast path, a memory guard, forcing a specific code path, ...). Some
 *   of these also log a message as part of that action, but disabling
 *   the flag changes what the emulator *does*, not just what it prints.
 *   They are typed 'bool', except for those holding a parameter value.
 *
 * Both tables are X-macro lists: each entry names a flag exactly once, and
 * is expanded both into the variable declaration and (in debug builds) into
 * a descriptor table used by RetroShell. To add a flag, add one line here.
 */


//
// Logging flags
//

//        name              default    description
#define LOG_FLAGS(E)                                                          \
                                                                              \
    /* General */                                                             \
    E(LOG_XFILES,         LOG_NONE,  "Report paranormal activity")            \
    E(LOG_CNF,            LOG_NONE,  "Configuration options")                 \
    E(LOG_OBJ,            LOG_NONE,  "Object life-times")                     \
    E(LOG_DEF,            LOG_NONE,  "User defaults")                         \
                                                                              \
    /* Emulator */                                                            \
    E(LOG_RUN,            LOG_NONE,  "Run loop, component states")            \
    E(LOG_TIM,            LOG_NONE,  "Thread synchronization")                \
    E(LOG_WARP,           LOG_NONE,  "Warp mode")                             \
    E(LOG_CMD,            LOG_NONE,  "Command queue")                         \
    E(LOG_MSG,            LOG_NONE,  "Message queue")                         \
                                                                              \
    /* Run ahead */                                                           \
    E(LOG_RUA,            LOG_NONE,  "Run-ahead activity")                    \
                                                                              \
    /* CPU */                                                                 \
    E(LOG_CPU,            LOG_NONE,  "CPU")                                   \
                                                                              \
    /* Memory access */                                                       \
    E(LOG_OCSREG,         LOG_NONE,  "General OCS register debugging")        \
    E(LOG_ECSREG,         LOG_NONE,  "Special ECS register debugging")        \
    E(LOG_INVREG,         LOG_NONE,  "Invalid register accesses")             \
    E(LOG_MEM,            LOG_NONE,  "Memory")                                \
                                                                              \
    /* Agnus */                                                               \
    E(LOG_DMA,            LOG_NONE,  "DMA registers")                         \
    E(LOG_DDF,            LOG_NONE,  "Display data fetch")                    \
    E(LOG_SEQ,            LOG_NONE,  "Bitplane sequencer")                    \
    E(LOG_NTSC,           LOG_NONE,  "NTSC mode")                             \
                                                                              \
    /* Copper */                                                              \
    E(LOG_COPREG,         LOG_NONE,  "Copper registers")                      \
    E(LOG_COP,            LOG_NONE,  "Copper execution")                      \
                                                                              \
    /* Blitter */                                                             \
    E(LOG_BLTREG,         LOG_NONE,  "Blitter registers")                     \
    E(LOG_BLT_REG_GUARD,  LOG_NONE,  "Register writes while Blitter runs")    \
    E(LOG_BLT,            LOG_NONE,  "Blitter execution")                     \
    E(LOG_BLTTIM,         LOG_NONE,  "Blitter timing")                        \
                                                                              \
    /* Denise */                                                              \
    E(LOG_BPLREG,         LOG_NONE,  "Bitplane registers")                    \
    E(LOG_BPLDAT,         LOG_NONE,  "BPLxDAT registers")                     \
    E(LOG_BPLMOD,         LOG_NONE,  "BPLxMOD registers")                     \
    E(LOG_SPRREG,         LOG_NONE,  "Sprite registers")                      \
    E(LOG_COLREG,         LOG_NONE,  "Color registers")                       \
    E(LOG_CLXREG,         LOG_NONE,  "Collision detection registers")         \
    E(LOG_DIW,            LOG_NONE,  "Display window")                        \
    E(LOG_SPR,            LOG_NONE,  "Sprites")                               \
    E(LOG_CLX,            LOG_NONE,  "Collision detection")                   \
                                                                              \
    /* Paula */                                                               \
    E(LOG_INTREG,         LOG_NONE,  "Interrupt registers")                   \
    E(LOG_INT,            LOG_NONE,  "Interrupt logic")                       \
                                                                              \
    /* CIAs */                                                                \
    E(LOG_CIAREG,         LOG_NONE,  "CIA registers")                         \
    E(LOG_CIASER,         LOG_NONE,  "CIA serial register")                   \
    E(LOG_CIA,            LOG_NONE,  "CIA execution")                         \
    E(LOG_TOD,            LOG_NONE,  "TODs (CIA 24-bit counters)")            \
                                                                              \
    /* Floppy drives */                                                       \
    E(LOG_DSKREG,         LOG_NONE,  "Disk controller registers")             \
    E(LOG_DSK,            LOG_NONE,  "Disk controller execution")             \
    E(LOG_MFM,            LOG_NONE,  "Disk encoder / decoder")                \
    E(LOG_FS,             LOG_NONE,  "File system classes (OFS / FFS)")       \
                                                                              \
    /* Hard drives */                                                         \
    E(LOG_WT,             LOG_NONE,  "Write-through mode")                    \
                                                                              \
    /* Audio */                                                               \
    E(LOG_AUDREG,         LOG_NONE,  "Audio registers")                       \
    E(LOG_AUD,            LOG_NONE,  "Audio execution")                       \
    E(LOG_AUDBUF,         LOG_NONE,  "Audio buffers")                         \
    E(LOG_AUDVOL,         LOG_NONE,  "Audio volume")                          \
                                                                              \
    /* Ports */                                                               \
    E(LOG_POSREG,         LOG_NONE,  "Beam position registers")               \
    E(LOG_JOYREG,         LOG_NONE,  "Joystick registers")                    \
    E(LOG_POTREG,         LOG_NONE,  "Potentiometer registers")               \
    E(LOG_VID,            LOG_NONE,  "Video port")                            \
    E(LOG_PRT,            LOG_NONE,  "Control ports")                         \
    E(LOG_SER,            LOG_NONE,  "Serial port")                           \
    E(LOG_POT,            LOG_NONE,  "Potentiometer inputs")                  \
                                                                              \
    /* Expansion boards */                                                    \
    E(LOG_ZOR,            LOG_NONE,  "Zorro space")                           \
    E(LOG_ACF,            LOG_NONE,  "Autoconfig")                            \
    E(LOG_FAS,            LOG_NONE,  "FastRam")                               \
    E(LOG_HDR,            LOG_NONE,  "Hard drive")                            \
    E(LOG_DBD,            LOG_NONE,  "Debug board")                           \
                                                                              \
    /* Image files */                                                         \
    E(LOG_HDF,            LOG_NONE,  "HDF and HDZ files")                     \
    E(LOG_DMS,            LOG_NONE,  "DMS files")                             \
    E(LOG_IMG,            LOG_NONE,  "Disk images")                           \
                                                                              \
    /* Real-time clock */                                                     \
    E(LOG_RTC,            LOG_NONE,  "Real-time clock")                       \
                                                                              \
    /* Keyboard */                                                            \
    E(LOG_KBD,            LOG_NONE,  "Keyboard")                              \
    E(LOG_KEY,            LOG_NONE,  "Keyboard key events")                   \
                                                                              \
    /* Misc */                                                                \
    E(LOG_RSH,            LOG_NONE,  "RetroShell")                            \
    E(LOG_REC,            LOG_NONE,  "Screen recorder")                       \
    E(LOG_SCK,            LOG_NONE,  "Sockets")                               \
    E(LOG_SRV,            LOG_NONE,  "Remote server")                         \
    E(LOG_GDB,            LOG_NONE,  "GDB server")


//
// Action flags
//

//        type  name              default  description
#define DEBUG_FLAGS(E)                                                          \
                                                                                \
    /* General */                                                               \
    E(bool, MIMIC_UAE,          false, "Mimic UAE quirks")                      \
                                                                                \
    /* Emulator */                                                              \
    E(bool, SNP_DEBUG,          false, "Serialization (snapshots)")             \
                                                                                \
    /* Run ahead */                                                             \
    E(bool, RUA_CHECKSUM,       false, "Run-ahead instance integrity")          \
    E(bool, RUA_ON_STEROIDS,    false, "Update RUA instance every frame")       \
                                                                                \
    /* Agnus */                                                                 \
    E(bool, SEQ_ON_STEROIDS,    false, "Disable sequencer fast-paths")          \
                                                                                \
    /* Copper */                                                                \
    E(bool, COP_CHECKSUM,       false, "Compute Copper checksums")              \
                                                                                \
    /* Blitter */                                                               \
    E(bool, BLT_CHECKSUM,       false, "Compute Blitter checksums")             \
    E(bool, BLT_MEM_GUARD,      false, "Guard memory while Blitter runs")       \
    E(bool, BLT_MINTERM_CHECK,  false, "Verify the Blitter minterm logic")      \
    E(bool, SLOW_BLT_DEBUG,     false, "Execute micro-instructions in a chunk") \
                                                                                \
    /* Denise */                                                                \
    E(bool, BPL_ON_STEROIDS,    false, "Disable drawing fast-paths")            \
    E(bool, BORDER_DEBUG,       false, "Draw the border in debug colors")       \
    E(long, LINE_DEBUG,         -1,    "Draw a certain line in debug color")    \
    E(bool, DENISE_ON_STEROIDS, false, "Disable Denise fast-paths")             \
                                                                                \
    /* Floppy drives */                                                         \
    E(bool, ALIGN_HEAD,         false, "Make head movement deterministic")      \
    E(bool, DSK_CHECKSUM,       false, "Compute disk checksums")                \
    E(bool, FS_VERIFY,          false, "Verify file system integrity")          \
                                                                                \
    /* Hard drives */                                                           \
    E(bool, HDR_ACCEPT_ALL,     false, "Disable hard drive layout checks")      \
    E(bool, HDR_FS_LOAD_ALL,    false, "Don't filter out unneeded file systems")\
                                                                                \
    /* Audio */                                                                 \
    E(bool, DISABLE_AUDIRQ,     false, "Disable audio interrupts")              \
                                                                                \
    /* Ports */                                                                 \
    E(bool, HOLD_MOUSE_L,       false, "Hold down the left mouse button")       \
    E(bool, HOLD_MOUSE_M,       false, "Hold down the middle mouse button")     \
    E(bool, HOLD_MOUSE_R,       false, "Hold down the right mouse button")


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


//
// Flag declarations (generated from the tables above)
//

#define DECLARE_LOG_FLAG(name, dflt, help) \
    inline CONSTEXPR LogLevel name = LogLevel::dflt;
LOG_FLAGS(DECLARE_LOG_FLAG)
#undef DECLARE_LOG_FLAG

#define DECLARE_DEBUG_FLAG(type, name, dflt, help) \
    inline CONSTEXPR type name = dflt;
DEBUG_FLAGS(DECLARE_DEBUG_FLAG)
#undef DECLARE_DEBUG_FLAG


//
// Flag descriptors (debug builds only)
//

/* RetroShell uses these tables to list the flags and to register a setter
 * for each of them. Both accessors funnel through 'long' so that LogLevel,
 * bool, and value flags can share a single descriptor type.
 *
 * The tables exist in debug builds only. In release builds the flags are
 * 'constexpr': they cannot be assigned, and taking their address would
 * needlessly emit all of them into the binary.
 */

#ifndef NDEBUG

struct FlagInfo {

    // Name of the flag, as written in the tables above
    const char *name;

    // Human-readable description
    const char *help;

    // Indicates whether this flag is a boolean switch
    bool boolean;

    // Accessors
    long (*get)();
    void (*set)(long);
};

extern const std::vector<FlagInfo> logFlags;
extern const std::vector<FlagInfo> debugFlags;

#endif

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
