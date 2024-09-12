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

//
// Enumerations
//

enum_long(OPT)
{
    // Host
    OPT_HOST_REFRESH_RATE,      ///< Refresh rate of the host display
    OPT_HOST_SAMPLE_RATE,       ///< Refresh rate of the host display
    OPT_HOST_FRAMEBUF_WIDTH,    ///< Current width of the emulator window
    OPT_HOST_FRAMEBUF_HEIGHT,   ///< Current height of the emulator window

    // Amiga
    OPT_AMIGA_VIDEO_FORMAT,     ///< Machine type (PAL or NTSC)
    OPT_AMIGA_WARP_BOOT,        ///< Warp-boot time in seconds
    OPT_AMIGA_WARP_MODE,        ///< Warp activation mode
    OPT_AMIGA_VSYNC,            ///< Derive the frame rate to the VSYNC signal
    OPT_AMIGA_SPEED_BOOST,      ///< Speed adjustment in percent
    OPT_AMIGA_RUN_AHEAD,        ///< Number of run-ahead frames

    // Snapshots
    OPT_AMIGA_SNAP_AUTO,        ///< Automatically take a snapshots
    OPT_AMIGA_SNAP_DELAY,       ///< Delay between two snapshots in seconds
    OPT_AMIGA_SNAP_COMPRESS,    ///< Compress snapshot data

    // Agnus
    OPT_AGNUS_REVISION,
    OPT_AGNUS_PTR_DROPS,

    // Denise
    OPT_DENISE_REVISION,
    OPT_DENISE_VIEWPORT_TRACKING,
    OPT_DENISE_FRAME_SKIPPING,
    OPT_DENISE_HIDDEN_BITPLANES,
    OPT_DENISE_HIDDEN_SPRITES,
    OPT_DENISE_HIDDEN_LAYERS,
    OPT_DENISE_HIDDEN_LAYER_ALPHA,
    OPT_DENISE_CLX_SPR_SPR,
    OPT_DENISE_CLX_SPR_PLF,
    OPT_DENISE_CLX_PLF_PLF,

    // Monitor
    OPT_MON_PALETTE,
    OPT_MON_BRIGHTNESS,
    OPT_MON_CONTRAST,
    OPT_MON_SATURATION,

    // DMA Debugger
    OPT_DMA_DEBUG_ENABLE,
    OPT_DMA_DEBUG_MODE,
    OPT_DMA_DEBUG_OPACITY,
    OPT_DMA_DEBUG_CHANNEL0,
    OPT_DMA_DEBUG_CHANNEL1,
    OPT_DMA_DEBUG_CHANNEL2,
    OPT_DMA_DEBUG_CHANNEL3,
    OPT_DMA_DEBUG_CHANNEL4,
    OPT_DMA_DEBUG_CHANNEL5,
    OPT_DMA_DEBUG_CHANNEL6,
    OPT_DMA_DEBUG_CHANNEL7,
    OPT_DMA_DEBUG_COLOR0,
    OPT_DMA_DEBUG_COLOR1,
    OPT_DMA_DEBUG_COLOR2,
    OPT_DMA_DEBUG_COLOR3,
    OPT_DMA_DEBUG_COLOR4,
    OPT_DMA_DEBUG_COLOR5,
    OPT_DMA_DEBUG_COLOR6,
    OPT_DMA_DEBUG_COLOR7,

    // Video port
    OPT_VID_WHITE_NOISE,        ///< Generate white-noise when switched off
    ///<
    ///<
    // CPU
    OPT_CPU_REVISION,
    OPT_CPU_DASM_REVISION,
    OPT_CPU_DASM_SYNTAX,
    OPT_CPU_OVERCLOCKING,
    OPT_CPU_RESET_VAL,

    // Real-time clock
    OPT_RTC_MODEL,

    // Memory
    OPT_MEM_CHIP_RAM,
    OPT_MEM_SLOW_RAM,
    OPT_MEM_FAST_RAM,
    OPT_MEM_EXT_START,
    OPT_MEM_SAVE_ROMS,
    OPT_MEM_SLOW_RAM_DELAY,
    OPT_MEM_SLOW_RAM_MIRROR,
    OPT_MEM_BANKMAP,
    OPT_MEM_UNMAPPING_TYPE,
    OPT_MEM_RAM_INIT_PATTERN,

    // Disk controller
    OPT_DC_SPEED,
    OPT_DC_LOCK_DSKSYNC,
    OPT_DC_AUTO_DSKSYNC,

    // Floppy Drives
    OPT_DRIVE_CONNECT,
    OPT_DRIVE_TYPE,
    OPT_DRIVE_MECHANICS,
    OPT_DRIVE_RPM,
    OPT_DRIVE_SWAP_DELAY,
    OPT_DRIVE_PAN,
    OPT_DRIVE_STEP_VOLUME,
    OPT_DRIVE_POLL_VOLUME,
    OPT_DRIVE_INSERT_VOLUME,
    OPT_DRIVE_EJECT_VOLUME,

    // Hard drive controllers
    OPT_HDC_CONNECT,

    // Hard drives
    OPT_HDR_TYPE,
    OPT_HDR_WRITE_THROUGH,
    OPT_HDR_PAN,
    OPT_HDR_STEP_VOLUME,

    // Ports
    OPT_SER_DEVICE,
    OPT_SER_VERBOSE,

    // Blitter
    OPT_BLITTER_ACCURACY,

    // CIAs
    OPT_CIA_REVISION,
    OPT_CIA_TODBUG,
    OPT_CIA_ECLOCK_SYNCING,
    OPT_CIA_IDLE_SLEEP,

    // Keyboard
    OPT_KBD_ACCURACY,

    // Mouse
    OPT_MOUSE_PULLUP_RESISTORS,
    OPT_MOUSE_SHAKE_DETECTION,
    OPT_MOUSE_VELOCITY,

    // Joystick
    OPT_JOY_AUTOFIRE,           ///< Autofire status [on/off]
    OPT_JOY_AUTOFIRE_BURSTS,    ///< Burst mode [on/off]
    OPT_JOY_AUTOFIRE_BULLETS,   ///< Number of bullets per burst
    OPT_JOY_AUTOFIRE_DELAY,     ///< Delay between two button events [frames]

    // Paula audio
    OPT_AUD_SAMPLING_METHOD,
    OPT_AUD_FILTER_TYPE,
    OPT_AUD_PAN0,
    OPT_AUD_PAN1,
    OPT_AUD_PAN2,
    OPT_AUD_PAN3,
    OPT_AUD_VOL0,
    OPT_AUD_VOL1,
    OPT_AUD_VOL2,
    OPT_AUD_VOL3,
    OPT_AUD_VOLL,
    OPT_AUD_VOLR,
    OPT_AUD_FASTPATH,

    // Expansion boards
    OPT_DIAG_BOARD,

    // Remote servers
    OPT_SRV_PORT,
    OPT_SRV_PROTOCOL,
    OPT_SRV_AUTORUN,
    OPT_SRV_VERBOSE
};
typedef OPT Option;

#ifdef __cplusplus
struct OptionEnum : vamiga::util::Reflection<OptionEnum, Option>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = OPT_SRV_VERBOSE;

    static const char *prefix() { return "OPT"; }
    static const char *_key(long value)
    {
        switch (value) {

            case OPT_HOST_REFRESH_RATE:         return "HOST.REFRESH_RATE";
            case OPT_HOST_SAMPLE_RATE:          return "HOST.SAMPLE_RATE";
            case OPT_HOST_FRAMEBUF_WIDTH:       return "HOST.FRAMEBUF_WIDTH";
            case OPT_HOST_FRAMEBUF_HEIGHT:      return "HOST.FRAMEBUF_HEIGHT";

            case OPT_AMIGA_VIDEO_FORMAT:        return "AMIGA.VIDEO_FORMAT";
            case OPT_AMIGA_WARP_BOOT:           return "AMIGA.WARP_BOOT";
            case OPT_AMIGA_WARP_MODE:           return "AMIGA.WARP_MODE";
            case OPT_AMIGA_VSYNC:               return "AMIGA.VSYNC";
            case OPT_AMIGA_SPEED_BOOST:         return "AMIGA.SPEED_BOOST";
            case OPT_AMIGA_RUN_AHEAD:           return "AMIGA.RUN_AHEAD";

            case OPT_AMIGA_SNAP_AUTO:           return "AMIGA.SNAP_AUTO";
            case OPT_AMIGA_SNAP_DELAY:          return "AMIGA.SNAP_DELAY";
            case OPT_AMIGA_SNAP_COMPRESS:       return "AMIGA.SNAP_COMPRESS";

            case OPT_AGNUS_REVISION:            return "AGNUS.REVISION";
            case OPT_AGNUS_PTR_DROPS:           return "AGNUS.PTR_DROPS";

            case OPT_DENISE_REVISION:           return "DENISE.REVISION";
            case OPT_DENISE_VIEWPORT_TRACKING:  return "DENISE.VIEWPORT_TRACKING";
            case OPT_DENISE_FRAME_SKIPPING:     return "DENISE.FRAME_SKIPPING";
            case OPT_DENISE_HIDDEN_BITPLANES:   return "HIDDEN_BITPLANES";
            case OPT_DENISE_HIDDEN_SPRITES:     return "HIDDEN_SPRITES";
            case OPT_DENISE_HIDDEN_LAYERS:      return "HIDDEN_LAYERS";
            case OPT_DENISE_HIDDEN_LAYER_ALPHA: return "HIDDEN_LAYER_ALPHA";
            case OPT_DENISE_CLX_SPR_SPR:        return "CLX_SPR_SPR";
            case OPT_DENISE_CLX_SPR_PLF:        return "CLX_SPR_PLF";
            case OPT_DENISE_CLX_PLF_PLF:        return "CLX_PLF_PLF";

            case OPT_MON_PALETTE:               return "MON.PALETTE";
            case OPT_MON_BRIGHTNESS:            return "MON.BRIGHTNESS";
            case OPT_MON_CONTRAST:              return "MON.CONTRAST";
            case OPT_MON_SATURATION:            return "MON.SATURATION";

            case OPT_DMA_DEBUG_ENABLE:          return "DMA.DEBUG_ENABLE";
            case OPT_DMA_DEBUG_MODE:            return "DMA.DEBUG_MODE";
            case OPT_DMA_DEBUG_OPACITY:         return "DMA.DEBUG_OPACITY";
            case OPT_DMA_DEBUG_CHANNEL0:        return "DMA.DEBUG_CHANNEL0";
            case OPT_DMA_DEBUG_CHANNEL1:        return "DMA.DEBUG_CHANNEL1";
            case OPT_DMA_DEBUG_CHANNEL2:        return "DMA.DEBUG_CHANNEL2";
            case OPT_DMA_DEBUG_CHANNEL3:        return "DMA.DEBUG_CHANNEL3";
            case OPT_DMA_DEBUG_CHANNEL4:        return "DMA.DEBUG_CHANNEL4";
            case OPT_DMA_DEBUG_CHANNEL5:        return "DMA.DEBUG_CHANNEL5";
            case OPT_DMA_DEBUG_CHANNEL6:        return "DMA.DEBUG_CHANNEL6";
            case OPT_DMA_DEBUG_CHANNEL7:        return "DMA.DEBUG_CHANNEL7";
            case OPT_DMA_DEBUG_COLOR0:          return "DMA.DEBUG_COLOR0";
            case OPT_DMA_DEBUG_COLOR1:          return "DMA.DEBUG_COLOR1";
            case OPT_DMA_DEBUG_COLOR2:          return "DMA.DEBUG_COLOR2";
            case OPT_DMA_DEBUG_COLOR3:          return "DMA.DEBUG_COLOR3";
            case OPT_DMA_DEBUG_COLOR4:          return "DMA.DEBUG_COLOR4";
            case OPT_DMA_DEBUG_COLOR5:          return "DMA.DEBUG_COLOR5";
            case OPT_DMA_DEBUG_COLOR6:          return "DMA.DEBUG_COLOR6";
            case OPT_DMA_DEBUG_COLOR7:          return "DMA.DEBUG_COLOR7";

            case OPT_VID_WHITE_NOISE:           return "VID.WHITE_NOISE";

            case OPT_CPU_REVISION:              return "CPU.REVISION";
            case OPT_CPU_DASM_REVISION:         return "CPU.DASM_REVISION";
            case OPT_CPU_DASM_SYNTAX:           return "CPU.DASM_SYNTAX";
            case OPT_CPU_OVERCLOCKING:          return "CPU.OVERCLOCKING";
            case OPT_CPU_RESET_VAL:             return "CPU.RESET_VAL";

            case OPT_RTC_MODEL:                 return "RTC.MODEL";

            case OPT_MEM_CHIP_RAM:              return "MEM.CHIP_RAM";
            case OPT_MEM_SLOW_RAM:              return "MEM.SLOW_RAM";
            case OPT_MEM_FAST_RAM:              return "MEM.FAST_RAM";
            case OPT_MEM_EXT_START:             return "MEM.EXT_START";
            case OPT_MEM_SAVE_ROMS:             return "MEM.SAVE_ROMS";
            case OPT_MEM_SLOW_RAM_DELAY:        return "MEM.SLOW_RAM_DELAY";
            case OPT_MEM_SLOW_RAM_MIRROR:       return "MEM.SLOW_RAM_MIRROR";
            case OPT_MEM_BANKMAP:               return "MEM.BANKMAP";
            case OPT_MEM_UNMAPPING_TYPE:        return "MEM.UNMAPPING_TYPE";
            case OPT_MEM_RAM_INIT_PATTERN:      return "MEM.RAM_INIT_PATTERN";

            case OPT_DC_SPEED:                  return "DC.SPEED";
            case OPT_DC_LOCK_DSKSYNC:           return "DC.LOCK_DSKSYNC";
            case OPT_DC_AUTO_DSKSYNC:           return "DC.AUTO_DSKSYNC";

            case OPT_DRIVE_CONNECT:             return "DRIVE.CONNECT";
            case OPT_DRIVE_TYPE:                return "DRIVE.TYPE";
            case OPT_DRIVE_MECHANICS:           return "DRIVE.MECHANICS";
            case OPT_DRIVE_RPM:                 return "DRIVE.RPM";
            case OPT_DRIVE_SWAP_DELAY:          return "DRIVE.SWAP_DELAY";
            case OPT_DRIVE_PAN:                 return "DRIVE.PAN";
            case OPT_DRIVE_STEP_VOLUME:         return "DRIVE.STEP_VOLUME";
            case OPT_DRIVE_POLL_VOLUME:         return "DRIVE.POLL_VOLUME";
            case OPT_DRIVE_INSERT_VOLUME:       return "DRIVE.INSERT_VOLUME";
            case OPT_DRIVE_EJECT_VOLUME:        return "DRIVE.EJECT_VOLUME";

            case OPT_HDC_CONNECT:               return "HDC.CONNECT";

            case OPT_HDR_TYPE:                  return "HDR.TYPE";
            case OPT_HDR_WRITE_THROUGH:         return "HDR.WRITE_THROUGH";
            case OPT_HDR_PAN:                   return "HDR.PAN";
            case OPT_HDR_STEP_VOLUME:           return "HDR.STEP_VOLUME";

            case OPT_SER_DEVICE:                return "SER.DEVICE";
            case OPT_SER_VERBOSE:               return "SER.VERBOSE";

            case OPT_BLITTER_ACCURACY:          return "BLITTER.ACCURACY";

            case OPT_CIA_REVISION:              return "CIA.REVISION";
            case OPT_CIA_TODBUG:                return "CIA.TODBUG";
            case OPT_CIA_ECLOCK_SYNCING:        return "CIA.ECLOCK_SYNCING";
            case OPT_CIA_IDLE_SLEEP:            return "CIA.IDLE_SLEEP";

            case OPT_KBD_ACCURACY:              return "KBD.ACCURACY";

            case OPT_MOUSE_PULLUP_RESISTORS:    return "MOUSE.PULLUP_RESISTORS";
            case OPT_MOUSE_SHAKE_DETECTION:     return "MOUSE.SHAKE_DETECTION";
            case OPT_MOUSE_VELOCITY:            return "MOUSE.VELOCITY";

            case OPT_JOY_AUTOFIRE:              return "JOY.AUTOFIRE";
            case OPT_JOY_AUTOFIRE_BURSTS:       return "JOY.AUTOFIRE_BURSTS";
            case OPT_JOY_AUTOFIRE_BULLETS:      return "JOY.AUTOFIRE_BULLETS";
            case OPT_JOY_AUTOFIRE_DELAY:        return "JOY.AUTOFIRE_DELAY";

            case OPT_AUD_SAMPLING_METHOD:       return "AUD.SAMPLING_METHOD";
            case OPT_AUD_FILTER_TYPE:           return "AUD.FILTER_TYPE";
            case OPT_AUD_PAN0:                  return "AUD.PAN0";
            case OPT_AUD_PAN1:                  return "AUD.PAN1";
            case OPT_AUD_PAN2:                  return "AUD.PAN2";
            case OPT_AUD_PAN3:                  return "AUD.PAN3";
            case OPT_AUD_VOL0:                  return "AUD.VOL0";
            case OPT_AUD_VOL1:                  return "AUD.VOL1";
            case OPT_AUD_VOL2:                  return "AUD.VOL2";
            case OPT_AUD_VOL3:                  return "AUD.VOL3";
            case OPT_AUD_VOLL:                  return "AUD.VOLL";
            case OPT_AUD_VOLR:                  return "AUD.VOLR";
            case OPT_AUD_FASTPATH:              return "AUD.FASTPATH";

            case OPT_DIAG_BOARD:                return "DIAG_BOARD";

            case OPT_SRV_PORT:                  return "SRV.PORT";
            case OPT_SRV_PROTOCOL:              return "SRV.PROTOCOL";
            case OPT_SRV_AUTORUN:               return "SRV.AUTORUN";
            case OPT_SRV_VERBOSE:               return "SRV.VERBOSE";
        }
        return "???";
    }

    static const char *help(Option value)
    {
        switch (value) {

            case OPT_HOST_REFRESH_RATE:         return "Host video refresh rate";
            case OPT_HOST_SAMPLE_RATE:          return "Host audio sample rate";
            case OPT_HOST_FRAMEBUF_WIDTH:       return "Window width";
            case OPT_HOST_FRAMEBUF_HEIGHT:      return "Window height";

            case OPT_AMIGA_VIDEO_FORMAT:        return "Video format";
            case OPT_AMIGA_WARP_BOOT:           return "Warp-boot duration";
            case OPT_AMIGA_WARP_MODE:           return "Warp activation";
            case OPT_AMIGA_VSYNC:               return "VSYNC mode";
            case OPT_AMIGA_SPEED_BOOST:         return "Speed adjustment";
            case OPT_AMIGA_RUN_AHEAD:           return "Run-ahead frames";

            case OPT_AMIGA_SNAP_AUTO:           return "Automatically take snapshots";
            case OPT_AMIGA_SNAP_DELAY:          return "Time span between two snapshots";
            case OPT_AMIGA_SNAP_COMPRESS:       return "Compress snapshot data";

            case OPT_AGNUS_REVISION:            return "Chip revision";
            case OPT_AGNUS_PTR_DROPS:           return "Ignore certain register writes";

            case OPT_DENISE_REVISION:           return "Chip revision";
            case OPT_DENISE_VIEWPORT_TRACKING:  return "Track the currently used viewport";
            case OPT_DENISE_FRAME_SKIPPING:     return "Reduce frame rate in warp mode";
            case OPT_DENISE_HIDDEN_BITPLANES:   return "Hide bitplanes";
            case OPT_DENISE_HIDDEN_SPRITES:     return "Hide sprites";
            case OPT_DENISE_HIDDEN_LAYERS:      return "Hide playfields";
            case OPT_DENISE_HIDDEN_LAYER_ALPHA: return "Hidden playfield opacity";
            case OPT_DENISE_CLX_SPR_SPR:        return "Detect sprite-sprite collisions";
            case OPT_DENISE_CLX_SPR_PLF:        return "Detect sprite-playfield collisions";
            case OPT_DENISE_CLX_PLF_PLF:        return "Detect playfield-playfield collisions";

            case OPT_MON_PALETTE:               return "Color palette";
            case OPT_MON_BRIGHTNESS:            return "Monitor brightness";
            case OPT_MON_CONTRAST:              return "Monitor contrast";
            case OPT_MON_SATURATION:            return "Monitor saturation";

            case OPT_DMA_DEBUG_ENABLE:          return "DMA Debugger";
            case OPT_DMA_DEBUG_MODE:            return "DMA Debugger style";
            case OPT_DMA_DEBUG_OPACITY:         return "Opacity";
            case OPT_DMA_DEBUG_CHANNEL0:        return "Copper DMA";
            case OPT_DMA_DEBUG_CHANNEL1:        return "Blitter DMA";
            case OPT_DMA_DEBUG_CHANNEL2:        return "Disk DMA";
            case OPT_DMA_DEBUG_CHANNEL3:        return "Audio DMA";
            case OPT_DMA_DEBUG_CHANNEL4:        return "Sprite DMA";
            case OPT_DMA_DEBUG_CHANNEL5:        return "Bitplane DMA";
            case OPT_DMA_DEBUG_CHANNEL6:        return "CPU DMA";
            case OPT_DMA_DEBUG_CHANNEL7:        return "Memory Refresh DMA";
            case OPT_DMA_DEBUG_COLOR0:          return "Copper color";
            case OPT_DMA_DEBUG_COLOR1:          return "Blitter color";
            case OPT_DMA_DEBUG_COLOR2:          return "Disk color";
            case OPT_DMA_DEBUG_COLOR3:          return "Audio color";
            case OPT_DMA_DEBUG_COLOR4:          return "Sprite color";
            case OPT_DMA_DEBUG_COLOR5:          return "Bitplane color";
            case OPT_DMA_DEBUG_COLOR6:          return "CPU color";
            case OPT_DMA_DEBUG_COLOR7:          return "Memory refresh color";

            case OPT_VID_WHITE_NOISE:           return "White noise";

            case OPT_CPU_REVISION:              return "Chip revision";
            case OPT_CPU_DASM_REVISION:         return "Chip revision (disassembler)";
            case OPT_CPU_DASM_SYNTAX:           return "Disassembler syntax";
            case OPT_CPU_OVERCLOCKING:          return "Overclocking factor";
            case OPT_CPU_RESET_VAL:             return "Register reset value";

            case OPT_RTC_MODEL:                 return "Chip revision";

            case OPT_MEM_CHIP_RAM:              return "Chip RAM size";
            case OPT_MEM_SLOW_RAM:              return "Slow RAM size";
            case OPT_MEM_FAST_RAM:              return "Fast RAM size";
            case OPT_MEM_EXT_START:             return "Extension ROM location";
            case OPT_MEM_SAVE_ROMS:             return "Include ROMs in snapshots";
            case OPT_MEM_SLOW_RAM_DELAY:        return "Emulate slow RAM bus delays";
            case OPT_MEM_SLOW_RAM_MIRROR:       return "ECS memory mirroring";
            case OPT_MEM_BANKMAP:               return "Memory mapping scheme";
            case OPT_MEM_UNMAPPING_TYPE:        return "Unmapped memory behavior";
            case OPT_MEM_RAM_INIT_PATTERN:      return "RAM initialization pattern";

            case OPT_DC_SPEED:                  return "Drive speed";
            case OPT_DC_LOCK_DSKSYNC:           return "Ignore writes to DSKSYNC";
            case OPT_DC_AUTO_DSKSYNC:           return "Always find a sync mark";

            case OPT_DRIVE_CONNECT:             return "Connected";
            case OPT_DRIVE_TYPE:                return "Drive model";
            case OPT_DRIVE_MECHANICS:           return "Emulate drive mechanics";
            case OPT_DRIVE_RPM:                 return "Disk rotation speed";
            case OPT_DRIVE_SWAP_DELAY:          return "Disk swap delay";
            case OPT_DRIVE_PAN:                 return "Pan";
            case OPT_DRIVE_STEP_VOLUME:         return "Head step volume";
            case OPT_DRIVE_POLL_VOLUME:         return "Disk polling step volume";
            case OPT_DRIVE_INSERT_VOLUME:       return "Disk insertion volume";
            case OPT_DRIVE_EJECT_VOLUME:        return "Disk ejection volume";

            case OPT_HDC_CONNECT:               return "Connected";

            case OPT_HDR_TYPE:                  return "Drive model";
            case OPT_HDR_WRITE_THROUGH:         return "Keep data alive";
            case OPT_HDR_PAN:                   return "Pan";
            case OPT_HDR_STEP_VOLUME:           return "Head step volume";

            case OPT_SER_DEVICE:                return "Serial device type";
            case OPT_SER_VERBOSE:               return "Verbose";

            case OPT_BLITTER_ACCURACY:          return "Blitter accuracy level";

            case OPT_CIA_REVISION:              return "Chip revision";
            case OPT_CIA_TODBUG:                return "Emulate TOD bug";
            case OPT_CIA_ECLOCK_SYNCING:        return "Sync accesses with E-clock";
            case OPT_CIA_IDLE_SLEEP:            return "Enter idle state while not in use";

            case OPT_KBD_ACCURACY:              return "Keyboard emulation accuracy";

            case OPT_MOUSE_PULLUP_RESISTORS:    return "Emulate pullup resistors";
            case OPT_MOUSE_SHAKE_DETECTION:     return "Detect a shaked mouse";
            case OPT_MOUSE_VELOCITY:            return "Mouse velocity";

            case OPT_JOY_AUTOFIRE:              return "Autofire";
            case OPT_JOY_AUTOFIRE_BURSTS:       return "Burst mode";
            case OPT_JOY_AUTOFIRE_BULLETS:      return "Number of bullets per burst";
            case OPT_JOY_AUTOFIRE_DELAY:        return "Autofire delay in frames";

            case OPT_AUD_SAMPLING_METHOD:       return "Sampling method";
            case OPT_AUD_FILTER_TYPE:           return "Audio filter type";
            case OPT_AUD_PAN0:                  return "Channel 0 pan";
            case OPT_AUD_PAN1:                  return "Channel 1 pan";
            case OPT_AUD_PAN2:                  return "Channel 2 pan";
            case OPT_AUD_PAN3:                  return "Channel 3 pan";
            case OPT_AUD_VOL0:                  return "Channel 0 volume";
            case OPT_AUD_VOL1:                  return "Channel 1 volume";
            case OPT_AUD_VOL2:                  return "Channel 2 volume";
            case OPT_AUD_VOL3:                  return "Channel 3 volume";
            case OPT_AUD_VOLL:                  return "Master volume (left)";
            case OPT_AUD_VOLR:                  return "Master volume (right)";
            case OPT_AUD_FASTPATH:              return "Boost performance";

            case OPT_DIAG_BOARD:                return "Diagnose board";

            case OPT_SRV_PORT:                  return "Server port";
            case OPT_SRV_PROTOCOL:              return "Server protocol";
            case OPT_SRV_AUTORUN:               return "Auto run";
            case OPT_SRV_VERBOSE:               return "Verbose mode";
        }
        return "???";
    }
};
#endif
