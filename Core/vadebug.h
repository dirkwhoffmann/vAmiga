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
 * - Logging flags gate a logmsg() call and nothing else. Disabling one
 *   removes the call (release builds) or simply keeps it silent (debug
 *   builds); the emulator behaves identically either way. They are typed
 *   'long', because their value doubles as the severity the message is
 *   issued with. LOG_OFF disables the call, any other severity enables it
 *   at that level.
 *
 * - Debug flags enable extra behavior with a real side effect (an
 *   integrity check, a redundant computation compared against the fast
 *   path, forcing a specific code path, simulating an error condition,
 *   ...). Some of these also log a message as part of that action, but
 *   disabling the flag changes what the emulator *does*, not just what
 *   it prints. Each one carries its type in the first column. Most are
 *   plain 'bool' switches, but a flag may also hold a parameter, as
 *   'isize' does for TRACE_FRAME. RetroShell derives its interface from
 *   that type: a bool flag is set with 'debug set <name> true|false',
 *   anything else takes a numeric argument (see FlagInfo::boolean, which
 *   the descriptor table computes from the declared type).
 *
 * Both tables are X-macro lists: each entry names a flag exactly once, and
 * is expanded both into the variable declaration and (in debug builds) into
 * a descriptor table used by RetroShell. To add a flag, add one line here.
 *
 * Note that rvlib maintains its own, independent set of flags (rvdebug.h).
 */

namespace vamiga {

using utl::LOG_OFF;
using utl::LOG_FATAL;
using utl::LOG_ERROR;
using utl::LOG_WARN;
using utl::LOG_INFO;
using utl::LOG_DEBUG;
using utl::LOG_TRACE;
using utl::FlagInfo;
using utl::isize;


//
// Logging flags
//

#define VA_LOG_FLAGS(E)                                                          \
                                                                              \
    /* General */                                                             \
    E(LOG_XFILES,         LOG_OFF,  "Report paranormal activity")             \
    E(LOG_CNF,            LOG_OFF,  "Configuration options")                  \
    E(LOG_OBJ,            LOG_OFF,  "Object life-times")                      \
    E(LOG_DEF,            LOG_OFF,  "User defaults")                          \
                                                                              \
    /* Emulator */                                                            \
    E(LOG_RUN,            LOG_OFF,  "Run loop, component states")             \
    E(LOG_TIM,            LOG_OFF,  "Thread synchronization")                 \
    E(LOG_WARP,           LOG_OFF,  "Warp mode")                              \
    E(LOG_CMD,            LOG_OFF,  "Command queue")                          \
    E(LOG_MSG,            LOG_OFF,  "Message queue")                          \
                                                                              \
    /* Run ahead */                                                           \
    E(LOG_RUA,            LOG_OFF,  "Run-ahead activity")                     \
                                                                              \
    /* CPU */                                                                 \
    E(LOG_CPU,            LOG_OFF,  "CPU")                                    \
                                                                              \
    /* Memory access */                                                       \
    E(LOG_OCSREG,         LOG_OFF,  "General OCS register debugging")         \
    E(LOG_ECSREG,         LOG_OFF,  "Special ECS register debugging")         \
    E(LOG_INVREG,         LOG_OFF,  "Invalid register accesses")              \
    E(LOG_MEM,            LOG_OFF,  "Memory")                                 \
                                                                              \
    /* Agnus */                                                               \
    E(LOG_DMA,            LOG_OFF,  "DMA registers")                          \
    E(LOG_DDF,            LOG_OFF,  "Display data fetch")                     \
    E(LOG_SEQ,            LOG_OFF,  "Bitplane sequencer")                     \
    E(LOG_NTSC,           LOG_OFF,  "NTSC mode")                              \
                                                                              \
    /* Copper */                                                              \
    E(LOG_COPREG,         LOG_OFF,  "Copper registers")                       \
    E(LOG_COP,            LOG_OFF,  "Copper execution")                       \
                                                                              \
    /* Blitter */                                                             \
    E(LOG_BLTREG,         LOG_OFF,  "Blitter registers")                      \
    E(LOG_BLT_REG_GUARD,  LOG_OFF,  "Register writes while Blitter runs")     \
    E(LOG_BLT,            LOG_OFF,  "Blitter execution")                      \
    E(LOG_BLTTIM,         LOG_OFF,  "Blitter timing")                         \
                                                                              \
    /* Denise */                                                              \
    E(LOG_BPLREG,         LOG_OFF,  "Bitplane registers")                     \
    E(LOG_BPLDAT,         LOG_OFF,  "BPLxDAT registers")                      \
    E(LOG_BPLMOD,         LOG_OFF,  "BPLxMOD registers")                      \
    E(LOG_SPRREG,         LOG_OFF,  "Sprite registers")                       \
    E(LOG_COLREG,         LOG_OFF,  "Color registers")                        \
    E(LOG_CLXREG,         LOG_OFF,  "Collision detection registers")          \
    E(LOG_DIW,            LOG_OFF,  "Display window")                         \
    E(LOG_SPR,            LOG_OFF,  "Sprites")                                \
    E(LOG_CLX,            LOG_OFF,  "Collision detection")                    \
                                                                              \
    /* Paula */                                                               \
    E(LOG_INTREG,         LOG_OFF,  "Interrupt registers")                    \
    E(LOG_INT,            LOG_OFF,  "Interrupt logic")                        \
                                                                              \
    /* CIAs */                                                                \
    E(LOG_CIAREG,         LOG_OFF,  "CIA registers")                          \
    E(LOG_CIASER,         LOG_OFF,  "CIA serial register")                    \
    E(LOG_CIA,            LOG_OFF,  "CIA execution")                          \
    E(LOG_TOD,            LOG_OFF,  "TODs (CIA 24-bit counters)")             \
                                                                              \
    /* Floppy drives */                                                       \
    E(LOG_DSKREG,         LOG_OFF,  "Disk controller registers")              \
    E(LOG_DSK,            LOG_OFF,  "Disk controller execution")              \
    E(LOG_MFM,            LOG_OFF,  "Disk encoder / decoder")                 \
    E(LOG_IMG,            LOG_OFF,  "Disk images")                            \
                                                                              \
    /* Hard drives */                                                         \
    E(LOG_WT,             LOG_OFF,  "Write-through mode")                     \
                                                                              \
    /* Audio */                                                               \
    E(LOG_AUDREG,         LOG_OFF,  "Audio registers")                        \
    E(LOG_AUD,            LOG_OFF,  "Audio execution")                        \
    E(LOG_AUDBUF,         LOG_OFF,  "Audio buffers")                          \
    E(LOG_AUDVOL,         LOG_OFF,  "Audio volume")                           \
                                                                              \
    /* Ports */                                                               \
    E(LOG_POSREG,         LOG_OFF,  "Beam position registers")                \
    E(LOG_JOYREG,         LOG_OFF,  "Joystick registers")                     \
    E(LOG_POTREG,         LOG_OFF,  "Potentiometer registers")                \
    E(LOG_VID,            LOG_OFF,  "Video port")                             \
    E(LOG_PRT,            LOG_OFF,  "Control ports")                          \
    E(LOG_SER,            LOG_OFF,  "Serial port")                            \
    E(LOG_POT,            LOG_OFF,  "Potentiometer inputs")                   \
                                                                              \
    /* Expansion boards */                                                    \
    E(LOG_ZOR,            LOG_OFF,  "Zorro space")                            \
    E(LOG_ACF,            LOG_OFF,  "Autoconfig")                             \
    E(LOG_FAS,            LOG_OFF,  "FastRam")                                \
    E(LOG_HDR,            LOG_OFF,  "Hard drive")                             \
    E(LOG_DBD,            LOG_OFF,  "Debug board")                            \
                                                                              \
    /* Real-time clock */                                                     \
    E(LOG_RTC,            LOG_OFF,  "Real-time clock")                        \
                                                                              \
    /* Keyboard */                                                            \
    E(LOG_KBD,            LOG_OFF,  "Keyboard")                               \
    E(LOG_KEY,            LOG_OFF,  "Keyboard key events")                    \
                                                                              \
    /* Misc */                                                                \
    E(LOG_RSH,            LOG_OFF,  "RetroShell")                             \
    E(LOG_REC,            LOG_OFF,  "Screen recorder")                        \
    E(LOG_SCK,            LOG_OFF,  "Sockets")                                \
    E(LOG_SRV,            LOG_OFF,  "Remote server")                          \
    E(LOG_GDB,            LOG_OFF,  "GDB server")


//
// Debug flags
//

#define VA_DEBUG_FLAGS(E)                                                          \
                                                                                   \
    /* General */                                                                  \
    E(bool,  MIMIC_UAE,           false, "Mimic UAE quirks")                       \
                                                                                   \
    /* Emulator */                                                                 \
    E(bool,  SNP_DEBUG,           false, "Serialization (snapshots)")              \
                                                                                   \
    /* Run ahead */                                                                \
    E(bool,  RUA_CHECKSUM,        false, "Run-ahead instance integrity")           \
    E(bool,  RUA_ON_STEROIDS,     false, "Update RUA instance every frame")        \
                                                                                   \
    /* CPU */                                                                      \
    E(isize, TRACE_FRAME,         -1,    "Frame to trace (-1 = no tracing)")       \
                                                                                   \
    /* Agnus */                                                                    \
    E(bool,  SEQ_ON_STEROIDS,     false, "Disable sequencer fast-paths")           \
                                                                                   \
    /* Copper */                                                                   \
    E(bool,  COP_CHECKSUM,        false, "Compute Copper checksums")               \
                                                                                   \
    /* Blitter */                                                                  \
    E(bool,  BLT_CHECKSUM,        false, "Compute Blitter checksums")              \
    E(bool,  BLT_MEM_GUARD,       false, "Guard memory while Blitter runs")        \
    E(bool,  BLT_MINTERM_CHECK,   false, "Verify the Blitter minterm logic")       \
    E(bool,  SLOW_BLT_DEBUG,      false, "Execute micro-instructions in a chunk")  \
                                                                                   \
    /* Denise */                                                                   \
    E(bool,  BPL_ON_STEROIDS,     false, "Disable drawing fast-paths")             \
    E(bool,  BORDER_DEBUG,        false, "Draw the border in debug colors")        \
    E(bool,  BORDER_DISABLE,      false, "Never draw the border")                  \
    E(bool,  DENISE_ON_STEROIDS,  false, "Disable Denise fast-paths")              \
    E(isize, LINE_MARKER,         -1,    "Highlight a raster line (-1 to disable)")\
                                                                                   \
    /* Floppy drives */                                                            \
    E(bool,  ALIGN_HEAD,          false, "Make head movement deterministic")       \
    E(bool,  DSK_CHECKSUM,        false, "Compute disk checksums")                 \
    E(bool,  FS_VERIFY,           false, "Verify file system integrity")           \
                                                                                   \
    /* Hard drives */                                                              \
    E(bool,  HDR_FS_LOAD_ALL,     false, "Don't filter out unneeded file systems") \
                                                                                   \
    /* Audio */                                                                    \
    E(bool,  DISABLE_AUDIRQ,      false, "Disable audio interrupts")               \
                                                                                   \
    /* Ports */                                                                    \
    E(bool,  HOLD_MOUSE_L,        false, "Hold down the left mouse button")        \
    E(bool,  HOLD_MOUSE_M,        false, "Hold down the middle mouse button")      \
    E(bool,  HOLD_MOUSE_R,        false, "Hold down the right mouse button")       \
                                                                                   \
    /* Forced error conditions */                                                  \
    E(bool,  LAUNCH_ERROR,        false, "Force a launch error")                   \
    E(bool,  ROM_MISSING,         false, "Force a missing-ROM error")              \
    E(bool,  CHIP_RAM_MISSING,    false, "Force a missing chip-RAM error")         \
    E(bool,  AROS_NO_EXTROM,      false, "Force a missing AROS ext-ROM error")     \
    E(bool,  AROS_RAM_LIMIT,      false, "Force an AROS RAM-limit error")          \
    E(bool,  CHIP_RAM_LIMIT,      false, "Force a chip-RAM limit error")           \
    E(bool,  SNAP_TOO_OLD,        false, "Force a 'snapshot too old' error")       \
    E(bool,  SNAP_TOO_NEW,        false, "Force a 'snapshot too new' error")       \
    E(bool,  SNAP_IS_BETA,        false, "Force a 'beta snapshot' error")          \
    E(bool,  SNAP_CORRUPTED,      false, "Force a snapshot corruption error")      \
    E(bool,  DISK_INVALID_LAYOUT, false, "Force a disk layout error")              \
    E(bool,  DISK_MODIFIED,       false, "Force the disk-modified flag")           \
    E(bool,  HDR_MODIFIED,        false, "Force the drive-modified flag")


//
// Flag declarations
//

#define DECLARE_LOG_FLAG(name, dflt, help) \
    inline CONSTEXPR long name = dflt;
VA_LOG_FLAGS(DECLARE_LOG_FLAG)
#undef DECLARE_LOG_FLAG

#define DECLARE_DEBUG_FLAG(type, name, dflt, help) \
    inline CONSTEXPR type name = dflt;
VA_DEBUG_FLAGS(DECLARE_DEBUG_FLAG)
#undef DECLARE_DEBUG_FLAG


//
// Flag descriptors (debug builds)
//

#ifndef NDEBUG

extern const std::vector<FlagInfo> logFlags;
extern const std::vector<FlagInfo> debugFlags;

#endif

}
