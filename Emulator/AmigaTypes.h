// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AMIGA_TYPES_H
#define _AMIGA_TYPES_H

#include "Aliases.h"

#include "AgnusTypes.h"
#include "CPUTypes.h"
#include "CIATypes.h"
#include "DeniseTypes.h"
#include "DiskTypes.h"
#include "DmaDebuggerTypes.h"
#include "DriveTypes.h"
#include "EventHandlerTypes.h"
#include "FileTypes.h"
#include "FSTypes.h"
#include "KeyboardTypes.h"
#include "MemoryTypes.h"
#include "MessageQueueTypes.h"
#include "PaulaTypes.h"
#include "PortTypes.h"
#include "RTCTypes.h"

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
    OPT_BRDRBLNK,
    
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
    OPT_ACCURATE_KEYBOARD,
    
    // Paula audio
    OPT_SAMPLING_METHOD,
    OPT_FILTER_TYPE,
    OPT_FILTER_ALWAYS_ON,
    OPT_AUDPAN,
    OPT_AUDVOL,
    OPT_AUDVOLL,
    OPT_AUDVOLR
};
typedef OPT Option;

inline bool isOption(long value)
{
    return (unsigned long)value <= OPT_AUDVOLR;
}

inline const char *OptionName(Option value)
{
    switch (value) {
            
        case OPT_AGNUS_REVISION:      return "AGNUS_REVISION";
        case OPT_SLOW_RAM_MIRROR:     return "SLOW_RAM_MIRROR";
            
        case OPT_DENISE_REVISION:     return "DENISE_REVISION";
        case OPT_BRDRBLNK:            return "BRDRBLNK";
            
        case OPT_RTC_MODEL:           return "RTC_MODEL";

        case OPT_CHIP_RAM:            return "CHIP_RAM";
        case OPT_SLOW_RAM:            return "SLOW_RAM";
        case OPT_FAST_RAM:            return "FAST_RAM";
        case OPT_EXT_START:           return "EXT_START";
        case OPT_SLOW_RAM_DELAY:      return "SLOW_RAM_DELAY";
        case OPT_BANKMAP:             return "BANKMAP";
        case OPT_UNMAPPING_TYPE:      return "UNMAPPING_TYPE";
        case OPT_RAM_INIT_PATTERN:    return "RAM_INIT_PATTERN";
            
        case OPT_DRIVE_CONNECT:       return "DRIVE_CONNECT";
        case OPT_DRIVE_SPEED:         return "DRIVE_SPEED";
        case OPT_LOCK_DSKSYNC:        return "LOCK_DSKSYNC";
        case OPT_AUTO_DSKSYNC:        return "AUTO_DSKSYNC";

        case OPT_DRIVE_TYPE:          return "DRIVE_TYPE";
        case OPT_EMULATE_MECHANICS:   return "EMULATE_MECHANICS";
            
        case OPT_SERIAL_DEVICE:       return "SERIAL_DEVICE";

        case OPT_HIDDEN_SPRITES:      return "HIDDEN_SPRITES";
        case OPT_HIDDEN_LAYERS:       return "HIDDEN_LAYERS";
        case OPT_HIDDEN_LAYER_ALPHA:  return "HIDDEN_LAYER_ALPHA";
        case OPT_CLX_SPR_SPR:         return "CLX_SPR_SPR";
        case OPT_CLX_SPR_PLF:         return "CLX_SPR_PLF";
        case OPT_CLX_PLF_PLF:         return "CLX_PLF_PLF";
                
        case OPT_BLITTER_ACCURACY:    return "BLITTER_ACCURACY";
            
        case OPT_CIA_REVISION:        return "CIA_REVISION";
        case OPT_TODBUG:              return "TODBUG";
        case OPT_ECLOCK_SYNCING:      return "ECLOCK_SYNCING";
        case OPT_ACCURATE_KEYBOARD:   return "ACCURATE_KEYBOARD";
            
        case OPT_SAMPLING_METHOD:     return "SAMPLING_METHOD";
        case OPT_FILTER_TYPE:         return "FILTER_TYPE";
        case OPT_FILTER_ALWAYS_ON:    return "FILTER_ALWAYS_ON";
        case OPT_AUDPAN:              return "AUDPAN";
        case OPT_AUDVOL:              return "AUDVOL";
        case OPT_AUDVOLL:             return "AUDVOLL";
        case OPT_AUDVOLR:             return "AUDVOLR";
    }
    return "???";
}

enum_long(EMULATOR_STATE)
{
    EMULATOR_STATE_OFF,
    EMULATOR_STATE_PAUSED,
    EMULATOR_STATE_RUNNING
};
typedef EMULATOR_STATE EmulatorState;

inline bool isEmulatorState(long value) {
    return (unsigned long)value <=  EMULATOR_STATE_RUNNING;
}

inline const char *EmulatorStateName(EmulatorState value)
{
    switch (value) {
            
        case EMULATOR_STATE_OFF:      return "OFF";
        case EMULATOR_STATE_PAUSED:   return "PAUSED";
        case EMULATOR_STATE_RUNNING:  return "RUNNING";
    }
    return "???";
}

enum_long(ErrorCode)
{
    ERR_OK,
    ERR_ROM_MISSING,
    ERR_AROS_NO_EXTROM,
    ERR_AROS_RAM_LIMIT,
    ERR_CHIP_RAM_LIMIT
};

//
// Structures
//

typedef struct
{
    int cpuSpeed;
    CIAConfig ciaA;
    CIAConfig ciaB;
    RTCConfig rtc;
    MuxerConfig audio;
    MemoryConfig mem;
    AgnusConfig agnus;
    DeniseConfig denise;
    BlitterConfig blitter;
    SerialPortConfig serialPort;
    KeyboardConfig keyboard;
    DiskControllerConfig diskController;
    DriveConfig df0;
    DriveConfig df1;
    DriveConfig df2;
    DriveConfig df3;
}
AmigaConfiguration;

typedef struct
{
    Cycle cpuClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
AmigaInfo;

#endif
