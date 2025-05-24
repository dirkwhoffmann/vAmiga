// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"

// General
debugflag XFILES          = 0;
debugflag CNF_DEBUG       = 0;
debugflag OBJ_DEBUG       = 0;
debugflag DEF_DEBUG       = 0;
debugflag MIMIC_UAE       = 0;

// Runloop
debugflag RUN_DEBUG       = 0;
debugflag TIM_DEBUG       = 0;
debugflag WARP_DEBUG      = 0;
debugflag CMD_DEBUG       = 0;
debugflag MSG_DEBUG       = 0;
debugflag SNP_DEBUG       = 0;

// Run ahead
debugflag RUA_DEBUG       = 0;
debugflag RUA_CHECKSUM    = 0;
debugflag RUA_ON_STEROIDS = 0;

// CPU
debugflag CPU_DEBUG       = 0;

// Memory access
debugflag OCSREG_DEBUG    = 0;
debugflag ECSREG_DEBUG    = 0;
debugflag INVREG_DEBUG    = 0;
debugflag MEM_DEBUG       = 0;

// Agnus
debugflag DMA_DEBUG       = 0;
debugflag DDF_DEBUG       = 0;
debugflag SEQ_DEBUG       = 0;
debugflag SEQ_ON_STEROIDS = 0;
debugflag NTSC_DEBUG      = 0;

// Copper
debugflag COP_CHECKSUM    = 0;
debugflag COPREG_DEBUG    = 0;
debugflag COP_DEBUG       = 0;

// Blitter
debugflag BLT_CHECKSUM    = 0;
debugflag BLTREG_DEBUG    = 0;
debugflag BLT_REG_GUARD   = 0;
debugflag BLT_MEM_GUARD   = 0;
debugflag BLT_DEBUG       = 0;
debugflag BLTTIM_DEBUG    = 0;
debugflag SLOW_BLT_DEBUG  = 0;

// Denise
debugflag BPLREG_DEBUG    = 0;
debugflag BPLDAT_DEBUG    = 0;
debugflag BPLMOD_DEBUG    = 0;
debugflag SPRREG_DEBUG    = 0;
debugflag COLREG_DEBUG    = 0;
debugflag CLXREG_DEBUG    = 0;
debugflag BPL_ON_STEROIDS = 0;
debugflag DIW_DEBUG       = 0;
debugflag SPR_DEBUG       = 0;
debugflag CLX_DEBUG       = 0;
debugflag BORDER_DEBUG    = 0;
debugflag LINE_DEBUG      = 0;

// Paula
debugflag INTREG_DEBUG    = 0;
debugflag INT_DEBUG       = 0;

// CIAs
debugflag CIAREG_DEBUG    = 0;
debugflag CIASER_DEBUG    = 0;
debugflag CIA_DEBUG       = 0;
debugflag TOD_DEBUG       = 0;

// Floppy Drives
debugflag ALIGN_HEAD      = 0;
debugflag DSK_CHECKSUM    = 0;
debugflag DSKREG_DEBUG    = 0;
debugflag DSK_DEBUG       = 0;
debugflag MFM_DEBUG       = 0;
debugflag FS_DEBUG        = 0;

// Hard Drives
debugflag HDR_ACCEPT_ALL  = 0;
debugflag HDR_FS_LOAD_ALL = 0;
debugflag WT_DEBUG        = 0;

// Audio
debugflag AUDREG_DEBUG    = 0;
debugflag AUD_DEBUG       = 0;
debugflag AUDBUF_DEBUG    = 0;
debugflag AUDVOL_DEBUG    = 0;
debugflag DISABLE_AUDIRQ  = 0;

// Ports
debugflag POSREG_DEBUG    = 0;
debugflag JOYREG_DEBUG    = 0;
debugflag POTREG_DEBUG    = 0;
debugflag VID_DEBUG       = 0;
debugflag PRT_DEBUG       = 0;
debugflag SER_DEBUG       = 0;
debugflag POT_DEBUG       = 0;
debugflag HOLD_MOUSE_L    = 0;
debugflag HOLD_MOUSE_M    = 0;
debugflag HOLD_MOUSE_R    = 0;

// Expansion boards
debugflag ZOR_DEBUG       = 0;
debugflag ACF_DEBUG       = 0;
debugflag FAS_DEBUG       = 0;
debugflag HDR_DEBUG       = 0;
debugflag DBD_DEBUG       = 0;

// Media types
debugflag ADF_DEBUG       = 0;
debugflag HDF_DEBUG       = 0;
debugflag DMS_DEBUG       = 0;
debugflag IMG_DEBUG       = 0;

// Other components
debugflag RTC_DEBUG       = 0;
debugflag KBD_DEBUG       = 0;
debugflag KEY_DEBUG       = 0;

// Misc
debugflag REC_DEBUG       = 0;
debugflag SCK_DEBUG       = 0;
debugflag SRV_DEBUG       = 0;
debugflag GDB_DEBUG       = 0;


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
debugflag FORCE_ZLIB_ERROR               = 0;
