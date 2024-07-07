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

enum_long(COMP_TYPE)
{
    COMP_UNKNOWN,

    // Internal components
    COMP_AGNUS,
    COMP_AMIGA,
    COMP_AUDIO_FILTER,
    COMP_BLITTER,
    COMP_COPPER,
    COMP_COPPER_DEBUGGER,
    COMP_CIA,
    COMP_CONSOLE,
    COMP_CPU,
    COMP_DEBUGGER,
    COMP_DENISE,
    COMP_DENISE_DEBUGGER,
    COMP_DISK_CONTROLLER,
    COMP_DMA_DEBUGGER,
    COMP_MEM,
    COMP_OS_DEBUGGER,
    COMP_PAULA,
    COMP_PIXEL_ENGINE,
    COMP_RECORDER,
    COMP_REGRESSION_TESTER,
    COMP_REMOTE_MANAGER,
    COMP_REMOTE_SERVER,
    COMP_RETRO_SHELL,
    COMP_SEQUENCER,
    COMP_STATE_MACHINE,
    COMP_RTC,
    COMP_TOD,
    COMP_UART,
    COMP_ZORRO_BOARD,
    COMP_ZORRO_MANAGER,

    // Peripherals
    COMP_DRIVE,
    COMP_JOYSTICK,
    COMP_KEYBOARD,
    COMP_MOUSE,
    COMP_PADDLE,

    // Ports
    COMP_AUDIO_PORT,
    COMP_CONTROL_PORT,
    COMP_SERIAL_PORT,
    COMP_VIDEO_PORT
};
typedef COMP_TYPE CType;

#ifdef __cplusplus
struct CTypeEnum : util::Reflection<CTypeEnum, CType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = COMP_VIDEO_PORT;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "COMP"; }
    static const char *_key(long value)
    {
        switch (value) {

            case COMP_UNKNOWN:              return "UNKNOWN";
            case COMP_AGNUS:                return "AGNUS";
            case COMP_AMIGA:                return "AMIGA";
            case COMP_AUDIO_FILTER:         return "AUDIO_FILTER";
            case COMP_BLITTER:              return "BLITTER";
            case COMP_COPPER:               return "COPPER";
            case COMP_COPPER_DEBUGGER:      return "COPPER_DEBUGGER";
            case COMP_CIA:                  return "CIA";
            case COMP_CONSOLE:              return "CPU";
            case COMP_CPU:                  return "CPU";
            case COMP_DEBUGGER:             return "DEBUGGER";
            case COMP_DENISE:               return "DENISE";
            case COMP_DENISE_DEBUGGER:      return "DENISE_DEBUGGER";
            case COMP_DISK_CONTROLLER:      return "DISK_CONTROLLER";
            case COMP_DMA_DEBUGGER:         return "DMA_DEBUGGER";
            case COMP_MEM:                  return "MEM";
            case COMP_OS_DEBUGGER:          return "OS_DEBUGGER";
            case COMP_PAULA:                return "PAULA";
            case COMP_PIXEL_ENGINE:         return "PIXEL_ENGINE";
            case COMP_RECORDER:             return "RECORDER";
            case COMP_REGRESSION_TESTER:    return "REGRESSION_TESTER";
            case COMP_REMOTE_MANAGER:       return "REMOTE_MANAGER";
            case COMP_REMOTE_SERVER:        return "REMOTE_SERVER";
            case COMP_RETRO_SHELL:          return "RETRO_SHELL";
            case COMP_SEQUENCER:            return "SEQUENCER";
            case COMP_STATE_MACHINE:        return "STATE_MACHINE";
            case COMP_RTC:                  return "RTC";
            case COMP_TOD:                  return "TOD";
            case COMP_UART:                 return "UART";
            case COMP_ZORRO_BOARD:          return "ZORRO_BOARD";
            case COMP_ZORRO_MANAGER:        return "ZORRO_MANAGER";
            case COMP_DRIVE:                return "DRIVE";
            case COMP_JOYSTICK:             return "JOYSTICK";
            case COMP_KEYBOARD:             return "KEYBOARD";
            case COMP_MOUSE:                return "MOUSE";
            case COMP_PADDLE:               return "PADDLE";
            case COMP_AUDIO_PORT:           return "AUDIO_PORT";
            case COMP_CONTROL_PORT:         return "CONTROL_PORT";
            case COMP_SERIAL_PORT:          return "SERIAL_PORT";
            case COMP_VIDEO_PORT:           return "VIDEO_PORT";
        }
        return "???";
    }
};
#endif
