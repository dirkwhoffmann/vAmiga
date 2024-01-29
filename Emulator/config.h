// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

//
// Release settings
//

// Version number
#define VER_MAJOR 2
#define VER_MINOR 5
#define VER_SUBMINOR 0
#define VER_BETA 0

// Snapshot version number
#define SNP_MAJOR 2
#define SNP_MINOR 5
#define SNP_SUBMINOR 0
#define SNP_BETA 0

// Uncomment this setting in a release build
#define RELEASEBUILD


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
extern debugflag XFILES;
extern debugflag CNF_DEBUG;
extern debugflag OBJ_DEBUG;
extern debugflag DEF_DEBUG;
extern debugflag MIMIC_UAE;

// Runloop
extern debugflag RUN_DEBUG;
extern debugflag WARP_DEBUG;
extern debugflag QUEUE_DEBUG;
extern debugflag SNP_DEBUG;

// CPU
extern debugflag CPU_DEBUG;
extern debugflag CST_DEBUG;

// Memory access
extern debugflag OCSREG_DEBUG;
extern debugflag ECSREG_DEBUG;
extern debugflag INVREG_DEBUG;
extern debugflag MEM_DEBUG;

// Agnus
extern debugflag DMA_DEBUG;
extern debugflag DDF_DEBUG;
extern debugflag SEQ_DEBUG;
extern debugflag NTSC_DEBUG;

// Copper
extern debugflag COP_CHECKSUM;
extern debugflag COPREG_DEBUG;
extern debugflag COP_DEBUG;

// Blitter
extern debugflag BLT_CHECKSUM;
extern debugflag BLTREG_DEBUG;
extern debugflag BLT_REG_GUARD;
extern debugflag BLT_MEM_GUARD;
extern debugflag BLT_DEBUG;
extern debugflag BLTTIM_DEBUG;
extern debugflag SLOW_BLT_DEBUG;
extern debugflag OLD_LINE_BLIT;

// Denise
extern debugflag BPLREG_DEBUG;
extern debugflag BPLDAT_DEBUG;
extern debugflag BPLMOD_DEBUG;
extern debugflag SPRREG_DEBUG;
extern debugflag COLREG_DEBUG;
extern debugflag CLXREG_DEBUG;
extern debugflag BPL_DEBUG;
extern debugflag DIW_DEBUG;
extern debugflag SPR_DEBUG;
extern debugflag CLX_DEBUG;
extern debugflag BORDER_DEBUG;

// Paula
extern debugflag INTREG_DEBUG;
extern debugflag INT_DEBUG;

// CIAs
extern debugflag CIA_ON_STEROIDS;
extern debugflag CIAREG_DEBUG;
extern debugflag CIASER_DEBUG;
extern debugflag CIA_DEBUG;
extern debugflag TOD_DEBUG;

// Floppy Drives
extern debugflag ALIGN_HEAD;
extern debugflag DSK_CHECKSUM;
extern debugflag DSKREG_DEBUG;
extern debugflag DSK_DEBUG;
extern debugflag MFM_DEBUG;
extern debugflag FS_DEBUG;

// Hard Drives
extern debugflag HDR_ACCEPT_ALL;
extern debugflag HDR_FS_LOAD_ALL;
extern debugflag WT_DEBUG;

// Audio
extern debugflag AUDREG_DEBUG;
extern debugflag AUD_DEBUG;
extern debugflag AUDBUF_DEBUG;
extern debugflag DISABLE_AUDIRQ;

// Ports
extern debugflag POSREG_DEBUG;
extern debugflag JOYREG_DEBUG;
extern debugflag POTREG_DEBUG;
extern debugflag PRT_DEBUG;
extern debugflag SER_DEBUG;
extern debugflag POT_DEBUG;
extern debugflag HOLD_MOUSE_L;
extern debugflag HOLD_MOUSE_M;
extern debugflag HOLD_MOUSE_R;

// Expansion boards
extern debugflag ZOR_DEBUG;
extern debugflag ACF_DEBUG;
extern debugflag FAS_DEBUG;
extern debugflag HDR_DEBUG;
extern debugflag DBD_DEBUG;

// Media types
extern debugflag ADF_DEBUG;
extern debugflag DMS_DEBUG;
extern debugflag IMG_DEBUG;

// Other components
extern debugflag RTC_DEBUG;
extern debugflag KBD_DEBUG;

// Misc
extern debugflag REC_DEBUG;
extern debugflag SCK_DEBUG;
extern debugflag SRV_DEBUG;
extern debugflag GDB_DEBUG;


//
// Forced error conditions
//

extern debugflag FORCE_ROM_MISSING;
extern debugflag FORCE_CHIP_RAM_MISSING;
extern debugflag FORCE_AROS_NO_EXTROM;
extern debugflag FORCE_AROS_RAM_LIMIT;
extern debugflag FORCE_CHIP_RAM_LIMIT;
extern debugflag FORCE_SNAP_TOO_OLD;
extern debugflag FORCE_SNAP_TOO_NEW;
extern debugflag FORCE_SNAP_IS_BETA;
extern debugflag FORCE_SNAP_CORRUPTED;
extern debugflag FORCE_DISK_INVALID_LAYOUT;
extern debugflag FORCE_DISK_MODIFIED;
extern debugflag FORCE_HDR_TOO_LARGE;
extern debugflag FORCE_HDR_UNSUPPORTED_C;
extern debugflag FORCE_HDR_UNSUPPORTED_H;
extern debugflag FORCE_HDR_UNSUPPORTED_S;
extern debugflag FORCE_HDR_UNSUPPORTED_B;
extern debugflag FORCE_HDR_UNKNOWN_GEOMETRY;
extern debugflag FORCE_HDR_MODIFIED;
extern debugflag FORCE_FS_WRONG_BSIZE;
extern debugflag FORCE_FS_WRONG_CAPACITY;
extern debugflag FORCE_FS_WRONG_DOS_TYPE;
extern debugflag FORCE_DMS_CANT_CREATE;
extern debugflag FORCE_RECORDING_ERROR;
extern debugflag FORCE_NO_FFMPEG;

#include <assert.h>
