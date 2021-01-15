// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaPublicTypes.h"

#include "AgnusTypes.h"
#include "AudioTypes.h"
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
#include "MsgQueueTypes.h"
#include "PaulaTypes.h"
#include "PortTypes.h"
#include "RTCTypes.h"

//
// Reflection APIs
//

struct OptionEnum : Reflection<OptionEnum, Option> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < OPT_COUNT;
    }

    static const char *prefix() { return "OPT"; }
    static const char *key(Option value)
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
                
            case OPT_COUNT:               return "???";
        }
        return "???";
    }
};

struct EmulatorStateEnum : Reflection<EmulatorStateEnum, EmulatorState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < EMULATOR_STATE_COUNT;
    }

    static const char *prefix() { return "EMULATOR_STATE"; }
    static const char *key(EmulatorState value)
    {
        switch (value) {
                
            case EMULATOR_STATE_OFF:      return "OFF";
            case EMULATOR_STATE_PAUSED:   return "PAUSED";
            case EMULATOR_STATE_RUNNING:  return "RUNNING";
            case EMULATOR_STATE_COUNT:    return "???";
        }
        return "???";
    }
};

struct ErrorCodeEnum : Reflection<ErrorCodeEnum, ErrorCode> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ERROR_COUNT;
    }
    
    static const char *prefix() { return "ERROR"; }
    static const char *key(ErrorCode value)
    {
        switch (value) {
                
            case ERROR_OK:                  return "OK";
                
            case ERROR_OUT_OF_MEMORY:       return "OUT_OF_MEMORY";
            case ERROR_CHIP_RAM_LIMIT:      return "CHIP_RAM_LIMIT";
            case ERROR_AROS_RAM_LIMIT:      return "AROS_RAM_LIMIT";

            case ERROR_ROM_MISSING:         return "ROM_MISSING";
            case ERROR_AROS_NO_EXTROM:      return "AROS_NO_EXTROM";

            case ERROR_COUNT:               return "???";
        }
        return "???";
    }
};

//
// Private types
//

enum_u32(RunLoopControlFlag)
{
    RL_STOP               = 0b000001,
    RL_INSPECT            = 0b000010,
    RL_BREAKPOINT_REACHED = 0b000100,
    RL_WATCHPOINT_REACHED = 0b001000,
    RL_AUTO_SNAPSHOT      = 0b010000,
    RL_USER_SNAPSHOT      = 0b100000
};
