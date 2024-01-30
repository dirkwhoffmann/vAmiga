// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"

// General
debugflag XFILES          = 0; // Report paranormal activity
debugflag CNF_DEBUG       = 0; // Configuration options
debugflag OBJ_DEBUG       = 0; // Object life-times
debugflag DEF_DEBUG       = 0; // User defaults
debugflag MIMIC_UAE       = 0; // Enable to compare debug logs with UAE

// Runloop
debugflag RUN_DEBUG       = 0; // Run loop, component states
debugflag TIM_DEBUG       = 0; // Thread synchronization
debugflag WARP_DEBUG      = 0; // Warp mode
debugflag QUEUE_DEBUG     = 0; // Message queue
debugflag SNP_DEBUG       = 0; // Serialization (snapshots)

// CPU
debugflag CPU_DEBUG       = 0; // CPU
debugflag CST_DEBUG       = 0; // Call stack recording

// Memory access
debugflag OCSREG_DEBUG    = 0; // General OCS register debugging
debugflag ECSREG_DEBUG    = 0; // Special ECS register debugging
debugflag INVREG_DEBUG    = 0; // Invalid register accesses
debugflag MEM_DEBUG       = 0; // Memory

// Agnus
debugflag DMA_DEBUG       = 0; // DMA registers
debugflag DDF_DEBUG       = 0; // Display data fetch
debugflag SEQ_DEBUG       = 0; // Bitplane sequencer
debugflag NTSC_DEBUG      = 0; // NTSC mode

// Copper
debugflag COP_CHECKSUM    = 0; // Compute Copper checksums
debugflag COPREG_DEBUG    = 0; // Copper registers
debugflag COP_DEBUG       = 0; // Copper execution

// Blitter
debugflag BLT_CHECKSUM    = 0; // Compute Blitter checksums
debugflag BLTREG_DEBUG    = 0; // Blitter registers
debugflag BLT_REG_GUARD   = 0; // Guard registers while Blitter runs
debugflag BLT_MEM_GUARD   = 0; // Guard memory while Blitter runs
debugflag BLT_DEBUG       = 0; // Blitter execution
debugflag BLTTIM_DEBUG    = 0; // Blitter Timing
debugflag SLOW_BLT_DEBUG  = 0; // Execute micro-instructions in one chunk
debugflag OLD_LINE_BLIT   = 0; // Use the old line blitter implementation

// Denise
debugflag BPLREG_DEBUG    = 0; // Bitplane registers
debugflag BPLDAT_DEBUG    = 0; // BPLxDAT registers
debugflag BPLMOD_DEBUG    = 0; // BPLxMOD registers
debugflag SPRREG_DEBUG    = 0; // Sprite registers
debugflag COLREG_DEBUG    = 0; // Color registers
debugflag CLXREG_DEBUG    = 0; // Collision detection registers
debugflag BPL_DEBUG       = 0; // Bitplane DMA
debugflag DIW_DEBUG       = 0; // Display window
debugflag SPR_DEBUG       = 0; // Sprites
debugflag CLX_DEBUG       = 0; // Collision detection
debugflag BORDER_DEBUG    = 0; // Draw the border in debug colors

// Paula
debugflag INTREG_DEBUG    = 0; // Interrupt registers
debugflag INT_DEBUG       = 0; // Interrupt logic

// CIAs
debugflag CIAREG_DEBUG    = 0; // CIA registers
debugflag CIASER_DEBUG    = 0; // CIA serial register
debugflag CIA_DEBUG       = 0; // CIA execution
debugflag TOD_DEBUG       = 0; // TODs (CIA 24-bit counters)

// Floppy Drives
debugflag ALIGN_HEAD      = 0; // Make head movement deterministic
debugflag DSK_CHECKSUM    = 0; // Compute disk checksums
debugflag DSKREG_DEBUG    = 0; // Disk controller registers
debugflag DSK_DEBUG       = 0; // Disk controller execution
debugflag MFM_DEBUG       = 0; // Disk encoder / decoder
debugflag FS_DEBUG        = 0; // File System Classes (OFS / FFS)

// Hard Drives
debugflag HDR_ACCEPT_ALL  = 0; // Disables hard drive layout checks
debugflag HDR_FS_LOAD_ALL = 0; // Don't filter out unneeded file systems
debugflag WT_DEBUG        = 0; // Write-through mode

// Audio
debugflag AUDREG_DEBUG    = 0; // Audio registers
debugflag AUD_DEBUG       = 0; // Audio execution
debugflag AUDBUF_DEBUG    = 0; // Audio buffers
debugflag DISABLE_AUDIRQ  = 0; // Disable audio interrupts

// Ports
debugflag POSREG_DEBUG    = 0; // POSxxx registers
debugflag JOYREG_DEBUG    = 0; // JOYxxx registers
debugflag POTREG_DEBUG    = 0; // POTxxx registers
debugflag PRT_DEBUG       = 0; // Control ports and connected devices
debugflag SER_DEBUG       = 0; // Serial interface
debugflag POT_DEBUG       = 0; // Potentiometer inputs
debugflag HOLD_MOUSE_L    = 0; // Hold down the left mouse button
debugflag HOLD_MOUSE_M    = 0; // Hold down the middle mouse button
debugflag HOLD_MOUSE_R    = 0; // Hold down the right mouse button

// Expansion boards
debugflag ZOR_DEBUG       = 0; // Zorro space
debugflag ACF_DEBUG       = 0; // Autoconfig
debugflag FAS_DEBUG       = 0; // FastRam
debugflag HDR_DEBUG       = 0; // HardDrive
debugflag DBD_DEBUG       = 0; // DebugBoard

// Media types
debugflag ADF_DEBUG       = 0; // ADF and extended ADF files
debugflag DMS_DEBUG       = 0; // DMS files
debugflag IMG_DEBUG       = 0; // IMG files

// Other components
debugflag RTC_DEBUG       = 0; // Real-time clock
debugflag KBD_DEBUG       = 0; // Keyboard

// Misc
debugflag REC_DEBUG       = 0; // Screen recorder
debugflag SCK_DEBUG       = 0; // Sockets
debugflag SRV_DEBUG       = 0; // Remote server
debugflag GDB_DEBUG       = 0; // GDB server


//
// Forced error conditions
//

debugflag FORCE_ROM_MISSING              = 0;
debugflag FORCE_CHIP_RAM_MISSING         = 0;
debugflag FORCE_AROS_NO_EXTROM           = 0;
debugflag FORCE_AROS_RAM_LIMIT           = 0;
debugflag FORCE_CHIP_RAM_LIMIT           = 0;
debugflag FORCE_SNAP_TOO_OLD             = 0;
debugflag FORCE_SNAP_TOO_NEW             = 0;
debugflag FORCE_SNAP_IS_BETA             = 0;
debugflag FORCE_SNAP_CORRUPTED           = 0;
debugflag FORCE_DISK_INVALID_LAYOUT      = 0;
debugflag FORCE_DISK_MODIFIED            = 0;
debugflag FORCE_HDR_TOO_LARGE            = 0;
debugflag FORCE_HDR_UNSUPPORTED_C        = 0;
debugflag FORCE_HDR_UNSUPPORTED_H        = 0;
debugflag FORCE_HDR_UNSUPPORTED_S        = 0;
debugflag FORCE_HDR_UNSUPPORTED_B        = 0;
debugflag FORCE_HDR_UNKNOWN_GEOMETRY     = 0;
debugflag FORCE_HDR_MODIFIED             = 0;
debugflag FORCE_FS_WRONG_BSIZE           = 0;
debugflag FORCE_FS_WRONG_CAPACITY        = 0;
debugflag FORCE_FS_WRONG_DOS_TYPE        = 0;
debugflag FORCE_DMS_CANT_CREATE          = 0;
debugflag FORCE_RECORDING_ERROR          = 0;
debugflag FORCE_NO_FFMPEG                = 0;
