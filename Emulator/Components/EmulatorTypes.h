// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Reflection.h"
#include "AmigaTypes.h"

namespace vamiga {

//
// Enumerations
//

enum_long(DEBUG_FLAG)
{
    // General
    FLAG_XFILES,           ///< Report paranormal activity
    FLAG_CNF_DEBUG,        ///< Configuration options
    FLAG_OBJ_DEBUG,        ///< Object life-times
    FLAG_DEF_DEBUG,        ///< User defaults
    FLAG_MIMIC_UAE,        ///< Enable to compare debug logs with UAE

    // Runloop
    FLAG_RUN_DEBUG,        ///< Run loop, component states
    FLAG_TIM_DEBUG,        ///< Thread synchronization
    FLAG_WARP_DEBUG,       ///< Warp mode
    FLAG_QUEUE_DEBUG,      ///< Message queue
    FLAG_SNP_DEBUG,        ///< Serialization (snapshots)

    // CPU
    FLAG_CPU_DEBUG,        ///< CPU
    FLAG_CST_DEBUG,        ///< Call stack recording

    // Memory access
    FLAG_OCSREG_DEBUG,     ///< General OCS register debugging
    FLAG_ECSREG_DEBUG,     ///< Special ECS register debugging
    FLAG_INVREG_DEBUG,     ///< Invalid register accesses
    FLAG_MEM_DEBUG,        ///< Memory

    // Agnus
    FLAG_DMA_DEBUG,        ///< DMA registers
    FLAG_DDF_DEBUG,        ///< Display data fetch
    FLAG_SEQ_DEBUG,        ///< Bitplane sequencer
    FLAG_NTSC_DEBUG,       ///< NTSC mode

    // Copper
    FLAG_COP_CHECKSUM,     ///< Compute Copper checksums
    FLAG_COPREG_DEBUG,     ///< Copper registers
    FLAG_COP_DEBUG,        ///< Copper execution

    // Blitter
    FLAG_BLT_CHECKSUM,     ///< Compute Blitter checksums
    FLAG_BLTREG_DEBUG,     ///< Blitter registers
    FLAG_BLT_REG_GUARD,    ///< Guard registers while Blitter runs
    FLAG_BLT_MEM_GUARD,    ///< Guard memory while Blitter runs
    FLAG_BLT_DEBUG,        ///< Blitter execution
    FLAG_BLTTIM_DEBUG,     ///< Blitter Timing
    FLAG_SLOW_BLT_DEBUG,   ///< Execute micro-instructions in one chunk
    FLAG_OLD_LINE_BLIT,    ///< Use the old line blitter implementation

    // Denise
    FLAG_BPLREG_DEBUG,     ///< Bitplane registers
    FLAG_BPLDAT_DEBUG,     ///< BPLxDAT registers
    FLAG_BPLMOD_DEBUG,     ///< BPLxMOD registers
    FLAG_SPRREG_DEBUG,     ///< Sprite registers
    FLAG_COLREG_DEBUG,     ///< Color registers
    FLAG_CLXREG_DEBUG,     ///< Collision detection registers
    FLAG_BPL_DEBUG,        ///< Bitplane DMA
    FLAG_DIW_DEBUG,        ///< Display window
    FLAG_SPR_DEBUG,        ///< Sprites
    FLAG_CLX_DEBUG,        ///< Collision detection
    FLAG_BORDER_DEBUG,     ///< Draw the border in debug colors

    // Paula
    FLAG_INTREG_DEBUG,     ///< Interrupt registers
    FLAG_INT_DEBUG,        ///< Interrupt logic

    // CIAs
    FLAG_CIAREG_DEBUG,     ///< CIA registers
    FLAG_CIASER_DEBUG,     ///< CIA serial register
    FLAG_CIA_DEBUG,        ///< CIA execution
    FLAG_TOD_DEBUG,        ///< TODs (CIA 24-bit counters)

    // Floppy Drives
    FLAG_ALIGN_HEAD,       ///< Make head movement deterministic
    FLAG_DSK_CHECKSUM,     ///< Compute disk checksums
    FLAG_DSKREG_DEBUG,     ///< Disk controller registers
    FLAG_DSK_DEBUG,        ///< Disk controller execution
    FLAG_MFM_DEBUG,        ///< Disk encoder / decoder
    FLAG_FS_DEBUG,         ///< File System Classes (OFS / FFS)

    // Hard Drives
    FLAG_HDR_ACCEPT_ALL,   ///< Disables hard drive layout checks
    FLAG_HDR_FS_LOAD_ALL,  ///< Don't filter out unneeded file systems
    FLAG_WT_DEBUG,         ///< Write-through mode

    // Audio
    FLAG_AUDREG_DEBUG,     ///< Audio registers
    FLAG_AUD_DEBUG,        ///< Audio execution
    FLAG_AUDBUF_DEBUG,     ///< Audio buffers
    FLAG_DISABLE_AUDIRQ,   ///< Disable audio interrupts

    // Ports
    FLAG_POSREG_DEBUG,     ///< POSxxx registers
    FLAG_JOYREG_DEBUG,     ///< JOYxxx registers
    FLAG_POTREG_DEBUG,     ///< POTxxx registers
    FLAG_PRT_DEBUG,        ///< Control ports and connected devices
    FLAG_SER_DEBUG,        ///< Serial interface
    FLAG_POT_DEBUG,        ///< Potentiometer inputs
    FLAG_HOLD_MOUSE_L,     ///< Hold down the left mouse button
    FLAG_HOLD_MOUSE_M,     ///< Hold down the middle mouse button
    FLAG_HOLD_MOUSE_R,     ///< Hold down the right mouse button

    // Expansion boards
    FLAG_ZOR_DEBUG,        ///< Zorro space
    FLAG_ACF_DEBUG,        ///< Autoconfig
    FLAG_FAS_DEBUG,        ///< FastRam
    FLAG_HDR_DEBUG,        ///< HardDrive
    FLAG_DBD_DEBUG,        ///< DebugBoard

    // Media types
    FLAG_ADF_DEBUG,        ///< ADF and extended ADF files
    FLAG_DMS_DEBUG,        ///< DMS files
    FLAG_IMG_DEBUG,        ///< IMG files, ST files

    // Other components
    FLAG_RTC_DEBUG,        ///< Real-time clock
    FLAG_KBD_DEBUG,        ///< Keyboard

    // Misc
    FLAG_REC_DEBUG,        ///< Screen recorder
    FLAG_SCK_DEBUG,        ///< Sockets
    FLAG_SRV_DEBUG,        ///< Remote server
    FLAG_GDB_DEBUG,        ///< GDB server
};
typedef DEBUG_FLAG DebugFlag;

struct DebugFlagEnum : util::Reflection<DebugFlagEnum, DebugFlag>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = FLAG_GDB_DEBUG;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "FLAG"; }
    static const char *key(long value)
    {
        switch (value) {

                // General
            case FLAG_XFILES:           return "XFILES";
            case FLAG_CNF_DEBUG:        return "CNF_DEBUG";
            case FLAG_OBJ_DEBUG:        return "OBJ_DEBUG";
            case FLAG_DEF_DEBUG:        return "DEF_DEBUG";
            case FLAG_MIMIC_UAE:        return "MIMIC_UAE";

                // Runloop
            case FLAG_RUN_DEBUG:        return "RUN_DEBUG";
            case FLAG_TIM_DEBUG:        return "TIM_DEBUG";
            case FLAG_WARP_DEBUG:       return "WARP_DEBUG";
            case FLAG_QUEUE_DEBUG:      return "QUEUE_DEBUG";
            case FLAG_SNP_DEBUG:        return "SNP_DEBUG";

                // CPU
            case FLAG_CPU_DEBUG:        return "CPU_DEBUG";
            case FLAG_CST_DEBUG:        return "CST_DEBUG";

                // Memory access
            case FLAG_OCSREG_DEBUG:     return "OCSREG_DEBUG";
            case FLAG_ECSREG_DEBUG:     return "ECSREG_DEBUG";
            case FLAG_INVREG_DEBUG:     return "INVREG_DEBUG";
            case FLAG_MEM_DEBUG:        return "MEM_DEBUG";

                // Agnus
            case FLAG_DMA_DEBUG:        return "DMA_DEBUG";
            case FLAG_DDF_DEBUG:        return "DDF_DEBUG";
            case FLAG_SEQ_DEBUG:        return "SEQ_DEBUG";
            case FLAG_NTSC_DEBUG:       return "NTSC_DEBUG";

                // Copper
            case FLAG_COP_CHECKSUM:     return "COP_CHECKSUM";
            case FLAG_COPREG_DEBUG:     return "COPREG_DEBUG";
            case FLAG_COP_DEBUG:        return "COP_DEBUG";

                // Blitter
            case FLAG_BLT_CHECKSUM:     return "BLT_CHECKSUM";
            case FLAG_BLTREG_DEBUG:     return "BLTREG_DEBUG";
            case FLAG_BLT_REG_GUARD:    return "BLT_REG_GUARD";
            case FLAG_BLT_MEM_GUARD:    return "BLT_MEM_GUARD";
            case FLAG_BLT_DEBUG:        return "BLT_DEBUG";
            case FLAG_BLTTIM_DEBUG:     return "BLTTIM_DEBUG";
            case FLAG_SLOW_BLT_DEBUG:   return "SLOW_BLT_DEBUG";
            case FLAG_OLD_LINE_BLIT:    return "OLD_LINE_BLIT";

                // Denise
            case FLAG_BPLREG_DEBUG:     return "BPLREG_DEBUG";
            case FLAG_BPLDAT_DEBUG:     return "BPLDAT_DEBUG";
            case FLAG_BPLMOD_DEBUG:     return "BPLMOD_DEBUG";
            case FLAG_SPRREG_DEBUG:     return "SPRREG_DEBUG";
            case FLAG_COLREG_DEBUG:     return "COLREG_DEBUG";
            case FLAG_CLXREG_DEBUG:     return "CLXREG_DEBUG";
            case FLAG_BPL_DEBUG:        return "BPL_DEBUG";
            case FLAG_DIW_DEBUG:        return "DIW_DEBUG";
            case FLAG_SPR_DEBUG:        return "SPR_DEBUG";
            case FLAG_CLX_DEBUG:        return "CLX_DEBUG";
            case FLAG_BORDER_DEBUG:     return "BORDER_DEBUG";

                // Paula
            case FLAG_INTREG_DEBUG:     return "INTREG_DEBUG";
            case FLAG_INT_DEBUG:        return "INT_DEBUG";

                // CIAs
            case FLAG_CIAREG_DEBUG:     return "CIAREG_DEBUG";
            case FLAG_CIASER_DEBUG:     return "CIASER_DEBUG";
            case FLAG_CIA_DEBUG:        return "CIA_DEBUG";
            case FLAG_TOD_DEBUG:        return "TOD_DEBUG";

                // Floppy Drives
            case FLAG_ALIGN_HEAD:       return "ALIGN_HEAD";
            case FLAG_DSK_CHECKSUM:     return "DSK_CHECKSUM";
            case FLAG_DSKREG_DEBUG:     return "DSKREG_DEBUG";
            case FLAG_DSK_DEBUG:        return "DSK_DEBUG";
            case FLAG_MFM_DEBUG:        return "MFM_DEBUG";
            case FLAG_FS_DEBUG:         return "FS_DEBUG";

                // Hard Drives
            case FLAG_HDR_ACCEPT_ALL:   return "HDR_ACCEPT_ALL";
            case FLAG_HDR_FS_LOAD_ALL:  return "HDR_FS_LOAD_ALL";
            case FLAG_WT_DEBUG:         return "WT_DEBUG";

                // Audio
            case FLAG_AUDREG_DEBUG:     return "AUDREG_DEBUG";
            case FLAG_AUD_DEBUG:        return "AUD_DEBUG";
            case FLAG_AUDBUF_DEBUG:     return "AUDBUF_DEBUG";
            case FLAG_DISABLE_AUDIRQ:   return "DISABLE_AUDIRQ";

                // Ports
            case FLAG_POSREG_DEBUG:     return "POSREG_DEBUG";
            case FLAG_JOYREG_DEBUG:     return "JOYREG_DEBUG";
            case FLAG_POTREG_DEBUG:     return "POTREG_DEBUG";
            case FLAG_PRT_DEBUG:        return "PRT_DEBUG";
            case FLAG_SER_DEBUG:        return "SER_DEBUG";
            case FLAG_POT_DEBUG:        return "POT_DEBUG";
            case FLAG_HOLD_MOUSE_L:     return "HOLD_MOUSE_L";
            case FLAG_HOLD_MOUSE_M:     return "HOLD_MOUSE_M";
            case FLAG_HOLD_MOUSE_R:     return "HOLD_MOUSE_R";

                // Expansion boards
            case FLAG_ZOR_DEBUG:        return "ZOR_DEBUG";
            case FLAG_ACF_DEBUG:        return "ACF_DEBUG";
            case FLAG_FAS_DEBUG:        return "FAS_DEBUG";
            case FLAG_HDR_DEBUG:        return "HDR_DEBUG";
            case FLAG_DBD_DEBUG:        return "DBD_DEBUG";

                // Media types
            case FLAG_ADF_DEBUG:        return "ADF_DEBUG";
            case FLAG_DMS_DEBUG:        return "DMS_DEBUG";
            case FLAG_IMG_DEBUG:        return "IMG_DEBUG";

                // Other components
            case FLAG_RTC_DEBUG:        return "RTC_DEBUG";
            case FLAG_KBD_DEBUG:        return "KBD_DEBUG";

                // Misc
            case FLAG_REC_DEBUG:        return "REC_DEBUG";
            case FLAG_SCK_DEBUG:        return "SCK_DEBUG";
            case FLAG_SRV_DEBUG:        return "SRV_DEBUG";
            case FLAG_GDB_DEBUG:        return "GDB_DEBUG";
        }
        return "???";
    }

    static const char *help(long value)
    {
        switch (value) {

                // General
            case FLAG_XFILES:           return "Report paranormal activity";
            case FLAG_CNF_DEBUG:        return "Configuration options";
            case FLAG_OBJ_DEBUG:        return "Object life-times";
            case FLAG_DEF_DEBUG:        return "User defaults";
            case FLAG_MIMIC_UAE:        return "Enable to compare debug logs with UAE";

                // Runloop
            case FLAG_RUN_DEBUG:        return "Run loop, component states";
            case FLAG_TIM_DEBUG:        return "Thread synchronization";
            case FLAG_WARP_DEBUG:       return "Warp mode";
            case FLAG_QUEUE_DEBUG:      return "Message queue";
            case FLAG_SNP_DEBUG:        return "Serialization (snapshots)";

                // CPU
            case FLAG_CPU_DEBUG:        return "CPU";
            case FLAG_CST_DEBUG:        return "Call stack recording";

                // Memory access
            case FLAG_OCSREG_DEBUG:     return "General OCS register debugging";
            case FLAG_ECSREG_DEBUG:     return "Special ECS register debugging";
            case FLAG_INVREG_DEBUG:     return "Invalid register accesses";
            case FLAG_MEM_DEBUG:        return "Memory";

                // Agnus
            case FLAG_DMA_DEBUG:        return "DMA registers";
            case FLAG_DDF_DEBUG:        return "Display data fetch";
            case FLAG_SEQ_DEBUG:        return "Bitplane sequencer";
            case FLAG_NTSC_DEBUG:       return "NTSC mode";

                // Copper
            case FLAG_COP_CHECKSUM:     return "Compute Copper checksums";
            case FLAG_COPREG_DEBUG:     return "Copper registers";
            case FLAG_COP_DEBUG:        return "Copper execution";

                // Blitter
            case FLAG_BLT_CHECKSUM:     return "Compute Blitter checksums";
            case FLAG_BLTREG_DEBUG:     return "Blitter registers";
            case FLAG_BLT_REG_GUARD:    return "Guard registers while Blitter runs";
            case FLAG_BLT_MEM_GUARD:    return "Guard memory while Blitter runs";
            case FLAG_BLT_DEBUG:        return "Blitter execution";
            case FLAG_BLTTIM_DEBUG:     return "Blitter Timing";
            case FLAG_SLOW_BLT_DEBUG:   return "Execute micro-instructions in one chunk";
            case FLAG_OLD_LINE_BLIT:    return "Use the old line blitter implementation";

                // Denise
            case FLAG_BPLREG_DEBUG:     return "Bitplane registers";
            case FLAG_BPLDAT_DEBUG:     return "BPLxDAT registers";
            case FLAG_BPLMOD_DEBUG:     return "BPLxMOD registers";
            case FLAG_SPRREG_DEBUG:     return "Sprite registers";
            case FLAG_COLREG_DEBUG:     return "Color registers";
            case FLAG_CLXREG_DEBUG:     return "Collision detection registers";
            case FLAG_BPL_DEBUG:        return "Bitplane DMA";
            case FLAG_DIW_DEBUG:        return "Display window";
            case FLAG_SPR_DEBUG:        return "Sprites";
            case FLAG_CLX_DEBUG:        return "Collision detection";
            case FLAG_BORDER_DEBUG:     return "Draw the border in debug colors";

                // Paula
            case FLAG_INTREG_DEBUG:     return "Interrupt registers";
            case FLAG_INT_DEBUG:        return "Interrupt logic";

                // CIAs
            case FLAG_CIAREG_DEBUG:     return "CIA registers";
            case FLAG_CIASER_DEBUG:     return "CIA serial register";
            case FLAG_CIA_DEBUG:        return "CIA execution";
            case FLAG_TOD_DEBUG:        return "TODs (CIA 24-bit counters)";

                // Floppy Drives
            case FLAG_ALIGN_HEAD:       return "Make head movement deterministic";
            case FLAG_DSK_CHECKSUM:     return "Compute disk checksums";
            case FLAG_DSKREG_DEBUG:     return "Disk controller registers";
            case FLAG_DSK_DEBUG:        return "Disk controller execution";
            case FLAG_MFM_DEBUG:        return "Disk encoder / decoder";
            case FLAG_FS_DEBUG:         return "File System Classes (OFS / FFS)";

                // Hard Drives
            case FLAG_HDR_ACCEPT_ALL:   return "Disables hard drive layout checks";
            case FLAG_HDR_FS_LOAD_ALL:  return "Don't filter out unneeded file systems";
            case FLAG_WT_DEBUG:         return "Write-through mode";

                // Audio
            case FLAG_AUDREG_DEBUG:     return "Audio registers";
            case FLAG_AUD_DEBUG:        return "Audio execution";
            case FLAG_AUDBUF_DEBUG:     return "Audio buffers";
            case FLAG_DISABLE_AUDIRQ:   return "Disable audio interrupts";

                // Ports
            case FLAG_POSREG_DEBUG:     return "POSxxx registers";
            case FLAG_JOYREG_DEBUG:     return "JOYxxx registers";
            case FLAG_POTREG_DEBUG:     return "POTxxx registers";
            case FLAG_PRT_DEBUG:        return "Control ports and connected devices";
            case FLAG_SER_DEBUG:        return "Serial interface";
            case FLAG_POT_DEBUG:        return "Potentiometer inputs";
            case FLAG_HOLD_MOUSE_L:     return "Hold down the left mouse button";
            case FLAG_HOLD_MOUSE_M:     return "Hold down the middle mouse button";
            case FLAG_HOLD_MOUSE_R:     return "Hold down the right mouse button";

                // Expansion boards
            case FLAG_ZOR_DEBUG:        return "Zorro space";
            case FLAG_ACF_DEBUG:        return "Autoconfig";
            case FLAG_FAS_DEBUG:        return "FastRam";
            case FLAG_HDR_DEBUG:        return "HardDrive";
            case FLAG_DBD_DEBUG:        return "DebugBoard";

                // Media types
            case FLAG_ADF_DEBUG:        return "ADF and extended ADF files";
            case FLAG_DMS_DEBUG:        return "DMS files";
            case FLAG_IMG_DEBUG:        return "IMG files, ST files";

                // Other components
            case FLAG_RTC_DEBUG:        return "Real-time clock";
            case FLAG_KBD_DEBUG:        return "Keyboard";

                // Misc
            case FLAG_REC_DEBUG:        return "Screen recorder";
            case FLAG_SCK_DEBUG:        return "Sockets";
            case FLAG_SRV_DEBUG:        return "Remote server";
            case FLAG_GDB_DEBUG:        return "GDB server";
        }
        return "???";
    }
};

//
// Structures
//

//! The current emulator state
typedef struct
{
    ExecState state;        ///< The current emulator state
    isize refreshRate;      ///< Screen refresh rate of the virtual C64
    bool powered;           ///< Indicates if the emulator is powered on
    bool paused;            ///< Indicates if emulation is paused
    bool running;           ///< Indicates if the emulator is running
    bool suspended;         ///< Indicates if the emulator is in suspended state
    bool warping;           ///< Indicates if warp mode is currently on
    bool tracking;          ///< Indicates if track mode is enabled
}
EmulatorInfo;

//! Collected run-time data
typedef struct
{
    double cpuLoad;         ///< Measured CPU load
    double fps;             ///< Measured frames per seconds
    isize resyncs;          ///< Number of out-of-sync conditions
}
EmulatorStats;

}
