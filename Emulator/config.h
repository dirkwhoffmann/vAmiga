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
#define VER_MINOR 4
#define VER_SUBMINOR 0
#define VER_BETA 0

// Snapshot version number
#define SNP_MAJOR 2
#define SNP_MINOR 4
#define SNP_SUBMINOR 0
#define SNP_BETA 0

// Uncomment this setting in a release build
// #define RELEASEBUILD


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

// Uncomment to colorize a certain scanline
// #define LINE_DEBUG (vpos == 0 || vpos == 160)
// #define LINE_DEBUG (vpos == 200)


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
// Video settings
//

/* Texels per pixel. Set to 1 to generate a texture in hires resolution (every
 * hires pixel is represented by a single texel). Set to 2 to generate a
 * texture in super-hires resolution (every hires pixel is represented by a
 * two texels).
 */
#define TPP 1


//
// Audio settings
//

// Type alias for the datatype used by the host machine's audio backend
#define SAMPLE_T FloatStereo

// Scaling factor used by the FloatStereo SampleType
#define AUD_SCALE 0.00001f


//
// Execution settings
//

static const int NO_SEQ_FASTPATH = 0; // Disable sequencer fast path
static const int NO_BPL_FASTPATH = 0; // Disable drawing fast path
static const int DIAG_BOARD      = 0; // Plug in the diagnose board
static const int FPU_SUPPORT     = 0; // Enable FPU (not working yet)


//
// Debug settings
//

#ifdef RELEASEBUILD
#ifndef NDEBUG
#define NDEBUG
#endif
static const bool releaseBuild = 1;
static const bool debugBuild = 0;
typedef const int debugflag;
#else
static const bool releaseBuild = 0;
static const bool debugBuild = 1;
typedef int debugflag;
#endif

#if VER_BETA == 0
static const bool betaRelease = 0;
#else
static const bool betaRelease = 1;
#endif


// General
static debugflag XFILES          = 0; // Report paranormal activity
static debugflag CNF_DEBUG       = 0; // Configuration options
static debugflag OBJ_DEBUG       = 0; // Object life-times
static debugflag DEF_DEBUG       = 0; // User defaults
static debugflag MIMIC_UAE       = 0; // Enable to compare debug logs with UAE

// Runloop
static debugflag RUN_DEBUG       = 0; // Run loop, component states
static debugflag WARP_DEBUG      = 0; // Warp mode
static debugflag QUEUE_DEBUG     = 0; // Message queue
static debugflag SNP_DEBUG       = 0; // Serialization (snapshots)

// CPU
static debugflag CPU_DEBUG       = 0; // CPU
static debugflag FPU_DEBUG       = 0; // Floating point unit
static debugflag CST_DEBUG       = 0; // Call stack recording

// Memory access
static debugflag OCSREG_DEBUG    = 0; // General OCS register debugging
static debugflag ECSREG_DEBUG    = 0; // Special ECS register debugging
static debugflag INVREG_DEBUG    = 0; // Invalid register accesses
static debugflag MEM_DEBUG       = 0; // Memory

// Agnus
static debugflag DMA_DEBUG       = 0; // DMA registers
static debugflag DDF_DEBUG       = 0; // Display data fetch
static debugflag SEQ_DEBUG       = 0; // Bitplane sequencer
static debugflag NTSC_DEBUG      = 0; // NTSC mode

// Copper
static debugflag COP_CHECKSUM    = 0; // Compute Copper checksums
static debugflag COPREG_DEBUG    = 0; // Copper registers
static debugflag COP_DEBUG       = 0; // Copper execution

// Blitter
static debugflag BLT_CHECKSUM    = 0; // Compute Blitter checksums
static debugflag BLTREG_DEBUG    = 0; // Blitter registers
static debugflag BLT_REG_GUARD   = 0; // Guard registers while Blitter runs
static debugflag BLT_MEM_GUARD   = 0; // Guard memory while Blitter runs
static debugflag BLT_DEBUG       = 0; // Blitter execution
static debugflag BLTTIM_DEBUG    = 0; // Blitter Timing
static debugflag SLOW_BLT_DEBUG  = 0; // Execute micro-instructions in one chunk
static debugflag OLD_LINE_BLIT   = 0; // Use the old line blitter implementation

// Denise
static debugflag BPLREG_DEBUG    = 0; // Bitplane registers
static debugflag BPLDAT_DEBUG    = 0; // BPLxDAT registers
static debugflag BPLMOD_DEBUG    = 0; // BPLxMOD registers
static debugflag SPRREG_DEBUG    = 0; // Sprite registers
static debugflag COLREG_DEBUG    = 0; // Color registers
static debugflag CLXREG_DEBUG    = 0; // Collision detection registers
static debugflag BPL_DEBUG       = 0; // Bitplane DMA
static debugflag DIW_DEBUG       = 0; // Display window
static debugflag SPR_DEBUG       = 0; // Sprites
static debugflag CLX_DEBUG       = 0; // Collision detection
static debugflag BORDER_DEBUG    = 0; // Draw the border in debug colors

// Paula
static debugflag INTREG_DEBUG    = 0; // Interrupt registers
static debugflag INT_DEBUG       = 0; // Interrupt logic

// CIAs
static debugflag CIA_ON_STEROIDS = 0; // Keep the CIAs awake all the time
static debugflag CIAREG_DEBUG    = 0; // CIA registers
static debugflag CIASER_DEBUG    = 0; // CIA serial register
static debugflag CIA_DEBUG       = 0; // CIA execution
static debugflag TOD_DEBUG       = 0; // TODs (CIA 24-bit counters)

// Floppy Drives
static debugflag ALIGN_HEAD      = 0; // Make head movement deterministic
static debugflag DSK_CHECKSUM    = 0; // Compute disk checksums
static debugflag DSKREG_DEBUG    = 0; // Disk controller registers
static debugflag DSK_DEBUG       = 0; // Disk controller execution
static debugflag MFM_DEBUG       = 0; // Disk encoder / decoder
static debugflag FS_DEBUG        = 0; // File System Classes (OFS / FFS)

// Hard Drives
static debugflag HDR_ACCEPT_ALL  = 0; // Disables hard drive layout checks
static debugflag HDR_FS_LOAD_ALL = 0; // Don't filter out unneeded file systems
static debugflag WT_DEBUG        = 0; // Write-through mode

// Audio
static debugflag AUDREG_DEBUG    = 0; // Audio registers
static debugflag AUD_DEBUG       = 0; // Audio execution
static debugflag AUDBUF_DEBUG    = 0; // Audio buffers
static debugflag DISABLE_AUDIRQ  = 0; // Disable audio interrupts

// Ports
static debugflag POSREG_DEBUG    = 0; // POSxxx registers
static debugflag JOYREG_DEBUG    = 0; // JOYxxx registers
static debugflag POTREG_DEBUG    = 0; // POTxxx registers
static debugflag PRT_DEBUG       = 0; // Control ports and connected devices
static debugflag SER_DEBUG       = 0; // Serial interface
static debugflag POT_DEBUG       = 0; // Potentiometer inputs
static debugflag HOLD_MOUSE_L    = 0; // Hold down the left mouse button
static debugflag HOLD_MOUSE_M    = 0; // Hold down the middle mouse button
static debugflag HOLD_MOUSE_R    = 0; // Hold down the right mouse button

// Expansion boards
static debugflag ZOR_DEBUG       = 0; // Zorro space
static debugflag ACF_DEBUG       = 0; // Autoconfig
static debugflag FAS_DEBUG       = 0; // FastRam
static debugflag HDR_DEBUG       = 0; // HardDrive
static debugflag DBD_DEBUG       = 0; // DebugBoard

// Media types
static debugflag ADF_DEBUG       = 0; // ADF and extended ADF files
static debugflag DMS_DEBUG       = 0; // DMS files
static debugflag IMG_DEBUG       = 0; // IMG files

// Other components
static debugflag RTC_DEBUG       = 0; // Real-time clock
static debugflag KBD_DEBUG       = 0; // Keyboard

// Misc
static debugflag REC_DEBUG       = 0; // Screen recorder
static debugflag SCK_DEBUG       = 0; // Sockets
static debugflag SRV_DEBUG       = 0; // Remote server
static debugflag GDB_DEBUG       = 0; // GDB server


//
// Forced error conditions
//

static debugflag FORCE_ROM_MISSING              = 0;
static debugflag FORCE_CHIP_RAM_MISSING         = 0;
static debugflag FORCE_AROS_NO_EXTROM           = 0;
static debugflag FORCE_AROS_RAM_LIMIT           = 0;
static debugflag FORCE_CHIP_RAM_LIMIT           = 0;
static debugflag FORCE_SNAP_TOO_OLD             = 0;
static debugflag FORCE_SNAP_TOO_NEW             = 0;
static debugflag FORCE_SNAP_IS_BETA             = 0;
static debugflag FORCE_SNAP_CORRUPTED           = 0;
static debugflag FORCE_DISK_INVALID_LAYOUT      = 0;
static debugflag FORCE_DISK_MODIFIED            = 0;
static debugflag FORCE_HDR_TOO_LARGE            = 0;
static debugflag FORCE_HDR_UNSUPPORTED_C        = 0;
static debugflag FORCE_HDR_UNSUPPORTED_H        = 0;
static debugflag FORCE_HDR_UNSUPPORTED_S        = 0;
static debugflag FORCE_HDR_UNSUPPORTED_B        = 0;
static debugflag FORCE_HDR_UNKNOWN_GEOMETRY     = 0;
static debugflag FORCE_HDR_MODIFIED             = 0;
static debugflag FORCE_FS_WRONG_BSIZE           = 0;
static debugflag FORCE_FS_WRONG_CAPACITY        = 0;
static debugflag FORCE_FS_WRONG_DOS_TYPE        = 0;
static debugflag FORCE_DMS_CANT_CREATE          = 0;
static debugflag FORCE_RECORDING_ERROR          = 0;
static debugflag FORCE_NO_FFMPEG                = 0;

#include "assert.h"
