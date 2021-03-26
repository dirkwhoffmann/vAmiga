// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Enumerations
//

enum_long(OPT)
{
    // Agnus
    OPT_AGNUS_REVISION,
    OPT_SLOW_RAM_MIRROR,
    
    // Denise
    OPT_DENISE_REVISION,
    
    // Pixel engine
    OPT_PALETTE,
    OPT_BRIGHTNESS,
    OPT_CONTRAST,
    OPT_SATURATION,
    
    // Real-time clock
    OPT_RTC_MODEL,

    // Memory
    OPT_CHIP_RAM,
    OPT_SLOW_RAM,
    OPT_FAST_RAM,
    OPT_EXT_START,
    OPT_SLOW_RAM_DELAY,
    OPT_BANKMAP,
    OPT_UNMAPPING_TYPE,
    OPT_RAM_INIT_PATTERN,
    
    // Disk controller
    OPT_DRIVE_CONNECT,
    OPT_DRIVE_SPEED,
    OPT_LOCK_DSKSYNC,
    OPT_AUTO_DSKSYNC,

    // Drives
    OPT_DRIVE_TYPE,
    OPT_EMULATE_MECHANICS,
    OPT_DRIVE_PAN,
    OPT_STEP_VOLUME,
    OPT_POLL_VOLUME,
    OPT_INSERT_VOLUME,
    OPT_EJECT_VOLUME,
    OPT_DEFAULT_FILESYSTEM,
    OPT_DEFAULT_BOOTBLOCK,
    
    // Ports
    OPT_SERIAL_DEVICE,

    // Compatibility
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
    
    // Keyboard
    OPT_ACCURATE_KEYBOARD,
    
    // Mouse
    OPT_PULLUP_RESISTORS,
    OPT_SHAKE_DETECTION,
    OPT_MOUSE_VELOCITY,
    
    // Paula audio
    OPT_SAMPLING_METHOD,
    OPT_FILTER_TYPE,
    OPT_FILTER_ALWAYS_ON,
    OPT_AUDPAN,
    OPT_AUDVOL,
    OPT_AUDVOLL,
    OPT_AUDVOLR,
    
    OPT_COUNT
};
typedef OPT Option;

enum_long(EMULATOR_STATE)
{
    EMULATOR_STATE_OFF,
    EMULATOR_STATE_PAUSED,
    EMULATOR_STATE_RUNNING,

    EMULATOR_STATE_COUNT
};
typedef EMULATOR_STATE EmulatorState;
