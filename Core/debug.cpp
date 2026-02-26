// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "utl/abilities/Loggable.h"

#define STR(x) #x
#define XSTR(x) STR(x)

#define DEBUG_CHANNEL(name, description) \
LogChannel name = \
  ::utl::debug::name ? \
    Loggable::subscribe(XSTR(name), std::optional<long>(7), description) : \
    Loggable::subscribe(XSTR(name), std::optional<long>(std::nullopt), description);

namespace utl::channel {

// Register the default channels

DEBUG_CHANNEL(NULLDEV,          "Message sink");
DEBUG_CHANNEL(STDERR,           "Standard error");

// Register a logging channel for each debug flag

// General
DEBUG_CHANNEL(XFILES,           "Report paranormal activity")
DEBUG_CHANNEL(CNF_DEBUG,        "Configuration options")
DEBUG_CHANNEL(OBJ_DEBUG,        "Object life-times")
DEBUG_CHANNEL(DEF_DEBUG,        "User defaults")
DEBUG_CHANNEL(MIMIC_UAE,        "Deprecated")

// Runloop
DEBUG_CHANNEL(RUN_DEBUG,        "Run loop, component states");
DEBUG_CHANNEL(TIM_DEBUG,        "Thread synchronization");
DEBUG_CHANNEL(WARP_DEBUG,       "Warp mode");
DEBUG_CHANNEL(CMD_DEBUG,        "Command queue");
DEBUG_CHANNEL(MSG_DEBUG,        "Message queue");
DEBUG_CHANNEL(SNP_DEBUG,        "Serialization (snapshots)");

// Run ahead
DEBUG_CHANNEL(RUA_DEBUG,        "Run-ahead activit");
DEBUG_CHANNEL(RUA_CHECKSUM,     "Run-ahead instance integrity");
DEBUG_CHANNEL(RUA_ON_STEROIDS,  "Update RUA instance every frame");

// CPU
DEBUG_CHANNEL(CPU_DEBUG,        "CPU");

// Memory access
DEBUG_CHANNEL(OCSREG_DEBUG,     "General OCS register debugging");
DEBUG_CHANNEL(ECSREG_DEBUG,     "Special ECS register debugging");
DEBUG_CHANNEL(INVREG_DEBUG,     "Invalid register accesses");
DEBUG_CHANNEL(MEM_DEBUG,        "Memory");

// Agnus
DEBUG_CHANNEL(DMA_DEBUG,        "DMA registers");
DEBUG_CHANNEL(DDF_DEBUG,        "Display data fetch");
DEBUG_CHANNEL(SEQ_DEBUG,        "Bitplane sequencer");
DEBUG_CHANNEL(SEQ_ON_STEROIDS,  "Disable sequencer fast-paths");
DEBUG_CHANNEL(NTSC_DEBUG,       "NTSC mode");

// Copper
DEBUG_CHANNEL(COP_CHECKSUM,     "Compute Copper checksums");
DEBUG_CHANNEL(COPREG_DEBUG,     "Copper registers");
DEBUG_CHANNEL(COP_DEBUG,        "Copper execution");

// Blitter
DEBUG_CHANNEL(BLT_CHECKSUM,     "Compute Blitter checksums");
DEBUG_CHANNEL(BLTREG_DEBUG,     "Blitter registers");
DEBUG_CHANNEL(BLT_REG_GUARD,    "Guard registers while Blitter runs");
DEBUG_CHANNEL(BLT_MEM_GUARD,    "Guard memory while Blitter runs");
DEBUG_CHANNEL(BLT_DEBUG,        "Blitter execution");
DEBUG_CHANNEL(BLTTIM_DEBUG,     "Blitter Timing");
DEBUG_CHANNEL(SLOW_BLT_DEBUG,   "Execute micro-instructions in one chunk");

// Denise
DEBUG_CHANNEL(BPLREG_DEBUG,     "Bitplane registers");
DEBUG_CHANNEL(BPLDAT_DEBUG,     "BPLxDAT registers");
DEBUG_CHANNEL(BPLMOD_DEBUG,     "BPLxMOD registers");
DEBUG_CHANNEL(SPRREG_DEBUG,     "Sprite registers");
DEBUG_CHANNEL(COLREG_DEBUG,     "Color registers");
DEBUG_CHANNEL(CLXREG_DEBUG,     "Collision detection registers");
DEBUG_CHANNEL(BPL_ON_STEROIDS,  "Disable drawing fast-paths");
DEBUG_CHANNEL(DIW_DEBUG,        "Display window");
DEBUG_CHANNEL(SPR_DEBUG,        "Sprites");
DEBUG_CHANNEL(CLX_DEBUG,        "Collision detection");
DEBUG_CHANNEL(BORDER_DEBUG,     "Draw the border in debug colors");
DEBUG_CHANNEL(LINE_DEBUG,       "Draw a certain line in debug color");

// Paula
DEBUG_CHANNEL(INTREG_DEBUG,     "Interrupt registers");
DEBUG_CHANNEL(INT_DEBUG,        "Interrupt logic");

// CIAs
DEBUG_CHANNEL(CIAREG_DEBUG,     "CIA registers");
DEBUG_CHANNEL(CIASER_DEBUG,     "CIA serial register");
DEBUG_CHANNEL(CIA_DEBUG,        "CIA execution");
DEBUG_CHANNEL(TOD_DEBUG,        "TODs (CIA 24-bit counters)");

// Floppy Drives
DEBUG_CHANNEL(ALIGN_HEAD,       "Make head movement deterministic");
DEBUG_CHANNEL(DSK_CHECKSUM,     "Compute disk checksums");
DEBUG_CHANNEL(DSKREG_DEBUG,     "Disk controller registers");
DEBUG_CHANNEL(DSK_DEBUG,        "Disk controller execution");
DEBUG_CHANNEL(MFM_DEBUG,        "Disk encoder / decoder");
DEBUG_CHANNEL(FS_DEBUG,         "File System Classes (OFS / FFS)");

// Hard Drives
DEBUG_CHANNEL(HDR_ACCEPT_ALL,   "Disables hard drive layout checks");
DEBUG_CHANNEL(HDR_FS_LOAD_ALL,  "Don't filter out unneeded file systems");
DEBUG_CHANNEL(WT_DEBUG,         "Write-through mode");

// Audio
DEBUG_CHANNEL(AUDREG_DEBUG,     "Audio registers");
DEBUG_CHANNEL(AUD_DEBUG,        "Audio execution");
DEBUG_CHANNEL(AUDBUF_DEBUG,     "Audio execution");
DEBUG_CHANNEL(AUDVOL_DEBUG,     "Audio execution");
DEBUG_CHANNEL(DISABLE_AUDIRQ,   "Audio execution");

// Ports
DEBUG_CHANNEL(POSREG_DEBUG,     "Audio execution");
DEBUG_CHANNEL(JOYREG_DEBUG,     "Audio execution");
DEBUG_CHANNEL(POTREG_DEBUG,     "Audio execution");
DEBUG_CHANNEL(VID_DEBUG,        "Audio execution");
DEBUG_CHANNEL(PRT_DEBUG,        "Audio execution");
DEBUG_CHANNEL(SER_DEBUG,        "Audio execution");
DEBUG_CHANNEL(POT_DEBUG,        "Audio execution");
DEBUG_CHANNEL(HOLD_MOUSE_L,     "Hold down the left mouse button");
DEBUG_CHANNEL(HOLD_MOUSE_M,     "Hold down the middle mouse button");
DEBUG_CHANNEL(HOLD_MOUSE_R,     "Hold down the right mouse button");

// Expansion boards
DEBUG_CHANNEL(ZOR_DEBUG,        "Zorro space");
DEBUG_CHANNEL(ACF_DEBUG,        "Autoconfig");
DEBUG_CHANNEL(FAS_DEBUG,        "FastRam");
DEBUG_CHANNEL(HDR_DEBUG,        "HardDrive");
DEBUG_CHANNEL(DBD_DEBUG,        "DebugBoard");

// Image files
DEBUG_CHANNEL(IMG_DEBUG,        "Disk images");
DEBUG_CHANNEL(HDF_DEBUG,        "HDF and HDZ files");
DEBUG_CHANNEL(DMS_DEBUG,        "DMS files");

// Other components
DEBUG_CHANNEL(RTC_DEBUG,        "Real-time clock");
DEBUG_CHANNEL(KBD_DEBUG,        "Keyboard");
DEBUG_CHANNEL(KEY_DEBUG,        "Keyboard key events");

// Misc
DEBUG_CHANNEL(RSH_DEBUG,        "RetroShell");
DEBUG_CHANNEL(REC_DEBUG,        "Screen recorde");
DEBUG_CHANNEL(SCK_DEBUG,        "Sockets");
DEBUG_CHANNEL(SRV_DEBUG,        "Remote server");
DEBUG_CHANNEL(GDB_DEBUG,        "GDB server");

}
