#pragma once

#include "Reflection.hpp"

namespace va {

#include "Option.h"

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
            case OPT_DRIVE_PAN:           return "DRIVE_PAN";
            case OPT_STEP_VOLUME:         return "STEP_VOLUME";
            case OPT_POLL_VOLUME:         return "POLL_VOLUME";
            case OPT_INSERT_VOLUME:       return "INSERT_VOLUME";
            case OPT_EJECT_VOLUME:        return "EJECT_VOLUME";
            case OPT_DEFAULT_FILESYSTEM:  return "DEFAULT_FILESYSTEM";
            case OPT_DEFAULT_BOOTBLOCK:   return "DEFAULT_BOOTBLOCK";
                
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

            case OPT_PULLUP_RESISTORS:    return "PULLUP_RESISTORS";
            case OPT_MOUSE_VELOCITY:      return "MOUSE_VELOCITY";

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

}
