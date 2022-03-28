// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

//
// Release settings
//

// Version number
#define VER_MAJOR 2
#define VER_MINOR 0
#define VER_SUBMINOR 0
#define VER_BETA 1

// Snapshot version number
#define SNP_MAJOR 2
#define SNP_MINOR 0
#define SNP_SUBMINOR 0
#define SNP_BETA 1

// Uncomment this setting in a release build
// #define RELEASEBUILD

//
// Build settings
//

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif

// Type alias for the datatype used by the host machine's audio backend
// struct U16Mono; typedef U16Mono SampleType;
// struct U16Stereo; typedef U16Stereo SampleType;
struct FloatStereo; typedef struct FloatStereo SampleType;


//
// Configuration overrides
//

#define OVERRIDES { }
/*
 { \
 { OPT_AGNUS_REVISION,   AGNUS_OCS      }, \
 { OPT_BLITTER_ACCURACY, 0              }, \
 { OPT_CHIP_RAM,         512            }, \
 { OPT_SLOW_RAM,         512            }, \
 { OPT_FAST_RAM,         0              }, \
 { OPT_RTC_MODEL,        RTC_NONE       }, \
 { OPT_DRIVE_SPEED,      -1             }  }
*/

// Uncomment to colorize a certain row or column
// #define LINE_DEBUG (agnus.pos.v == 260 || agnus.pos.v == 300)
// #define COLUMN_DEBUG 114


//
// Launch settings
//

// Add pathes to ADF files to launch the emulator with preset floppy disks
#define INITIAL_DF0 ""
#define INITIAL_DF1 ""
#define INITIAL_DF2 ""
#define INITIAL_DF3 ""

// Add pathes to HDF files to launch the emulator with preset hard drives
#define INITIAL_HD0 "" // /tmp/rdb2.hdf"
#define INITIAL_HD1 ""
#define INITIAL_HD2 ""
#define INITIAL_HD3 ""

// Add a path to a snapshot file to launch the emulator in a preset state
#define INITIAL_SNAPSHOT ""

// Add addresses to launch the emulator with preset breakpoints
#define INITIAL_BREAKPOINTS { }


//
// Execution settings
//

static const int NO_SEQ_FASTPATH = 0; // Disable sequencer speed optimizations
static const int LEGACY_COPPER   = 0; // Enable deprecated Copper code
static const int DIAG_BOARD      = 1; // Plug in the diagnose board


//
// Debug settings
//

// General
static const int XFILES          = 0; // Report paranormal activity
static const int CNF_DEBUG       = 0; // Configuration options
static const int OBJ_DEBUG       = 0; // Object life-times
static const int MIMIC_UAE       = 0; // Enable to compare debug logs with UAE

// Runloop
static const int RUN_DEBUG       = 0; // Run loop, component states
static const int WARP_DEBUG      = 0; // Warp mode
static const int QUEUE_DEBUG     = 0; // Message queue
static const int SNP_DEBUG       = 0; // Serialization (snapshots)

// CPU
static const int CPU_DEBUG       = 0; // CPU
static const int CST_DEBUG       = 0; // Call stack recording

// Memory access
static const int OCSREG_DEBUG    = 0; // General OCS register debugging
static const int ECSREG_DEBUG    = 0; // Special ECS register debugging
static const int INVREG_DEBUG    = 0; // Invalid register accesses
static const int MEM_DEBUG       = 0; // Memory

// Agnus
static const int DMA_DEBUG       = 0; // DMA registers
static const int DDF_DEBUG       = 0; // Display data fetch
static const int SEQ_DEBUG       = 0; // Bitplane sequencer
static const int NO_PTR_DROPS    = 0; // Never drop a pointer register write

// Copper
static const int COP_CHECKSUM    = 0; // Compute Copper checksums
static const int COPREG_DEBUG    = 0; // Copper registers
static const int COP_DEBUG       = 0; // Copper execution

// Blitter
static const int BLT_CHECKSUM    = 0; // Compute Blitter checksums
static const int BLTREG_DEBUG    = 0; // Blitter registers
static const int BLT_GUARD       = 0; // Guard registers while Blitter runs
static const int BLT_DEBUG       = 0; // Blitter execution
static const int BLTTIM_DEBUG    = 0; // Blitter Timing
static const int SLOW_BLT_DEBUG  = 0; // Execute micro-instructions in one chunk
static const int OLD_LINE_BLIT   = 0; // Use the old line blitter implementation

// Denise
static const int BPLREG_DEBUG    = 0; // Bitplane registers
static const int BPLDAT_DEBUG    = 0; // BPLxDAT registers
static const int BPLMOD_DEBUG    = 0; // BPLxMOD registers
static const int SPRREG_DEBUG    = 0; // Sprite registers
static const int COLREG_DEBUG    = 0; // Color registers
static const int CLXREG_DEBUG    = 0; // Collision detection registers
static const int BPL_DEBUG       = 0; // Bitplane DMA
static const int DIW_DEBUG       = 0; // Display window
static const int SPR_DEBUG       = 0; // Sprites
static const int CLX_DEBUG       = 0; // Collision detection
static const int BORDER_DEBUG    = 0; // Draw the border in debug colors

// Paula
static const int INTREG_DEBUG    = 0; // Interrupt registers
static const int INT_DEBUG       = 0; // Interrupt logic

// CIAs
static const int CIA_ON_STEROIDS = 0; // Keep the CIAs awake all the time
static const int CIAREG_DEBUG    = 0; // CIA registers
static const int CIASER_DEBUG    = 0; // CIA serial register
static const int CIA_DEBUG       = 0; // CIA execution
static const int TOD_DEBUG       = 0; // TODs (CIA 24-bit counters)

// Floppy Drives
static const int ALIGN_HEAD      = 0; // Make head movement deterministic
static const int DSK_CHECKSUM    = 0; // Compute disk checksums
static const int DSKREG_DEBUG    = 0; // Disk controller registers
static const int DSK_DEBUG       = 0; // Disk controller execution
static const int MFM_DEBUG       = 0; // Disk encoder / decoder
static const int FS_DEBUG        = 0; // File System Classes (OFS / FFS)

// Hard Drives
static const int HDR_ACCEPT_ALL  = 0; // Disables hard drive layout checks

// Audio
static const int AUDREG_DEBUG    = 0; // Audio registers
static const int AUD_DEBUG       = 0; // Audio execution
static const int AUDBUF_DEBUG    = 0; // Audio buffers
static const int DISABLE_AUDIRQ  = 0; // Disable audio interrupts

// Ports
static const int POSREG_DEBUG    = 0; // POSxxx registers
static const int JOYREG_DEBUG    = 0; // JOYxxx registers
static const int POTREG_DEBUG    = 0; // POTxxx registers
static const int PRT_DEBUG       = 0; // Control ports and connected devices
static const int SER_DEBUG       = 0; // Serial interface
static const int POT_DEBUG       = 0; // Potentiometer inputs
static const int HOLD_MOUSE_L    = 0; // Hold down the left mouse button
static const int HOLD_MOUSE_R    = 0; // Hold down the right mouse button

// Expansion boards
static const int ZOR_DEBUG       = 0; // Zorro space, autoconfig
static const int FAS_DEBUG       = 0; // FastRam
static const int HDR_DEBUG       = 0; // HardDrive

// Media types
static const int ADF_DEBUG       = 0; // ADF and extended ADF files
static const int DMS_DEBUG       = 0; // DMS files
static const int IMG_DEBUG       = 0; // IMG files

// Other components
static const int RTC_DEBUG       = 0; // Real-time clock
static const int KBD_DEBUG       = 0; // Keyboard

// Misc
static const int REC_DEBUG       = 0; // Screen recorder
static const int SCK_DEBUG       = 0; // Sockets
static const int SRV_DEBUG       = 0; // Remote server
static const int GDB_DEBUG       = 0; // GDB server


//
// Forced error conditions
//

static const int FORCE_ROM_MISSING              = 0;
static const int FORCE_CHIP_RAM_MISSING         = 0;
static const int FORCE_AROS_NO_EXTROM           = 0;
static const int FORCE_AROS_RAM_LIMIT           = 0;
static const int FORCE_CHIP_RAM_LIMIT           = 0;
static const int FORCE_SNAP_TOO_OLD             = 0;
static const int FORCE_SNAP_TOO_NEW             = 0;
static const int FORCE_SNAP_IS_BETA             = 0;
static const int FORCE_SNAP_CORRUPTED           = 0;
static const int FORCE_DISK_INVALID_LAYOUT      = 0;
static const int FORCE_DISK_MODIFIED            = 0;
static const int FORCE_HDR_TOO_LARGE            = 0;
static const int FORCE_HDR_UNSUPPORTED_C        = 0;
static const int FORCE_HDR_UNSUPPORTED_H        = 0;
static const int FORCE_HDR_UNSUPPORTED_S        = 0;
static const int FORCE_HDR_UNSUPPORTED_B        = 0;
static const int FORCE_HDR_MODIFIED             = 0;
static const int FORCE_FS_WRONG_BSIZE           = 0;
static const int FORCE_FS_WRONG_CAPACITY        = 0;
static const int FORCE_FS_WRONG_DOS_TYPE        = 0;
static const int FORCE_DMS_CANT_CREATE          = 0;
static const int FORCE_RECORDING_ERROR          = 0;
static const int FORCE_NO_FFMPEG                = 0;


#ifdef RELEASEBUILD
#ifndef NDEBUG
#define NDEBUG
#endif
static const bool releaseBuild = 1;
static const bool debugBuild = 0;
#else
static const bool releaseBuild = 0;
static const bool debugBuild = 1;
#endif

#if VER_BETA == 0
static const bool betaRelease = 0;
#else
static const bool betaRelease = 1;
#endif

#include "assert.h"
