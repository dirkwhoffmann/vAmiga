// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/ThreadTypes.h"
#include "Components/AmigaTypes.h"

namespace vamiga {

//
// Enumerations
//

enum class DebugFlag
{
    // General
    XFILES,           ///< Report paranormal activity
    CNF_DEBUG,        ///< Configuration options
    OBJ_DEBUG,        ///< Object life-times
    DEF_DEBUG,        ///< User defaults
    MIMIC_UAE,        ///< Enable to compare debug logs with UAE

    // Runloop
    RUN_DEBUG,        ///< Run loop, component states
    TIM_DEBUG,        ///< Thread synchronization
    WARP_DEBUG,       ///< Warp mode
    CMD_DEBUG,        ///< Debug the command queue
    MSG_DEBUG,        ///< Message queue
    SNP_DEBUG,        ///< Serialization (snapshots)

    // Run ahead
    RUA_DEBUG,         ///< Inform about run-ahead activity
    RUA_CHECKSUM,      ///< Verify the integrity of the run-ahead instance
    RUA_ON_STEROIDS,   ///< Update the run-ahead instance in every frame

    // CPU
    CPU_DEBUG,        ///< CPU

    // Memory access
    OCSREG_DEBUG,     ///< General OCS register debugging
    ECSREG_DEBUG,     ///< Special ECS register debugging
    INVREG_DEBUG,     ///< Invalid register accesses
    MEM_DEBUG,        ///< Memory

    // Agnus
    DMA_DEBUG,        ///< DMA registers
    DDF_DEBUG,        ///< Display data fetch
    SEQ_DEBUG,        ///< Bitplane sequencer
    SEQ_ON_STEROIDS,  ///< Disable sequencer fast-paths
    NTSC_DEBUG,       ///< NTSC mode

    // Copper
    COP_CHECKSUM,     ///< Compute Copper checksums
    COPREG_DEBUG,     ///< Copper registers
    COP_DEBUG,        ///< Copper execution

    // Blitter
    BLT_CHECKSUM,     ///< Compute Blitter checksums
    BLTREG_DEBUG,     ///< Blitter registers
    BLT_REG_GUARD,    ///< Guard registers while Blitter runs
    BLT_MEM_GUARD,    ///< Guard memory while Blitter runs
    BLT_DEBUG,        ///< Blitter execution
    BLTTIM_DEBUG,     ///< Blitter Timing
    SLOW_BLT_DEBUG,   ///< Execute micro-instructions in one chunk

    // Denise
    BPLREG_DEBUG,     ///< Bitplane registers
    BPLDAT_DEBUG,     ///< BPLxDAT registers
    BPLMOD_DEBUG,     ///< BPLxMOD registers
    SPRREG_DEBUG,     ///< Sprite registers
    COLREG_DEBUG,     ///< Color registers
    CLXREG_DEBUG,     ///< Collision detection registers
    BPL_ON_STEROIDS,  ///< Disable drawing fast-paths
    DIW_DEBUG,        ///< Display window
    SPR_DEBUG,        ///< Sprites
    CLX_DEBUG,        ///< Collision detection
    BORDER_DEBUG,     ///< Draw the border in debug colors
    LINE_DEBUG,       ///< Draw the specified line in debug colors

    // Paula
    INTREG_DEBUG,     ///< Interrupt registers
    INT_DEBUG,        ///< Interrupt logic

    // CIAs
    CIAREG_DEBUG,     ///< CIA registers
    CIASER_DEBUG,     ///< CIA serial register
    CIA_DEBUG,        ///< CIA execution
    TOD_DEBUG,        ///< TODs (CIA 24-bit counters)

    // Floppy Drives
    ALIGN_HEAD,       ///< Make head movement deterministic
    DSK_CHECKSUM,     ///< Compute disk checksums
    DSKREG_DEBUG,     ///< Disk controller registers
    DSK_DEBUG,        ///< Disk controller execution
    MFM_DEBUG,        ///< Disk encoder / decoder
    FS_DEBUG,         ///< File System Classes (OFS / FFS)

    // Hard Drives
    HDR_ACCEPT_ALL,   ///< Disables hard drive layout checks
    HDR_FS_LOAD_ALL,  ///< Don't filter out unneeded file systems
    WT_DEBUG,         ///< Write-through mode

    // Audio
    AUDREG_DEBUG,     ///< Audio registers
    AUD_DEBUG,        ///< Audio execution
    AUDBUF_DEBUG,     ///< Audio buffers
    AUDVOL_DEBUG,     ///< Audio volume
    DISABLE_AUDIRQ,   ///< Disable audio interrupts

    // Ports
    POSREG_DEBUG,     ///< POSxxx registers
    JOYREG_DEBUG,     ///< JOYxxx registers
    POTREG_DEBUG,     ///< POTxxx registers
    VID_DEBUG,        ///< Video port
    PRT_DEBUG,        ///< Control ports and connected devices
    SER_DEBUG,        ///< Serial interface
    POT_DEBUG,        ///< Potentiometer inputs
    HOLD_MOUSE_L,     ///< Hold down the left mouse button
    HOLD_MOUSE_M,     ///< Hold down the middle mouse button
    HOLD_MOUSE_R,     ///< Hold down the right mouse button

    // Expansion boards
    ZOR_DEBUG,        ///< Zorro space
    ACF_DEBUG,        ///< Autoconfig
    FAS_DEBUG,        ///< FastRam
    HDR_DEBUG,        ///< HardDrive
    DBD_DEBUG,        ///< DebugBoard

    // Media types
    ADF_DEBUG,        ///< ADF, ADZ, and extended ADF files
    HDF_DEBUG,        ///< HDF and HDZ files
    DMS_DEBUG,        ///< DMS files
    IMG_DEBUG,        ///< IMG files, ST files

    // Other components
    RTC_DEBUG,        ///< Real-time clock
    KBD_DEBUG,        ///< Keyboard
    KEY_DEBUG,        ///< Keyboard key events

    // Misc
    RSH_DEBUG,        ///< Retro shell
    REC_DEBUG,        ///< Screen recorder
    SCK_DEBUG,        ///< Sockets
    SRV_DEBUG,        ///< Remote server
    GDB_DEBUG,        ///< GDB server
};

struct DebugFlagEnum : Reflection<DebugFlagEnum, DebugFlag>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DebugFlag::GDB_DEBUG);

    static const char *_key(DebugFlag value)
    {
        switch (value) {

                // General
            case DebugFlag::XFILES:           return "XFILES";
            case DebugFlag::CNF_DEBUG:        return "CNF_DEBUG";
            case DebugFlag::OBJ_DEBUG:        return "OBJ_DEBUG";
            case DebugFlag::DEF_DEBUG:        return "DEF_DEBUG";
            case DebugFlag::MIMIC_UAE:        return "MIMIC_UAE";

                // Runloop
            case DebugFlag::RUN_DEBUG:        return "RUN_DEBUG";
            case DebugFlag::TIM_DEBUG:        return "TIM_DEBUG";
            case DebugFlag::WARP_DEBUG:       return "WARP_DEBUG";
            case DebugFlag::CMD_DEBUG:        return "CMD_DEBUG";
            case DebugFlag::MSG_DEBUG:        return "MSG_DEBUG";
            case DebugFlag::SNP_DEBUG:        return "SNP_DEBUG";

                // Run ahead
            case DebugFlag::RUA_DEBUG:        return "RUA_DEBUG";
            case DebugFlag::RUA_CHECKSUM:     return "RUA_CHECKSUM";
            case DebugFlag::RUA_ON_STEROIDS:  return "RUA_ON_STEROIDS";

                // CPU
            case DebugFlag::CPU_DEBUG:        return "CPU_DEBUG";

                // Memory access
            case DebugFlag::OCSREG_DEBUG:     return "OCSREG_DEBUG";
            case DebugFlag::ECSREG_DEBUG:     return "ECSREG_DEBUG";
            case DebugFlag::INVREG_DEBUG:     return "INVREG_DEBUG";
            case DebugFlag::MEM_DEBUG:        return "MEM_DEBUG";

                // Agnus
            case DebugFlag::DMA_DEBUG:        return "DMA_DEBUG";
            case DebugFlag::DDF_DEBUG:        return "DDF_DEBUG";
            case DebugFlag::SEQ_DEBUG:        return "SEQ_DEBUG";
            case DebugFlag::SEQ_ON_STEROIDS:  return "SEQ_ON_STEROIDS";
            case DebugFlag::NTSC_DEBUG:       return "NTSC_DEBUG";

                // Copper
            case DebugFlag::COP_CHECKSUM:     return "COP_CHECKSUM";
            case DebugFlag::COPREG_DEBUG:     return "COPREG_DEBUG";
            case DebugFlag::COP_DEBUG:        return "COP_DEBUG";

                // Blitter
            case DebugFlag::BLT_CHECKSUM:     return "BLT_CHECKSUM";
            case DebugFlag::BLTREG_DEBUG:     return "BLTREG_DEBUG";
            case DebugFlag::BLT_REG_GUARD:    return "BLT_REG_GUARD";
            case DebugFlag::BLT_MEM_GUARD:    return "BLT_MEM_GUARD";
            case DebugFlag::BLT_DEBUG:        return "BLT_DEBUG";
            case DebugFlag::BLTTIM_DEBUG:     return "BLTTIM_DEBUG";
            case DebugFlag::SLOW_BLT_DEBUG:   return "SLOW_BLT_DEBUG";

                // Denise
            case DebugFlag::BPLREG_DEBUG:     return "BPLREG_DEBUG";
            case DebugFlag::BPLDAT_DEBUG:     return "BPLDAT_DEBUG";
            case DebugFlag::BPLMOD_DEBUG:     return "BPLMOD_DEBUG";
            case DebugFlag::SPRREG_DEBUG:     return "SPRREG_DEBUG";
            case DebugFlag::COLREG_DEBUG:     return "COLREG_DEBUG";
            case DebugFlag::CLXREG_DEBUG:     return "CLXREG_DEBUG";
            case DebugFlag::BPL_ON_STEROIDS:  return "BPL_ON_STEROIDS";
            case DebugFlag::DIW_DEBUG:        return "DIW_DEBUG";
            case DebugFlag::SPR_DEBUG:        return "SPR_DEBUG";
            case DebugFlag::CLX_DEBUG:        return "CLX_DEBUG";
            case DebugFlag::BORDER_DEBUG:     return "BORDER_DEBUG";
            case DebugFlag::LINE_DEBUG:       return "LINE_DEBUG";

                // Paula
            case DebugFlag::INTREG_DEBUG:     return "INTREG_DEBUG";
            case DebugFlag::INT_DEBUG:        return "INT_DEBUG";

                // CIAs
            case DebugFlag::CIAREG_DEBUG:     return "CIAREG_DEBUG";
            case DebugFlag::CIASER_DEBUG:     return "CIASER_DEBUG";
            case DebugFlag::CIA_DEBUG:        return "CIA_DEBUG";
            case DebugFlag::TOD_DEBUG:        return "TOD_DEBUG";

                // Floppy Drives
            case DebugFlag::ALIGN_HEAD:       return "ALIGN_HEAD";
            case DebugFlag::DSK_CHECKSUM:     return "DSK_CHECKSUM";
            case DebugFlag::DSKREG_DEBUG:     return "DSKREG_DEBUG";
            case DebugFlag::DSK_DEBUG:        return "DSK_DEBUG";
            case DebugFlag::MFM_DEBUG:        return "MFM_DEBUG";
            case DebugFlag::FS_DEBUG:         return "FS_DEBUG";

                // Hard Drives
            case DebugFlag::HDR_ACCEPT_ALL:   return "HDR_ACCEPT_ALL";
            case DebugFlag::HDR_FS_LOAD_ALL:  return "HDR_FS_LOAD_ALL";
            case DebugFlag::WT_DEBUG:         return "WT_DEBUG";

                // Audio
            case DebugFlag::AUDREG_DEBUG:     return "AUDREG_DEBUG";
            case DebugFlag::AUD_DEBUG:        return "AUD_DEBUG";
            case DebugFlag::AUDBUF_DEBUG:     return "AUDBUF_DEBUG";
            case DebugFlag::AUDVOL_DEBUG:     return "AUDVOL_DEBUG";
            case DebugFlag::DISABLE_AUDIRQ:   return "DISABLE_AUDIRQ";

                // Ports
            case DebugFlag::POSREG_DEBUG:     return "POSREG_DEBUG";
            case DebugFlag::JOYREG_DEBUG:     return "JOYREG_DEBUG";
            case DebugFlag::POTREG_DEBUG:     return "POTREG_DEBUG";
            case DebugFlag::VID_DEBUG:        return "VID_DEBUG";
            case DebugFlag::PRT_DEBUG:        return "PRT_DEBUG";
            case DebugFlag::SER_DEBUG:        return "SER_DEBUG";
            case DebugFlag::POT_DEBUG:        return "POT_DEBUG";
            case DebugFlag::HOLD_MOUSE_L:     return "HOLD_MOUSE_L";
            case DebugFlag::HOLD_MOUSE_M:     return "HOLD_MOUSE_M";
            case DebugFlag::HOLD_MOUSE_R:     return "HOLD_MOUSE_R";

                // Expansion boards
            case DebugFlag::ZOR_DEBUG:        return "ZOR_DEBUG";
            case DebugFlag::ACF_DEBUG:        return "ACF_DEBUG";
            case DebugFlag::FAS_DEBUG:        return "FAS_DEBUG";
            case DebugFlag::HDR_DEBUG:        return "HDR_DEBUG";
            case DebugFlag::DBD_DEBUG:        return "DBD_DEBUG";

                // Media types
            case DebugFlag::ADF_DEBUG:        return "ADF_DEBUG";
            case DebugFlag::HDF_DEBUG:        return "HDF_DEBUG";
            case DebugFlag::DMS_DEBUG:        return "DMS_DEBUG";
            case DebugFlag::IMG_DEBUG:        return "IMG_DEBUG";

                // Other components
            case DebugFlag::RTC_DEBUG:        return "RTC_DEBUG";
            case DebugFlag::KBD_DEBUG:        return "KBD_DEBUG";
            case DebugFlag::KEY_DEBUG:        return "KEY_DEBUG";

                // Misc
            case DebugFlag::RSH_DEBUG:        return "RSH_DEBUG";
            case DebugFlag::REC_DEBUG:        return "REC_DEBUG";
            case DebugFlag::SCK_DEBUG:        return "SCK_DEBUG";
            case DebugFlag::SRV_DEBUG:        return "SRV_DEBUG";
            case DebugFlag::GDB_DEBUG:        return "GDB_DEBUG";
        }
        return "???";
    }

    static const char *help(DebugFlag value)
    {
        switch (value) {

                // General
            case DebugFlag::XFILES:           return "Report paranormal activity";
            case DebugFlag::CNF_DEBUG:        return "Configuration options";
            case DebugFlag::OBJ_DEBUG:        return "Object life-times";
            case DebugFlag::DEF_DEBUG:        return "User defaults";
            case DebugFlag::MIMIC_UAE:        return "Enable to compare debug logs with UAE";

                // Emulator
            case DebugFlag::RUN_DEBUG:        return "Run loop, component states";
            case DebugFlag::TIM_DEBUG:        return "Thread synchronization";
            case DebugFlag::WARP_DEBUG:       return "Warp mode";
            case DebugFlag::CMD_DEBUG:        return "Command queue";
            case DebugFlag::MSG_DEBUG:        return "Message queue";
            case DebugFlag::SNP_DEBUG:        return "Serialization (snapshots)";

                // Run ahead
            case DebugFlag::RUA_DEBUG:        return "Inform about run-ahead activity";
            case DebugFlag::RUA_CHECKSUM:     return "Verify the integrity of the run-ahead instance";
            case DebugFlag::RUA_ON_STEROIDS:  return "Update the run-ahead instance in every frame";


                // CPU
            case DebugFlag::CPU_DEBUG:        return "CPU";

                // Memory access
            case DebugFlag::OCSREG_DEBUG:     return "General OCS register debugging";
            case DebugFlag::ECSREG_DEBUG:     return "Special ECS register debugging";
            case DebugFlag::INVREG_DEBUG:     return "Invalid register accesses";
            case DebugFlag::MEM_DEBUG:        return "Memory";

                // Agnus
            case DebugFlag::DMA_DEBUG:        return "DMA registers";
            case DebugFlag::DDF_DEBUG:        return "Display data fetch";
            case DebugFlag::SEQ_DEBUG:        return "Bitplane sequencer";
            case DebugFlag::SEQ_ON_STEROIDS:  return "Disable sequencer fast-paths";
            case DebugFlag::NTSC_DEBUG:       return "NTSC mode";

                // Copper
            case DebugFlag::COP_CHECKSUM:     return "Compute Copper checksums";
            case DebugFlag::COPREG_DEBUG:     return "Copper registers";
            case DebugFlag::COP_DEBUG:        return "Copper execution";

                // Blitter
            case DebugFlag::BLT_CHECKSUM:     return "Compute Blitter checksums";
            case DebugFlag::BLTREG_DEBUG:     return "Blitter registers";
            case DebugFlag::BLT_REG_GUARD:    return "Guard registers while Blitter runs";
            case DebugFlag::BLT_MEM_GUARD:    return "Guard memory while Blitter runs";
            case DebugFlag::BLT_DEBUG:        return "Blitter execution";
            case DebugFlag::BLTTIM_DEBUG:     return "Blitter Timing";
            case DebugFlag::SLOW_BLT_DEBUG:   return "Execute micro-instructions in one chunk";

                // Denise
            case DebugFlag::BPLREG_DEBUG:     return "Bitplane registers";
            case DebugFlag::BPLDAT_DEBUG:     return "BPLxDAT registers";
            case DebugFlag::BPLMOD_DEBUG:     return "BPLxMOD registers";
            case DebugFlag::SPRREG_DEBUG:     return "Sprite registers";
            case DebugFlag::COLREG_DEBUG:     return "Color registers";
            case DebugFlag::CLXREG_DEBUG:     return "Collision detection registers";
            case DebugFlag::BPL_ON_STEROIDS:  return "Disable drawing fast-paths";
            case DebugFlag::DIW_DEBUG:        return "Display window";
            case DebugFlag::SPR_DEBUG:        return "Sprites";
            case DebugFlag::CLX_DEBUG:        return "Collision detection";
            case DebugFlag::BORDER_DEBUG:     return "Draw the border in debug colors";
            case DebugFlag::LINE_DEBUG:       return "Draw a certain line in debug color";

                // Paula
            case DebugFlag::INTREG_DEBUG:     return "Interrupt registers";
            case DebugFlag::INT_DEBUG:        return "Interrupt logic";

                // CIAs
            case DebugFlag::CIAREG_DEBUG:     return "CIA registers";
            case DebugFlag::CIASER_DEBUG:     return "CIA serial register";
            case DebugFlag::CIA_DEBUG:        return "CIA execution";
            case DebugFlag::TOD_DEBUG:        return "TODs (CIA 24-bit counters)";

                // Floppy Drives
            case DebugFlag::ALIGN_HEAD:       return "Make head movement deterministic";
            case DebugFlag::DSK_CHECKSUM:     return "Compute disk checksums";
            case DebugFlag::DSKREG_DEBUG:     return "Disk controller registers";
            case DebugFlag::DSK_DEBUG:        return "Disk controller execution";
            case DebugFlag::MFM_DEBUG:        return "Disk encoder / decoder";
            case DebugFlag::FS_DEBUG:         return "File System Classes (OFS / FFS)";

                // Hard Drives
            case DebugFlag::HDR_ACCEPT_ALL:   return "Disables hard drive layout checks";
            case DebugFlag::HDR_FS_LOAD_ALL:  return "Don't filter out unneeded file systems";
            case DebugFlag::WT_DEBUG:         return "Write-through mode";

                // Audio
            case DebugFlag::AUDREG_DEBUG:     return "Audio registers";
            case DebugFlag::AUD_DEBUG:        return "Audio execution";
            case DebugFlag::AUDBUF_DEBUG:     return "Audio buffers";
            case DebugFlag::AUDVOL_DEBUG:     return "Audio volumes";
            case DebugFlag::DISABLE_AUDIRQ:   return "Disable audio interrupts";

                // Ports
            case DebugFlag::POSREG_DEBUG:     return "POSxxx registers";
            case DebugFlag::JOYREG_DEBUG:     return "JOYxxx registers";
            case DebugFlag::POTREG_DEBUG:     return "POTxxx registers";
            case DebugFlag::VID_DEBUG:        return "Video port";
            case DebugFlag::PRT_DEBUG:        return "Control ports and connected devices";
            case DebugFlag::SER_DEBUG:        return "Serial interface";
            case DebugFlag::POT_DEBUG:        return "Potentiometer inputs";
            case DebugFlag::HOLD_MOUSE_L:     return "Hold down the left mouse button";
            case DebugFlag::HOLD_MOUSE_M:     return "Hold down the middle mouse button";
            case DebugFlag::HOLD_MOUSE_R:     return "Hold down the right mouse button";

                // Expansion boards
            case DebugFlag::ZOR_DEBUG:        return "Zorro space";
            case DebugFlag::ACF_DEBUG:        return "Autoconfig";
            case DebugFlag::FAS_DEBUG:        return "FastRam";
            case DebugFlag::HDR_DEBUG:        return "HardDrive";
            case DebugFlag::DBD_DEBUG:        return "DebugBoard";

                // Media types
            case DebugFlag::ADF_DEBUG:        return "ADF, ADZ and extended ADF files";
            case DebugFlag::HDF_DEBUG:        return "HDF and HDZ files";
            case DebugFlag::DMS_DEBUG:        return "DMS files";
            case DebugFlag::IMG_DEBUG:        return "IMG files, ST files";

                // Other components
            case DebugFlag::RTC_DEBUG:        return "Real-time clock";
            case DebugFlag::KBD_DEBUG:        return "Keyboard";
            case DebugFlag::KEY_DEBUG:        return "Keyboard key events";

                // Misc
            case DebugFlag::RSH_DEBUG:        return "RetroShell";
            case DebugFlag::REC_DEBUG:        return "Screen recorder";
            case DebugFlag::SCK_DEBUG:        return "Sockets";
            case DebugFlag::SRV_DEBUG:        return "Remote server";
            case DebugFlag::GDB_DEBUG:        return "GDB server";
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
    isize clones;           ///< Number of created run-ahead instances
}
EmulatorStats;

}
