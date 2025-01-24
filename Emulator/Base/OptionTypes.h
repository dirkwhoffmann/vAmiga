// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class Option : long
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
    AMIGA_SNAP_COMPRESS,    ///< Compress snapshot data
    
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
    
    ///<
    // Video port
    VID_WHITE_NOISE,        ///< Generate white-noise when switched off
    ///<
    ///<
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
    HDR_WRITE_THROUGH,
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
    AUD_SAMPLING_METHOD,
    AUD_FILTER_TYPE,
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
    AUD_FASTPATH,
    
    // Expansion boards
    DIAG_BOARD,
    
    // Remote servers
    SRV_PORT,
    SRV_PROTOCOL,
    SRV_AUTORUN,
    SRV_VERBOSE
};

struct OptionEnum : util::Reflection<OptionEnum, Option>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Option::SRV_VERBOSE);
    
    static const char *prefix() { return "OPT"; }
    static const char *_key(Option value)
    {
        switch (value) {
                
            case Option::HOST_REFRESH_RATE:         return "HOST.REFRESH_RATE";
            case Option::HOST_SAMPLE_RATE:          return "HOST.SAMPLE_RATE";
            case Option::HOST_FRAMEBUF_WIDTH:       return "HOST.FRAMEBUF_WIDTH";
            case Option::HOST_FRAMEBUF_HEIGHT:      return "HOST.FRAMEBUF_HEIGHT";
                
            case Option::AMIGA_VIDEO_FORMAT:        return "AMIGA.VIDEO_FORMAT";
            case Option::AMIGA_WARP_BOOT:           return "AMIGA.WARP_BOOT";
            case Option::AMIGA_WARP_MODE:           return "AMIGA.WARP_MODE";
            case Option::AMIGA_VSYNC:               return "AMIGA.VSYNC";
            case Option::AMIGA_SPEED_BOOST:         return "AMIGA.SPEED_BOOST";
            case Option::AMIGA_RUN_AHEAD:           return "AMIGA.RUN_AHEAD";
                
            case Option::AMIGA_SNAP_AUTO:           return "AMIGA.SNAP_AUTO";
            case Option::AMIGA_SNAP_DELAY:          return "AMIGA.SNAP_DELAY";
            case Option::AMIGA_SNAP_COMPRESS:       return "AMIGA.SNAP_COMPRESS";
                
            case Option::AGNUS_REVISION:            return "AGNUS.REVISION";
            case Option::AGNUS_PTR_DROPS:           return "AGNUS.PTR_DROPS";
                
            case Option::DENISE_REVISION:           return "DENISE.REVISION";
            case Option::DENISE_VIEWPORT_TRACKING:  return "DENISE.VIEWPORT_TRACKING";
            case Option::DENISE_FRAME_SKIPPING:     return "DENISE.FRAME_SKIPPING";
            case Option::DENISE_HIDDEN_BITPLANES:   return "HIDDEN_BITPLANES";
            case Option::DENISE_HIDDEN_SPRITES:     return "HIDDEN_SPRITES";
            case Option::DENISE_HIDDEN_LAYERS:      return "HIDDEN_LAYERS";
            case Option::DENISE_HIDDEN_LAYER_ALPHA: return "HIDDEN_LAYER_ALPHA";
            case Option::DENISE_CLX_SPR_SPR:        return "CLX_SPR_SPR";
            case Option::DENISE_CLX_SPR_PLF:        return "CLX_SPR_PLF";
            case Option::DENISE_CLX_PLF_PLF:        return "CLX_PLF_PLF";
                
            case Option::MON_PALETTE:               return "MON.PALETTE";
            case Option::MON_BRIGHTNESS:            return "MON.BRIGHTNESS";
            case Option::MON_CONTRAST:              return "MON.CONTRAST";
            case Option::MON_SATURATION:            return "MON.SATURATION";
                
            case Option::DMA_DEBUG_ENABLE:          return "DMA.DEBUG_ENABLE";
            case Option::DMA_DEBUG_MODE:            return "DMA.DEBUG_MODE";
            case Option::DMA_DEBUG_OPACITY:         return "DMA.DEBUG_OPACITY";
            case Option::DMA_DEBUG_CHANNEL0:        return "DMA.DEBUG_CHANNEL0";
            case Option::DMA_DEBUG_CHANNEL1:        return "DMA.DEBUG_CHANNEL1";
            case Option::DMA_DEBUG_CHANNEL2:        return "DMA.DEBUG_CHANNEL2";
            case Option::DMA_DEBUG_CHANNEL3:        return "DMA.DEBUG_CHANNEL3";
            case Option::DMA_DEBUG_CHANNEL4:        return "DMA.DEBUG_CHANNEL4";
            case Option::DMA_DEBUG_CHANNEL5:        return "DMA.DEBUG_CHANNEL5";
            case Option::DMA_DEBUG_CHANNEL6:        return "DMA.DEBUG_CHANNEL6";
            case Option::DMA_DEBUG_CHANNEL7:        return "DMA.DEBUG_CHANNEL7";
            case Option::DMA_DEBUG_COLOR0:          return "DMA.DEBUG_COLOR0";
            case Option::DMA_DEBUG_COLOR1:          return "DMA.DEBUG_COLOR1";
            case Option::DMA_DEBUG_COLOR2:          return "DMA.DEBUG_COLOR2";
            case Option::DMA_DEBUG_COLOR3:          return "DMA.DEBUG_COLOR3";
            case Option::DMA_DEBUG_COLOR4:          return "DMA.DEBUG_COLOR4";
            case Option::DMA_DEBUG_COLOR5:          return "DMA.DEBUG_COLOR5";
            case Option::DMA_DEBUG_COLOR6:          return "DMA.DEBUG_COLOR6";
            case Option::DMA_DEBUG_COLOR7:          return "DMA.DEBUG_COLOR7";
                
            case Option::LA_PROBE0:                 return "LA.PROBE0";
            case Option::LA_PROBE1:                 return "LA.PROBE1";
            case Option::LA_PROBE2:                 return "LA.PROBE2";
            case Option::LA_PROBE3:                 return "LA.PROBE3";
            case Option::LA_ADDR0:                  return "LA.ADDR0";
            case Option::LA_ADDR1:                  return "LA.ADDR1";
            case Option::LA_ADDR2:                  return "LA.ADDR2";
            case Option::LA_ADDR3:                  return "LA.ADDR3";
                
            case Option::VID_WHITE_NOISE:           return "VID.WHITE_NOISE";
                
            case Option::CPU_REVISION:              return "CPU.REVISION";
            case Option::CPU_DASM_REVISION:         return "CPU.DASM_REVISION";
            case Option::CPU_DASM_SYNTAX:           return "CPU.DASM_SYNTAX";
            case Option::CPU_DASM_NUMBERS:          return "CPU.DASM_NUMBERS";
            case Option::CPU_OVERCLOCKING:          return "CPU.OVERCLOCKING";
            case Option::CPU_RESET_VAL:             return "CPU.RESET_VAL";
                
            case Option::RTC_MODEL:                 return "RTC.MODEL";
                
            case Option::MEM_CHIP_RAM:              return "MEM.CHIP_RAM";
            case Option::MEM_SLOW_RAM:              return "MEM.SLOW_RAM";
            case Option::MEM_FAST_RAM:              return "MEM.FAST_RAM";
            case Option::MEM_EXT_START:             return "MEM.EXT_START";
            case Option::MEM_SAVE_ROMS:             return "MEM.SAVE_ROMS";
            case Option::MEM_SLOW_RAM_DELAY:        return "MEM.SLOW_RAM_DELAY";
            case Option::MEM_SLOW_RAM_MIRROR:       return "MEM.SLOW_RAM_MIRROR";
            case Option::MEM_BANKMAP:               return "MEM.BANKMAP";
            case Option::MEM_UNMAPPING_TYPE:        return "MEM.UNMAPPING_TYPE";
            case Option::MEM_RAM_INIT_PATTERN:      return "MEM.RAM_INIT_PATTERN";
                
            case Option::DC_SPEED:                  return "DC.SPEED";
            case Option::DC_LOCK_DSKSYNC:           return "DC.LOCK_DSKSYNC";
            case Option::DC_AUTO_DSKSYNC:           return "DC.AUTO_DSKSYNC";
                
            case Option::DRIVE_CONNECT:             return "DRIVE.CONNECT";
            case Option::DRIVE_TYPE:                return "DRIVE.TYPE";
            case Option::DRIVE_MECHANICS:           return "DRIVE.MECHANICS";
            case Option::DRIVE_RPM:                 return "DRIVE.RPM";
            case Option::DRIVE_SWAP_DELAY:          return "DRIVE.SWAP_DELAY";
            case Option::DRIVE_PAN:                 return "DRIVE.PAN";
            case Option::DRIVE_STEP_VOLUME:         return "DRIVE.STEP_VOLUME";
            case Option::DRIVE_POLL_VOLUME:         return "DRIVE.POLL_VOLUME";
            case Option::DRIVE_INSERT_VOLUME:       return "DRIVE.INSERT_VOLUME";
            case Option::DRIVE_EJECT_VOLUME:        return "DRIVE.EJECT_VOLUME";
                
            case Option::HDC_CONNECT:               return "HDC.CONNECT";
                
            case Option::HDR_TYPE:                  return "HDR.TYPE";
            case Option::HDR_WRITE_THROUGH:         return "HDR.WRITE_THROUGH";
            case Option::HDR_PAN:                   return "HDR.PAN";
            case Option::HDR_STEP_VOLUME:           return "HDR.STEP_VOLUME";
                
            case Option::SER_DEVICE:                return "SER.DEVICE";
            case Option::SER_VERBOSE:               return "SER.VERBOSE";
                
            case Option::BLITTER_ACCURACY:          return "BLITTER.ACCURACY";
                
            case Option::CIA_REVISION:              return "CIA.REVISION";
            case Option::CIA_TODBUG:                return "CIA.TODBUG";
            case Option::CIA_ECLOCK_SYNCING:        return "CIA.ECLOCK_SYNCING";
            case Option::CIA_IDLE_SLEEP:            return "CIA.IDLE_SLEEP";
                
            case Option::KBD_ACCURACY:              return "KBD.ACCURACY";
                
            case Option::MOUSE_PULLUP_RESISTORS:    return "MOUSE.PULLUP_RESISTORS";
            case Option::MOUSE_SHAKE_DETECTION:     return "MOUSE.SHAKE_DETECTION";
            case Option::MOUSE_VELOCITY:            return "MOUSE.VELOCITY";
                
            case Option::JOY_AUTOFIRE:              return "JOY.AUTOFIRE";
            case Option::JOY_AUTOFIRE_BURSTS:       return "JOY.AUTOFIRE_BURSTS";
            case Option::JOY_AUTOFIRE_BULLETS:      return "JOY.AUTOFIRE_BULLETS";
            case Option::JOY_AUTOFIRE_DELAY:        return "JOY.AUTOFIRE_DELAY";
                
            case Option::AUD_SAMPLING_METHOD:       return "AUD.SAMPLING_METHOD";
            case Option::AUD_FILTER_TYPE:           return "AUD.FILTER_TYPE";
            case Option::AUD_PAN0:                  return "AUD.PAN0";
            case Option::AUD_PAN1:                  return "AUD.PAN1";
            case Option::AUD_PAN2:                  return "AUD.PAN2";
            case Option::AUD_PAN3:                  return "AUD.PAN3";
            case Option::AUD_VOL0:                  return "AUD.VOL0";
            case Option::AUD_VOL1:                  return "AUD.VOL1";
            case Option::AUD_VOL2:                  return "AUD.VOL2";
            case Option::AUD_VOL3:                  return "AUD.VOL3";
            case Option::AUD_VOLL:                  return "AUD.VOLL";
            case Option::AUD_VOLR:                  return "AUD.VOLR";
            case Option::AUD_FASTPATH:              return "AUD.FASTPATH";
                
            case Option::DIAG_BOARD:                return "DIAG_BOARD";
                
            case Option::SRV_PORT:                  return "SRV.PORT";
            case Option::SRV_PROTOCOL:              return "SRV.PROTOCOL";
            case Option::SRV_AUTORUN:               return "SRV.AUTORUN";
            case Option::SRV_VERBOSE:               return "SRV.VERBOSE";
        }
        return "???";
    }
    
    static const char *help(Option value)
    {
        switch (value) {
                
            case Option::HOST_REFRESH_RATE:         return "Host video refresh rate";
            case Option::HOST_SAMPLE_RATE:          return "Host audio sample rate";
            case Option::HOST_FRAMEBUF_WIDTH:       return "Window width";
            case Option::HOST_FRAMEBUF_HEIGHT:      return "Window height";
                
            case Option::AMIGA_VIDEO_FORMAT:        return "Video format";
            case Option::AMIGA_WARP_BOOT:           return "Warp-boot duration";
            case Option::AMIGA_WARP_MODE:           return "Warp activation";
            case Option::AMIGA_VSYNC:               return "VSYNC mode";
            case Option::AMIGA_SPEED_BOOST:         return "Speed adjustment";
            case Option::AMIGA_RUN_AHEAD:           return "Run-ahead frames";
                
            case Option::AMIGA_SNAP_AUTO:           return "Automatically take snapshots";
            case Option::AMIGA_SNAP_DELAY:          return "Time span between two snapshots";
            case Option::AMIGA_SNAP_COMPRESS:       return "Compress snapshot data";
                
            case Option::AGNUS_REVISION:            return "Chip revision";
            case Option::AGNUS_PTR_DROPS:           return "Ignore certain register writes";
                
            case Option::DENISE_REVISION:           return "Chip revision";
            case Option::DENISE_VIEWPORT_TRACKING:  return "Track the currently used viewport";
            case Option::DENISE_FRAME_SKIPPING:     return "Reduce frame rate in warp mode";
            case Option::DENISE_HIDDEN_BITPLANES:   return "Hide bitplanes";
            case Option::DENISE_HIDDEN_SPRITES:     return "Hide sprites";
            case Option::DENISE_HIDDEN_LAYERS:      return "Hide playfields";
            case Option::DENISE_HIDDEN_LAYER_ALPHA: return "Hidden playfield opacity";
            case Option::DENISE_CLX_SPR_SPR:        return "Detect sprite-sprite collisions";
            case Option::DENISE_CLX_SPR_PLF:        return "Detect sprite-playfield collisions";
            case Option::DENISE_CLX_PLF_PLF:        return "Detect playfield-playfield collisions";
                
            case Option::MON_PALETTE:               return "Color palette";
            case Option::MON_BRIGHTNESS:            return "Monitor brightness";
            case Option::MON_CONTRAST:              return "Monitor contrast";
            case Option::MON_SATURATION:            return "Monitor saturation";
                
            case Option::DMA_DEBUG_ENABLE:          return "DMA Debugger";
            case Option::DMA_DEBUG_MODE:            return "DMA Debugger style";
            case Option::DMA_DEBUG_OPACITY:         return "Opacity";
            case Option::DMA_DEBUG_CHANNEL0:        return "Copper DMA";
            case Option::DMA_DEBUG_CHANNEL1:        return "Blitter DMA";
            case Option::DMA_DEBUG_CHANNEL2:        return "Disk DMA";
            case Option::DMA_DEBUG_CHANNEL3:        return "Audio DMA";
            case Option::DMA_DEBUG_CHANNEL4:        return "Sprite DMA";
            case Option::DMA_DEBUG_CHANNEL5:        return "Bitplane DMA";
            case Option::DMA_DEBUG_CHANNEL6:        return "CPU DMA";
            case Option::DMA_DEBUG_CHANNEL7:        return "Memory Refresh DMA";
            case Option::DMA_DEBUG_COLOR0:          return "Copper color";
            case Option::DMA_DEBUG_COLOR1:          return "Blitter color";
            case Option::DMA_DEBUG_COLOR2:          return "Disk color";
            case Option::DMA_DEBUG_COLOR3:          return "Audio color";
            case Option::DMA_DEBUG_COLOR4:          return "Sprite color";
            case Option::DMA_DEBUG_COLOR5:          return "Bitplane color";
            case Option::DMA_DEBUG_COLOR6:          return "CPU color";
            case Option::DMA_DEBUG_COLOR7:          return "Memory refresh color";
                
            case Option::LA_PROBE0:                 return "Probe on channel 0";
            case Option::LA_PROBE1:                 return "Probe on channel 1";
            case Option::LA_PROBE2:                 return "Probe on channel 2";
            case Option::LA_PROBE3:                 return "Probe on channel 3";
            case Option::LA_ADDR0:                  return "Channel 0 memory address";
            case Option::LA_ADDR1:                  return "Channel 1 memory address";
            case Option::LA_ADDR2:                  return "Channel 2 memory address";
            case Option::LA_ADDR3:                  return "Channel 3 memory address";
                
            case Option::VID_WHITE_NOISE:           return "White noise";
                
            case Option::CPU_REVISION:              return "Chip revision";
            case Option::CPU_DASM_REVISION:         return "Chip revision (disassembler)";
            case Option::CPU_DASM_SYNTAX:           return "Disassembler syntax";
            case Option::CPU_DASM_NUMBERS:          return "Disassembler number format";
            case Option::CPU_OVERCLOCKING:          return "Overclocking factor";
            case Option::CPU_RESET_VAL:             return "Register reset value";
                
            case Option::RTC_MODEL:                 return "Chip revision";
                
            case Option::MEM_CHIP_RAM:              return "Chip RAM size";
            case Option::MEM_SLOW_RAM:              return "Slow RAM size";
            case Option::MEM_FAST_RAM:              return "Fast RAM size";
            case Option::MEM_EXT_START:             return "Extension ROM location";
            case Option::MEM_SAVE_ROMS:             return "Include ROMs in snapshots";
            case Option::MEM_SLOW_RAM_DELAY:        return "Emulate slow RAM bus delays";
            case Option::MEM_SLOW_RAM_MIRROR:       return "ECS memory mirroring";
            case Option::MEM_BANKMAP:               return "Memory mapping scheme";
            case Option::MEM_UNMAPPING_TYPE:        return "Unmapped memory behavior";
            case Option::MEM_RAM_INIT_PATTERN:      return "RAM initialization pattern";
                
            case Option::DC_SPEED:                  return "Drive speed";
            case Option::DC_LOCK_DSKSYNC:           return "Ignore writes to DSKSYNC";
            case Option::DC_AUTO_DSKSYNC:           return "Always find a sync mark";
                
            case Option::DRIVE_CONNECT:             return "Connected";
            case Option::DRIVE_TYPE:                return "Drive model";
            case Option::DRIVE_MECHANICS:           return "Emulate drive mechanics";
            case Option::DRIVE_RPM:                 return "Disk rotation speed";
            case Option::DRIVE_SWAP_DELAY:          return "Disk swap delay";
            case Option::DRIVE_PAN:                 return "Pan";
            case Option::DRIVE_STEP_VOLUME:         return "Head step volume";
            case Option::DRIVE_POLL_VOLUME:         return "Disk polling step volume";
            case Option::DRIVE_INSERT_VOLUME:       return "Disk insertion volume";
            case Option::DRIVE_EJECT_VOLUME:        return "Disk ejection volume";
                
            case Option::HDC_CONNECT:               return "Connected";
                
            case Option::HDR_TYPE:                  return "Drive model";
            case Option::HDR_WRITE_THROUGH:         return "Keep data alive";
            case Option::HDR_PAN:                   return "Pan";
            case Option::HDR_STEP_VOLUME:           return "Head step volume";
                
            case Option::SER_DEVICE:                return "Serial device type";
            case Option::SER_VERBOSE:               return "Verbose";
                
            case Option::BLITTER_ACCURACY:          return "Blitter accuracy level";
                
            case Option::CIA_REVISION:              return "Chip revision";
            case Option::CIA_TODBUG:                return "Emulate TOD bug";
            case Option::CIA_ECLOCK_SYNCING:        return "Sync accesses with E-clock";
            case Option::CIA_IDLE_SLEEP:            return "Enter idle state while not in use";
                
            case Option::KBD_ACCURACY:              return "Keyboard emulation accuracy";
                
            case Option::MOUSE_PULLUP_RESISTORS:    return "Emulate pullup resistors";
            case Option::MOUSE_SHAKE_DETECTION:     return "Detect a shaked mouse";
            case Option::MOUSE_VELOCITY:            return "Mouse velocity";
                
            case Option::JOY_AUTOFIRE:              return "Autofire";
            case Option::JOY_AUTOFIRE_BURSTS:       return "Burst mode";
            case Option::JOY_AUTOFIRE_BULLETS:      return "Number of bullets per burst";
            case Option::JOY_AUTOFIRE_DELAY:        return "Autofire delay in frames";
                
            case Option::AUD_SAMPLING_METHOD:       return "Sampling method";
            case Option::AUD_FILTER_TYPE:           return "Audio filter type";
            case Option::AUD_PAN0:                  return "Channel 0 pan";
            case Option::AUD_PAN1:                  return "Channel 1 pan";
            case Option::AUD_PAN2:                  return "Channel 2 pan";
            case Option::AUD_PAN3:                  return "Channel 3 pan";
            case Option::AUD_VOL0:                  return "Channel 0 volume";
            case Option::AUD_VOL1:                  return "Channel 1 volume";
            case Option::AUD_VOL2:                  return "Channel 2 volume";
            case Option::AUD_VOL3:                  return "Channel 3 volume";
            case Option::AUD_VOLL:                  return "Master volume (left)";
            case Option::AUD_VOLR:                  return "Master volume (right)";
            case Option::AUD_FASTPATH:              return "Boost performance";
                
            case Option::DIAG_BOARD:                return "Diagnose board";
                
            case Option::SRV_PORT:                  return "Server port";
            case Option::SRV_PROTOCOL:              return "Server protocol";
            case Option::SRV_AUTORUN:               return "Auto run";
            case Option::SRV_VERBOSE:               return "Verbose mode";
        }
        return "???";
    }
};

}
