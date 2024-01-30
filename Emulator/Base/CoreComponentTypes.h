// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(OPT)
{
    // Amiga
    OPT_VIDEO_FORMAT,
    OPT_WARP_BOOT,
    OPT_WARP_MODE,
    OPT_SYNC_MODE,
    OPT_VSYNC,
    OPT_TIME_LAPSE,
    OPT_TIME_SLICES,

    // Agnus
    OPT_AGNUS_REVISION,
    OPT_SLOW_RAM_MIRROR,
    OPT_PTR_DROPS,

    // Denise
    OPT_DENISE_REVISION,
    OPT_VIEWPORT_TRACKING,
    OPT_FRAME_SKIPPING,

    // Pixel engine
    OPT_PALETTE,
    OPT_BRIGHTNESS,
    OPT_CONTRAST,
    OPT_SATURATION,
    
    // DMA Debugger
    OPT_DMA_DEBUG_ENABLE,
    OPT_DMA_DEBUG_MODE,
    OPT_DMA_DEBUG_OPACITY,
    OPT_DMA_DEBUG_CHANNEL,
    OPT_DMA_DEBUG_COLOR,
    
    // CPU
    OPT_CPU_REVISION,
    OPT_CPU_DASM_REVISION,
    OPT_CPU_DASM_SYNTAX,
    OPT_CPU_OVERCLOCKING,
    OPT_CPU_RESET_VAL,

    // Real-time clock
    OPT_RTC_MODEL,

    // Memory
    OPT_CHIP_RAM,
    OPT_SLOW_RAM,
    OPT_FAST_RAM,
    OPT_EXT_START,
    OPT_SAVE_ROMS,
    OPT_SLOW_RAM_DELAY,
    OPT_BANKMAP,
    OPT_UNMAPPING_TYPE,
    OPT_RAM_INIT_PATTERN,
    
    // Disk controller
    OPT_DRIVE_CONNECT,
    OPT_DRIVE_SPEED,
    OPT_LOCK_DSKSYNC,
    OPT_AUTO_DSKSYNC,

    // Floppy Drives
    OPT_DRIVE_TYPE,
    OPT_DRIVE_MECHANICS,
    OPT_DRIVE_RPM,
    OPT_DISK_SWAP_DELAY,
    OPT_DRIVE_PAN,
    OPT_STEP_VOLUME,
    OPT_POLL_VOLUME,
    OPT_INSERT_VOLUME,
    OPT_EJECT_VOLUME,
    
    // Hard drive controllers
    OPT_HDC_CONNECT,

    // Hard drives
    OPT_HDR_TYPE,
    OPT_HDR_PAN,
    OPT_HDR_STEP_VOLUME,
    
    // Ports
    OPT_SER_DEVICE,
    OPT_SER_VERBOSE,

    // Compatibility
    OPT_HIDDEN_BITPLANES,
    OPT_HIDDEN_SPRITES,
    OPT_HIDDEN_LAYERS,
    OPT_HIDDEN_LAYER_ALPHA,
    OPT_CLX_SPR_SPR,
    OPT_CLX_SPR_PLF,
    OPT_CLX_PLF_PLF,
        
    // Blitter
    OPT_BLITTER_ACCURACY,
    
    // CIAs
    OPT_CIA_REVISION,
    OPT_TODBUG,
    OPT_ECLOCK_SYNCING,
    OPT_CIA_IDLE_SLEEP,

    // Keyboard
    OPT_ACCURATE_KEYBOARD,
    
    // Mouse
    OPT_PULLUP_RESISTORS,
    OPT_SHAKE_DETECTION,
    OPT_MOUSE_VELOCITY,
    
    // Joystick
    OPT_AUTOFIRE,
    OPT_AUTOFIRE_BULLETS,
    OPT_AUTOFIRE_DELAY,
    
    // Paula audio
    OPT_SAMPLING_METHOD,
    OPT_FILTER_TYPE,
    OPT_AUDPAN,
    OPT_AUDVOL,
    OPT_AUDVOLL,
    OPT_AUDVOLR,
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
struct OptionEnum : util::Reflection<OptionEnum, Option>
{    
    static constexpr long minVal = 0;
    static constexpr long maxVal = OPT_SRV_VERBOSE;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "OPT"; }
    static const char *key(Option value)
    {
        switch (value) {

            case OPT_VIDEO_FORMAT:          return "VIDEO_FORMAT";
            case OPT_WARP_BOOT:             return "WARP_BOOT";
            case OPT_WARP_MODE:             return "WARP_MODE";
            case OPT_SYNC_MODE:             return "SYNC_MODE";
            case OPT_VSYNC:                 return "VSYNC";
            case OPT_TIME_LAPSE:            return "TIME_LAPSE";
            case OPT_TIME_SLICES:           return "TIME_SLICES";

            case OPT_AGNUS_REVISION:        return "AGNUS_REVISION";
            case OPT_SLOW_RAM_MIRROR:       return "SLOW_RAM_MIRROR";
            case OPT_PTR_DROPS:             return "PTR_DROPS";
                
            case OPT_DENISE_REVISION:       return "DENISE_REVISION";
            case OPT_VIEWPORT_TRACKING:     return "VIEWPORT_TRACKING";
            case OPT_FRAME_SKIPPING:        return "OPT_FRAME_SKIPPING";
                
            case OPT_PALETTE:               return "PALETTE";
            case OPT_BRIGHTNESS:            return "BRIGHTNESS";
            case OPT_CONTRAST:              return "CONTRAST";
            case OPT_SATURATION:            return "SATURATION";

            case OPT_DMA_DEBUG_ENABLE:      return "DMA_DEBUG_ENABLE";
            case OPT_DMA_DEBUG_MODE:        return "DMA_DEBUG_MODE";
            case OPT_DMA_DEBUG_OPACITY:     return "DMA_DEBUG_OPACITY";
            case OPT_DMA_DEBUG_CHANNEL:     return "DMA_DEBUG_CHANNEL";
            case OPT_DMA_DEBUG_COLOR:       return "DMA_DEBUG_COLOR";

            case OPT_CPU_REVISION:          return "CPU_REVISION";
            case OPT_CPU_DASM_REVISION:     return "OPT_CPU_DASM_REVISION";
            case OPT_CPU_DASM_SYNTAX:       return "OPT_CPU_DASM_SYNTAX";
            case OPT_CPU_OVERCLOCKING:      return "CPU_OVERCLOCKING";
            case OPT_CPU_RESET_VAL:         return "CPU_RESET_VAL";

            case OPT_RTC_MODEL:             return "RTC_MODEL";

            case OPT_CHIP_RAM:              return "CHIP_RAM";
            case OPT_SLOW_RAM:              return "SLOW_RAM";
            case OPT_FAST_RAM:              return "FAST_RAM";
            case OPT_EXT_START:             return "EXT_START";
            case OPT_SAVE_ROMS:             return "SAVE_ROMS";
            case OPT_SLOW_RAM_DELAY:        return "SLOW_RAM_DELAY";
            case OPT_BANKMAP:               return "BANKMAP";
            case OPT_UNMAPPING_TYPE:        return "UNMAPPING_TYPE";
            case OPT_RAM_INIT_PATTERN:      return "RAM_INIT_PATTERN";
                
            case OPT_DRIVE_CONNECT:         return "DRIVE_CONNECT";
            case OPT_DRIVE_SPEED:           return "DRIVE_SPEED";
            case OPT_LOCK_DSKSYNC:          return "LOCK_DSKSYNC";
            case OPT_AUTO_DSKSYNC:          return "AUTO_DSKSYNC";

            case OPT_DRIVE_TYPE:            return "DRIVE_TYPE";
            case OPT_DRIVE_MECHANICS:       return "DRIVE_MECHANICS";
            case OPT_DRIVE_RPM:             return "DRIVE_RPM";
            case OPT_DISK_SWAP_DELAY:       return "DISK_SWAP_DELAY";
            case OPT_DRIVE_PAN:             return "DRIVE_PAN";
            case OPT_STEP_VOLUME:           return "STEP_VOLUME";
            case OPT_POLL_VOLUME:           return "POLL_VOLUME";
            case OPT_INSERT_VOLUME:         return "INSERT_VOLUME";
            case OPT_EJECT_VOLUME:          return "EJECT_VOLUME";
                
            case OPT_HDC_CONNECT:           return "HDC_CONNECT";

            case OPT_HDR_TYPE:              return "HDR_TYPE";
            case OPT_HDR_PAN:               return "HDR_PAN";
            case OPT_HDR_STEP_VOLUME:       return "HDR_STEP_VOLUME";

            case OPT_SER_DEVICE:            return "SER_DEVICE";
            case OPT_SER_VERBOSE:           return "SER_VERBOSE";

            case OPT_HIDDEN_BITPLANES:      return "HIDDEN_BITPLANES";
            case OPT_HIDDEN_SPRITES:        return "HIDDEN_SPRITES";
            case OPT_HIDDEN_LAYERS:         return "HIDDEN_LAYERS";
            case OPT_HIDDEN_LAYER_ALPHA:    return "HIDDEN_LAYER_ALPHA";
            case OPT_CLX_SPR_SPR:           return "CLX_SPR_SPR";
            case OPT_CLX_SPR_PLF:           return "CLX_SPR_PLF";
            case OPT_CLX_PLF_PLF:           return "CLX_PLF_PLF";
                    
            case OPT_BLITTER_ACCURACY:      return "BLITTER_ACCURACY";
                
            case OPT_CIA_REVISION:          return "CIA_REVISION";
            case OPT_TODBUG:                return "TODBUG";
            case OPT_ECLOCK_SYNCING:        return "ECLOCK_SYNCING";
            case OPT_CIA_IDLE_SLEEP:        return "CIA_IDLE_SLEEP";

            case OPT_ACCURATE_KEYBOARD:     return "ACCURATE_KEYBOARD";

            case OPT_PULLUP_RESISTORS:      return "PULLUP_RESISTORS";
            case OPT_SHAKE_DETECTION:       return "SHAKE_DETECTION";
            case OPT_MOUSE_VELOCITY:        return "MOUSE_VELOCITY";

            case OPT_AUTOFIRE:              return "AUTOFIRE";
            case OPT_AUTOFIRE_BULLETS:      return "AUTOFIRE_BULLETS";
            case OPT_AUTOFIRE_DELAY:        return "AUTOFIRE_DELAY";

            case OPT_SAMPLING_METHOD:       return "SAMPLING_METHOD";
            case OPT_FILTER_TYPE:           return "FILTER_TYPE";
            case OPT_AUDPAN:                return "AUDPAN";
            case OPT_AUDVOL:                return "AUDVOL";
            case OPT_AUDVOLL:               return "AUDVOLL";
            case OPT_AUDVOLR:               return "AUDVOLR";
            case OPT_AUD_FASTPATH:          return "AUD_FASTPATH";

            case OPT_DIAG_BOARD:            return "DIAG_BOARD";

            case OPT_SRV_PORT:              return "SRV_PORT";
            case OPT_SRV_PROTOCOL:          return "SRV_PROTOCOL";
            case OPT_SRV_AUTORUN:           return "SRV_AUTORUN";
            case OPT_SRV_VERBOSE:           return "SRV_VERBOSE";
        }
        return "???";
    }
};
#endif
