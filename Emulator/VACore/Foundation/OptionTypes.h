// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VACore/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class Opt : long
{
    // Host
    HOST_REFRESH_RATE,      ///< Refresh rate of the host display
    HOST_SAMPLE_RATE,       ///< Refresh rate of the host display
    HOST_FRAMEBUF_WIDTH,    ///< Current width of the emulator window
    HOST_FRAMEBUF_HEIGHT,   ///< Current height of the emulator window
    
    // Amiga
    AMIGA_VIDEO_FORMAT,     ///< Machine type (PAL or NTSC)
    AMIGA_WARP_BOOT,        ///< Warp-boot time in seconds
    AMIGA_WARP_MODE,        ///< Warp activation mode
    AMIGA_VSYNC,            ///< Derive the frame rate to the VSYNC signal
    AMIGA_SPEED_BOOST,      ///< Speed adjustment in percent
    AMIGA_RUN_AHEAD,        ///< Number of run-ahead frames
    
    // Snapshots
    AMIGA_SNAP_AUTO,        ///< Automatically take a snapshots
    AMIGA_SNAP_DELAY,       ///< Delay between two snapshots in seconds
    AMIGA_SNAP_COMPRESSOR,  ///< Snapshot compression method

    // Workspaces
    AMIGA_WS_COMPRESSION,   ///< Workspace media file compression

    // Agnus
    AGNUS_REVISION,
    AGNUS_PTR_DROPS,
    
    // Denise
    DENISE_REVISION,
    DENISE_VIEWPORT_TRACKING,
    DENISE_FRAME_SKIPPING,
    DENISE_HIDDEN_BITPLANES,
    DENISE_HIDDEN_SPRITES,
    DENISE_HIDDEN_LAYERS,
    DENISE_HIDDEN_LAYER_ALPHA,
    DENISE_CLX_SPR_SPR,
    DENISE_CLX_SPR_PLF,
    DENISE_CLX_PLF_PLF,
    
    // Monitor
    MON_PALETTE,
    MON_BRIGHTNESS,
    MON_CONTRAST,
    MON_SATURATION,
    
    // DMA Debugger
    DMA_DEBUG_ENABLE,
    DMA_DEBUG_MODE,
    DMA_DEBUG_OPACITY,
    DMA_DEBUG_CHANNEL0,
    DMA_DEBUG_CHANNEL1,
    DMA_DEBUG_CHANNEL2,
    DMA_DEBUG_CHANNEL3,
    DMA_DEBUG_CHANNEL4,
    DMA_DEBUG_CHANNEL5,
    DMA_DEBUG_CHANNEL6,
    DMA_DEBUG_CHANNEL7,
    DMA_DEBUG_COLOR0,
    DMA_DEBUG_COLOR1,
    DMA_DEBUG_COLOR2,
    DMA_DEBUG_COLOR3,
    DMA_DEBUG_COLOR4,
    DMA_DEBUG_COLOR5,
    DMA_DEBUG_COLOR6,
    DMA_DEBUG_COLOR7,
    
    // Logic analyzer
    LA_PROBE0,              ///< Probe on channel 0
    LA_PROBE1,              ///< Probe on channel 1
    LA_PROBE2,              ///< Probe on channel 2
    LA_PROBE3,              ///< Probe on channel 3
    LA_ADDR0,               ///< Address for channel 0 (memory probing)
    LA_ADDR1,               ///< Address for channel 1 (memory probing)
    LA_ADDR2,               ///< Address for channel 2 (memory probing)
    LA_ADDR3,               ///< Address for channel 3 (memory probing)
    
    // Video port
    VID_WHITE_NOISE,        ///< Generate white-noise when switched off

    // CPU
    CPU_REVISION,
    CPU_DASM_REVISION,
    CPU_DASM_SYNTAX,
    CPU_DASM_NUMBERS,
    CPU_OVERCLOCKING,
    CPU_RESET_VAL,
    
    // Real-time clock
    RTC_MODEL,
    
    // Memory
    MEM_CHIP_RAM,
    MEM_SLOW_RAM,
    MEM_FAST_RAM,
    MEM_EXT_START,
    MEM_SAVE_ROMS,
    MEM_SLOW_RAM_DELAY,
    MEM_SLOW_RAM_MIRROR,
    MEM_BANKMAP,
    MEM_UNMAPPING_TYPE,
    MEM_RAM_INIT_PATTERN,
    
    // Disk controller
    DC_SPEED,
    DC_LOCK_DSKSYNC,
    DC_AUTO_DSKSYNC,
    
    // Floppy Drives
    DRIVE_CONNECT,
    DRIVE_TYPE,
    DRIVE_MECHANICS,
    DRIVE_RPM,
    DRIVE_SWAP_DELAY,
    DRIVE_PAN,
    DRIVE_STEP_VOLUME,
    DRIVE_POLL_VOLUME,
    DRIVE_INSERT_VOLUME,
    DRIVE_EJECT_VOLUME,
    
    // Hard drive controllers
    HDC_CONNECT,
    
    // Hard drives
    HDR_TYPE,
    HDR_PAN,
    HDR_STEP_VOLUME,
    
    // Ports
    SER_DEVICE,
    SER_VERBOSE,
    
    // Blitter
    BLITTER_ACCURACY,
    
    // CIAs
    CIA_REVISION,
    CIA_TODBUG,
    CIA_ECLOCK_SYNCING,
    CIA_IDLE_SLEEP,
    
    // Keyboard
    KBD_ACCURACY,
    
    // Mouse
    MOUSE_PULLUP_RESISTORS,
    MOUSE_SHAKE_DETECTION,
    MOUSE_VELOCITY,
    
    // Joystick
    JOY_AUTOFIRE,           ///< Autofire status [on/off]
    JOY_AUTOFIRE_BURSTS,    ///< Burst mode [on/off]
    JOY_AUTOFIRE_BULLETS,   ///< Number of bullets per burst
    JOY_AUTOFIRE_DELAY,     ///< Delay between two button events [frames]
    
    // Paula audio
    AUD_PAN0,
    AUD_PAN1,
    AUD_PAN2,
    AUD_PAN3,
    AUD_VOL0,
    AUD_VOL1,
    AUD_VOL2,
    AUD_VOL3,
    AUD_VOLL,
    AUD_VOLR,
    AUD_FILTER_TYPE,
    AUD_BUFFER_SIZE,
    AUD_SAMPLING_METHOD,
    AUD_ASR,
    AUD_FASTPATH,
    
    // Expansion boards
    DIAG_BOARD,
    
    // Remote servers
    SRV_PORT,
    SRV_PROTOCOL,
    SRV_AUTORUN,
    SRV_VERBOSE
};

struct OptEnum : Reflection<OptEnum, Opt>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Opt::SRV_VERBOSE);
    
    static const char *_key(Opt value)
    {
        switch (value) {
                
            case Opt::HOST_REFRESH_RATE:         return "HOST.REFRESH_RATE";
            case Opt::HOST_SAMPLE_RATE:          return "HOST.SAMPLE_RATE";
            case Opt::HOST_FRAMEBUF_WIDTH:       return "HOST.FRAMEBUF_WIDTH";
            case Opt::HOST_FRAMEBUF_HEIGHT:      return "HOST.FRAMEBUF_HEIGHT";
                
            case Opt::AMIGA_VIDEO_FORMAT:        return "AMIGA.VIDEO_FORMAT";
            case Opt::AMIGA_WARP_BOOT:           return "AMIGA.WARP_BOOT";
            case Opt::AMIGA_WARP_MODE:           return "AMIGA.WARP_MODE";
            case Opt::AMIGA_VSYNC:               return "AMIGA.VSYNC";
            case Opt::AMIGA_SPEED_BOOST:         return "AMIGA.SPEED_BOOST";
            case Opt::AMIGA_RUN_AHEAD:           return "AMIGA.RUN_AHEAD";
            case Opt::AMIGA_SNAP_AUTO:           return "AMIGA.SNAP_AUTO";
            case Opt::AMIGA_SNAP_DELAY:          return "AMIGA.SNAP_DELAY";
            case Opt::AMIGA_SNAP_COMPRESSOR:     return "AMIGA.SNAP_COMPRESSOR";
            case Opt::AMIGA_WS_COMPRESSION:      return "AMIGA.WS_COMPRESSION";
                
            case Opt::AGNUS_REVISION:            return "AGNUS.REVISION";
            case Opt::AGNUS_PTR_DROPS:           return "AGNUS.PTR_DROPS";
                
            case Opt::DENISE_REVISION:           return "DENISE.REVISION";
            case Opt::DENISE_VIEWPORT_TRACKING:  return "DENISE.VIEWPORT_TRACKING";
            case Opt::DENISE_FRAME_SKIPPING:     return "DENISE.FRAME_SKIPPING";
            case Opt::DENISE_HIDDEN_BITPLANES:   return "HIDDEN_BITPLANES";
            case Opt::DENISE_HIDDEN_SPRITES:     return "HIDDEN_SPRITES";
            case Opt::DENISE_HIDDEN_LAYERS:      return "HIDDEN_LAYERS";
            case Opt::DENISE_HIDDEN_LAYER_ALPHA: return "HIDDEN_LAYER_ALPHA";
            case Opt::DENISE_CLX_SPR_SPR:        return "CLX_SPR_SPR";
            case Opt::DENISE_CLX_SPR_PLF:        return "CLX_SPR_PLF";
            case Opt::DENISE_CLX_PLF_PLF:        return "CLX_PLF_PLF";
                
            case Opt::MON_PALETTE:               return "MON.PALETTE";
            case Opt::MON_BRIGHTNESS:            return "MON.BRIGHTNESS";
            case Opt::MON_CONTRAST:              return "MON.CONTRAST";
            case Opt::MON_SATURATION:            return "MON.SATURATION";
                
            case Opt::DMA_DEBUG_ENABLE:          return "DMA.DEBUG_ENABLE";
            case Opt::DMA_DEBUG_MODE:            return "DMA.DEBUG_MODE";
            case Opt::DMA_DEBUG_OPACITY:         return "DMA.DEBUG_OPACITY";
            case Opt::DMA_DEBUG_CHANNEL0:        return "DMA.DEBUG_CHANNEL0";
            case Opt::DMA_DEBUG_CHANNEL1:        return "DMA.DEBUG_CHANNEL1";
            case Opt::DMA_DEBUG_CHANNEL2:        return "DMA.DEBUG_CHANNEL2";
            case Opt::DMA_DEBUG_CHANNEL3:        return "DMA.DEBUG_CHANNEL3";
            case Opt::DMA_DEBUG_CHANNEL4:        return "DMA.DEBUG_CHANNEL4";
            case Opt::DMA_DEBUG_CHANNEL5:        return "DMA.DEBUG_CHANNEL5";
            case Opt::DMA_DEBUG_CHANNEL6:        return "DMA.DEBUG_CHANNEL6";
            case Opt::DMA_DEBUG_CHANNEL7:        return "DMA.DEBUG_CHANNEL7";
            case Opt::DMA_DEBUG_COLOR0:          return "DMA.DEBUG_COLOR0";
            case Opt::DMA_DEBUG_COLOR1:          return "DMA.DEBUG_COLOR1";
            case Opt::DMA_DEBUG_COLOR2:          return "DMA.DEBUG_COLOR2";
            case Opt::DMA_DEBUG_COLOR3:          return "DMA.DEBUG_COLOR3";
            case Opt::DMA_DEBUG_COLOR4:          return "DMA.DEBUG_COLOR4";
            case Opt::DMA_DEBUG_COLOR5:          return "DMA.DEBUG_COLOR5";
            case Opt::DMA_DEBUG_COLOR6:          return "DMA.DEBUG_COLOR6";
            case Opt::DMA_DEBUG_COLOR7:          return "DMA.DEBUG_COLOR7";
                
            case Opt::LA_PROBE0:                 return "LA.PROBE0";
            case Opt::LA_PROBE1:                 return "LA.PROBE1";
            case Opt::LA_PROBE2:                 return "LA.PROBE2";
            case Opt::LA_PROBE3:                 return "LA.PROBE3";
            case Opt::LA_ADDR0:                  return "LA.ADDR0";
            case Opt::LA_ADDR1:                  return "LA.ADDR1";
            case Opt::LA_ADDR2:                  return "LA.ADDR2";
            case Opt::LA_ADDR3:                  return "LA.ADDR3";
                
            case Opt::VID_WHITE_NOISE:           return "VID.WHITE_NOISE";
                
            case Opt::CPU_REVISION:              return "CPU.REVISION";
            case Opt::CPU_DASM_REVISION:         return "CPU.DASM_REVISION";
            case Opt::CPU_DASM_SYNTAX:           return "CPU.DASM_SYNTAX";
            case Opt::CPU_DASM_NUMBERS:          return "CPU.DASM_NUMBERS";
            case Opt::CPU_OVERCLOCKING:          return "CPU.OVERCLOCKING";
            case Opt::CPU_RESET_VAL:             return "CPU.RESET_VAL";
                
            case Opt::RTC_MODEL:                 return "RTC.MODEL";
                
            case Opt::MEM_CHIP_RAM:              return "MEM.CHIP_RAM";
            case Opt::MEM_SLOW_RAM:              return "MEM.SLOW_RAM";
            case Opt::MEM_FAST_RAM:              return "MEM.FAST_RAM";
            case Opt::MEM_EXT_START:             return "MEM.EXT_START";
            case Opt::MEM_SAVE_ROMS:             return "MEM.SAVE_ROMS";
            case Opt::MEM_SLOW_RAM_DELAY:        return "MEM.SLOW_RAM_DELAY";
            case Opt::MEM_SLOW_RAM_MIRROR:       return "MEM.SLOW_RAM_MIRROR";
            case Opt::MEM_BANKMAP:               return "MEM.BANKMAP";
            case Opt::MEM_UNMAPPING_TYPE:        return "MEM.UNMAPPING_TYPE";
            case Opt::MEM_RAM_INIT_PATTERN:      return "MEM.RAM_INIT_PATTERN";
                
            case Opt::DC_SPEED:                  return "DC.SPEED";
            case Opt::DC_LOCK_DSKSYNC:           return "DC.LOCK_DSKSYNC";
            case Opt::DC_AUTO_DSKSYNC:           return "DC.AUTO_DSKSYNC";
                
            case Opt::DRIVE_CONNECT:             return "DRIVE.CONNECT";
            case Opt::DRIVE_TYPE:                return "DRIVE.TYPE";
            case Opt::DRIVE_MECHANICS:           return "DRIVE.MECHANICS";
            case Opt::DRIVE_RPM:                 return "DRIVE.RPM";
            case Opt::DRIVE_SWAP_DELAY:          return "DRIVE.SWAP_DELAY";
            case Opt::DRIVE_PAN:                 return "DRIVE.PAN";
            case Opt::DRIVE_STEP_VOLUME:         return "DRIVE.STEP_VOLUME";
            case Opt::DRIVE_POLL_VOLUME:         return "DRIVE.POLL_VOLUME";
            case Opt::DRIVE_INSERT_VOLUME:       return "DRIVE.INSERT_VOLUME";
            case Opt::DRIVE_EJECT_VOLUME:        return "DRIVE.EJECT_VOLUME";
                
            case Opt::HDC_CONNECT:               return "HDC.CONNECT";
                
            case Opt::HDR_TYPE:                  return "HDR.TYPE";
            case Opt::HDR_PAN:                   return "HDR.PAN";
            case Opt::HDR_STEP_VOLUME:           return "HDR.STEP_VOLUME";
                
            case Opt::SER_DEVICE:                return "SER.DEVICE";
            case Opt::SER_VERBOSE:               return "SER.VERBOSE";
                
            case Opt::BLITTER_ACCURACY:          return "BLITTER.ACCURACY";
                
            case Opt::CIA_REVISION:              return "CIA.REVISION";
            case Opt::CIA_TODBUG:                return "CIA.TODBUG";
            case Opt::CIA_ECLOCK_SYNCING:        return "CIA.ECLOCK_SYNCING";
            case Opt::CIA_IDLE_SLEEP:            return "CIA.IDLE_SLEEP";
                
            case Opt::KBD_ACCURACY:              return "KBD.ACCURACY";
                
            case Opt::MOUSE_PULLUP_RESISTORS:    return "MOUSE.PULLUP_RESISTORS";
            case Opt::MOUSE_SHAKE_DETECTION:     return "MOUSE.SHAKE_DETECTION";
            case Opt::MOUSE_VELOCITY:            return "MOUSE.VELOCITY";
                
            case Opt::JOY_AUTOFIRE:              return "JOY.AUTOFIRE";
            case Opt::JOY_AUTOFIRE_BURSTS:       return "JOY.AUTOFIRE_BURSTS";
            case Opt::JOY_AUTOFIRE_BULLETS:      return "JOY.AUTOFIRE_BULLETS";
            case Opt::JOY_AUTOFIRE_DELAY:        return "JOY.AUTOFIRE_DELAY";
                
            case Opt::AUD_PAN0:                  return "AUD.PAN0";
            case Opt::AUD_PAN1:                  return "AUD.PAN1";
            case Opt::AUD_PAN2:                  return "AUD.PAN2";
            case Opt::AUD_PAN3:                  return "AUD.PAN3";
            case Opt::AUD_VOL0:                  return "AUD.VOL0";
            case Opt::AUD_VOL1:                  return "AUD.VOL1";
            case Opt::AUD_VOL2:                  return "AUD.VOL2";
            case Opt::AUD_VOL3:                  return "AUD.VOL3";
            case Opt::AUD_VOLL:                  return "AUD.VOLL";
            case Opt::AUD_VOLR:                  return "AUD.VOLR";
            case Opt::AUD_FILTER_TYPE:           return "AUD.FILTER_TYPE";
            case Opt::AUD_BUFFER_SIZE:           return "AUD.BUFFER_SIZE";
            case Opt::AUD_SAMPLING_METHOD:       return "AUD.SAMPLING_METHOD";
            case Opt::AUD_ASR:                   return "AUD.ASR";
            case Opt::AUD_FASTPATH:              return "AUD.FASTPATH";
                
            case Opt::DIAG_BOARD:                return "DIAG_BOARD";
                
            case Opt::SRV_PORT:                  return "SRV.PORT";
            case Opt::SRV_PROTOCOL:              return "SRV.PROTOCOL";
            case Opt::SRV_AUTORUN:               return "SRV.AUTORUN";
            case Opt::SRV_VERBOSE:               return "SRV.VERBOSE";
        }
        return "???";
    }
    
    static const char *help(Opt value)
    {
        switch (value) {
                
            case Opt::HOST_REFRESH_RATE:         return "Host video refresh rate";
            case Opt::HOST_SAMPLE_RATE:          return "Host audio sample rate";
            case Opt::HOST_FRAMEBUF_WIDTH:       return "Window width";
            case Opt::HOST_FRAMEBUF_HEIGHT:      return "Window height";
                
            case Opt::AMIGA_VIDEO_FORMAT:        return "Video format";
            case Opt::AMIGA_WARP_BOOT:           return "Warp-boot duration";
            case Opt::AMIGA_WARP_MODE:           return "Warp activation";
            case Opt::AMIGA_VSYNC:               return "VSYNC mode";
            case Opt::AMIGA_SPEED_BOOST:         return "Speed adjustment";
            case Opt::AMIGA_RUN_AHEAD:           return "Run-ahead frames";
            case Opt::AMIGA_SNAP_AUTO:           return "Automatically take snapshots";
            case Opt::AMIGA_SNAP_DELAY:          return "Time span between two snapshots";
            case Opt::AMIGA_SNAP_COMPRESSOR:     return "Snapshot compression method";
            case Opt::AMIGA_WS_COMPRESSION:      return "Compress workspaces";

            case Opt::AGNUS_REVISION:            return "Chip revision";
            case Opt::AGNUS_PTR_DROPS:           return "Ignore certain register writes";
                
            case Opt::DENISE_REVISION:           return "Chip revision";
            case Opt::DENISE_VIEWPORT_TRACKING:  return "Track the currently used viewport";
            case Opt::DENISE_FRAME_SKIPPING:     return "Reduce frame rate in warp mode";
            case Opt::DENISE_HIDDEN_BITPLANES:   return "Hide bitplanes";
            case Opt::DENISE_HIDDEN_SPRITES:     return "Hide sprites";
            case Opt::DENISE_HIDDEN_LAYERS:      return "Hide playfields";
            case Opt::DENISE_HIDDEN_LAYER_ALPHA: return "Hidden playfield opacity";
            case Opt::DENISE_CLX_SPR_SPR:        return "Detect sprite-sprite collisions";
            case Opt::DENISE_CLX_SPR_PLF:        return "Detect sprite-playfield collisions";
            case Opt::DENISE_CLX_PLF_PLF:        return "Detect playfield-playfield collisions";
                
            case Opt::MON_PALETTE:               return "Color palette";
            case Opt::MON_BRIGHTNESS:            return "Monitor brightness";
            case Opt::MON_CONTRAST:              return "Monitor contrast";
            case Opt::MON_SATURATION:            return "Monitor saturation";
                
            case Opt::DMA_DEBUG_ENABLE:          return "DMA Debugger";
            case Opt::DMA_DEBUG_MODE:            return "DMA Debugger style";
            case Opt::DMA_DEBUG_OPACITY:         return "Opacity";
            case Opt::DMA_DEBUG_CHANNEL0:        return "Copper DMA";
            case Opt::DMA_DEBUG_CHANNEL1:        return "Blitter DMA";
            case Opt::DMA_DEBUG_CHANNEL2:        return "Disk DMA";
            case Opt::DMA_DEBUG_CHANNEL3:        return "Audio DMA";
            case Opt::DMA_DEBUG_CHANNEL4:        return "Sprite DMA";
            case Opt::DMA_DEBUG_CHANNEL5:        return "Bitplane DMA";
            case Opt::DMA_DEBUG_CHANNEL6:        return "CPU DMA";
            case Opt::DMA_DEBUG_CHANNEL7:        return "Memory Refresh DMA";
            case Opt::DMA_DEBUG_COLOR0:          return "Copper color";
            case Opt::DMA_DEBUG_COLOR1:          return "Blitter color";
            case Opt::DMA_DEBUG_COLOR2:          return "Disk color";
            case Opt::DMA_DEBUG_COLOR3:          return "Audio color";
            case Opt::DMA_DEBUG_COLOR4:          return "Sprite color";
            case Opt::DMA_DEBUG_COLOR5:          return "Bitplane color";
            case Opt::DMA_DEBUG_COLOR6:          return "CPU color";
            case Opt::DMA_DEBUG_COLOR7:          return "Memory refresh color";
                
            case Opt::LA_PROBE0:                 return "Probe on channel 0";
            case Opt::LA_PROBE1:                 return "Probe on channel 1";
            case Opt::LA_PROBE2:                 return "Probe on channel 2";
            case Opt::LA_PROBE3:                 return "Probe on channel 3";
            case Opt::LA_ADDR0:                  return "Channel 0 memory address";
            case Opt::LA_ADDR1:                  return "Channel 1 memory address";
            case Opt::LA_ADDR2:                  return "Channel 2 memory address";
            case Opt::LA_ADDR3:                  return "Channel 3 memory address";
                
            case Opt::VID_WHITE_NOISE:           return "White noise";
                
            case Opt::CPU_REVISION:              return "Chip revision";
            case Opt::CPU_DASM_REVISION:         return "Chip revision (disassembler)";
            case Opt::CPU_DASM_SYNTAX:           return "Disassembler syntax";
            case Opt::CPU_DASM_NUMBERS:          return "Disassembler number format";
            case Opt::CPU_OVERCLOCKING:          return "Overclocking factor";
            case Opt::CPU_RESET_VAL:             return "Register reset value";
                
            case Opt::RTC_MODEL:                 return "Chip revision";
                
            case Opt::MEM_CHIP_RAM:              return "Chip RAM size";
            case Opt::MEM_SLOW_RAM:              return "Slow RAM size";
            case Opt::MEM_FAST_RAM:              return "Fast RAM size";
            case Opt::MEM_EXT_START:             return "Extension ROM location";
            case Opt::MEM_SAVE_ROMS:             return "Include ROMs in snapshots";
            case Opt::MEM_SLOW_RAM_DELAY:        return "Emulate slow RAM bus delays";
            case Opt::MEM_SLOW_RAM_MIRROR:       return "ECS memory mirroring";
            case Opt::MEM_BANKMAP:               return "Memory mapping scheme";
            case Opt::MEM_UNMAPPING_TYPE:        return "Unmapped memory behavior";
            case Opt::MEM_RAM_INIT_PATTERN:      return "RAM initialization pattern";
                
            case Opt::DC_SPEED:                  return "Drive speed";
            case Opt::DC_LOCK_DSKSYNC:           return "Ignore writes to DSKSYNC";
            case Opt::DC_AUTO_DSKSYNC:           return "Always find a sync mark";
                
            case Opt::DRIVE_CONNECT:             return "Connection status";
            case Opt::DRIVE_TYPE:                return "Drive model";
            case Opt::DRIVE_MECHANICS:           return "Emulate drive mechanics";
            case Opt::DRIVE_RPM:                 return "Disk rotation speed";
            case Opt::DRIVE_SWAP_DELAY:          return "Disk swap delay";
            case Opt::DRIVE_PAN:                 return "Pan";
            case Opt::DRIVE_STEP_VOLUME:         return "Head step volume";
            case Opt::DRIVE_POLL_VOLUME:         return "Disk polling step volume";
            case Opt::DRIVE_INSERT_VOLUME:       return "Disk insertion volume";
            case Opt::DRIVE_EJECT_VOLUME:        return "Disk ejection volume";
                
            case Opt::HDC_CONNECT:               return "Connection status";
                
            case Opt::HDR_TYPE:                  return "Drive model";
            case Opt::HDR_PAN:                   return "Pan";
            case Opt::HDR_STEP_VOLUME:           return "Head step volume";
                
            case Opt::SER_DEVICE:                return "Serial device type";
            case Opt::SER_VERBOSE:               return "Verbose";
                
            case Opt::BLITTER_ACCURACY:          return "Blitter accuracy level";
                
            case Opt::CIA_REVISION:              return "Chip revision";
            case Opt::CIA_TODBUG:                return "Emulate TOD bug";
            case Opt::CIA_ECLOCK_SYNCING:        return "Sync accesses with E-clock";
            case Opt::CIA_IDLE_SLEEP:            return "Enter idle state while not in use";
                
            case Opt::KBD_ACCURACY:              return "Keyboard emulation accuracy";
                
            case Opt::MOUSE_PULLUP_RESISTORS:    return "Emulate pullup resistors";
            case Opt::MOUSE_SHAKE_DETECTION:     return "Detect a shaked mouse";
            case Opt::MOUSE_VELOCITY:            return "Mouse velocity";
                
            case Opt::JOY_AUTOFIRE:              return "Autofire";
            case Opt::JOY_AUTOFIRE_BURSTS:       return "Burst mode";
            case Opt::JOY_AUTOFIRE_BULLETS:      return "Number of bullets per burst";
            case Opt::JOY_AUTOFIRE_DELAY:        return "Autofire delay in frames";
                
            case Opt::AUD_PAN0:                  return "Channel 0 pan";
            case Opt::AUD_PAN1:                  return "Channel 1 pan";
            case Opt::AUD_PAN2:                  return "Channel 2 pan";
            case Opt::AUD_PAN3:                  return "Channel 3 pan";
            case Opt::AUD_VOL0:                  return "Channel 0 volume";
            case Opt::AUD_VOL1:                  return "Channel 1 volume";
            case Opt::AUD_VOL2:                  return "Channel 2 volume";
            case Opt::AUD_VOL3:                  return "Channel 3 volume";
            case Opt::AUD_VOLL:                  return "Master volume (left)";
            case Opt::AUD_VOLR:                  return "Master volume (right)";
            case Opt::AUD_FILTER_TYPE:           return "Audio filter type";
            case Opt::AUD_BUFFER_SIZE:           return "Audio buffer capacity";
            case Opt::AUD_SAMPLING_METHOD:       return "Sampling method";
            case Opt::AUD_ASR:                   return "Adaptive Sample Rate";
            case Opt::AUD_FASTPATH:              return "Boost performance";
                
            case Opt::DIAG_BOARD:                return "Diagnose board";
                
            case Opt::SRV_PORT:                  return "Server port";
            case Opt::SRV_PROTOCOL:              return "Server protocol";
            case Opt::SRV_AUTORUN:               return "Auto run";
            case Opt::SRV_VERBOSE:               return "Verbose mode";
        }
        return "???";
    }
};

}
