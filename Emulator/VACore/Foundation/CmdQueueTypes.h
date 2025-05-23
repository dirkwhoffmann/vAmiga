// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

// #include "VACore/Foundation/Types.h"
#include "OptionTypes.h"
#include "JoystickTypes.h"
#include "KeyboardTypes.h"

namespace vamiga {

//
// Enumerations
//

/// Emulator command
enum class Cmd : long
{
    NONE,                   ///< None
    
    // Emulator
    CONFIG,                 ///< Configure the emulator
    CONFIG_ALL,             ///< Configure the emulator
    HARD_RESET,             ///< Perform a hard reset
    SOFT_RESET,             ///< Perform a soft reset
    POWER_ON,               ///< Switch power on
    POWER_OFF,              ///< Switch power off
    RUN,                    ///< Start emulation
    PAUSE,                  ///< Pause emulation
    WARP_ON,                ///< Switch on warp mode
    WARP_OFF,               ///< Switch off warp mode
    HALT,                   ///< Terminate the emulator thread

    // Amiga
    ALARM_ABS,              ///< Schedule an alarm (absolute cycle)
    ALARM_REL,              ///< Schedule an alarm (relative cycle)
    INSPECTION_TARGET,      ///< Sets the auto-inspection mask
    
    // CPU (Breakpoints, Watchpoints, Catchpoints)
    GUARD_SET_AT,
    GUARD_REMOVE_NR,        ///< Remove the n-th guard
    GUARD_MOVE_NR,          ///< Change the address of a guard
    GUARD_IGNORE_NR,        ///< Change the ignore count of a guard
    GUARD_REMOVE_AT,        ///< Remove the guard at an address
    GUARD_REMOVE_ALL,       ///< Remove all guards
    GUARD_ENABLE_NR,        ///< Enable the n-th guard
    GUARD_ENABLE_AT,        ///< Enable the guard at an address
    GUARD_ENABLE_ALL,       ///< Enable all guards
    GUARD_DISABLE_NR,       ///< Disable the n-th guard
    GUARD_DISABLE_AT,       ///< Disable the guard at an address
    GUARD_DISABLE_ALL,      ///< Disable all guards
    
    // Keyboard
    KEY_PRESS,              ///< Press a key on the C64 keyboard
    KEY_RELEASE,            ///< Release a key on the C64 keyboard
    KEY_RELEASE_ALL,        ///< Clear the keyboard matrix
    KEY_TOGGLE,             ///< Press or release a key on the C64 keyboard
    
    // Mouse
    MOUSE_MOVE_ABS,         ///< Signal a mouse movement (absolute)
    MOUSE_MOVE_REL,         ///< Signal a mouse movement (relative)
    MOUSE_BUTTON,           ///< Signal a mouse button event
    
    // Joystick
    JOY_EVENT,              ///< Signal a joystick button event
    
    // Floppy disk
    DSK_TOGGLE_WP,          ///< Toggle write-protection
    DSK_MODIFIED,           ///< Signal that the disk has been saved
    DSK_UNMODIFIED,         ///< Signan that the disk needs saving
    
    // RetroShell
    RSH_EXECUTE,            ///< Execute a script command
    
    // Experimental
    FUNC,
    
    // Host machine
    FOCUS                   ///< The emulator windows got or lost focus
};

struct CmdTypeEnum : Reflection<CmdTypeEnum, Cmd> {
    
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Cmd::FOCUS);
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }
    
    static const char *_key(Cmd value)
    {
        switch (value) {
                
            case Cmd::NONE:                  return "NONE";
                
            case Cmd::CONFIG:                return "CONFIG";
            case Cmd::CONFIG_ALL:            return "CONFIG_ALL";
            case Cmd::HARD_RESET:            return "HARD_RESET";
            case Cmd::SOFT_RESET:            return "SOFT_RESET";
            case Cmd::POWER_ON:              return "POWER_ON";
            case Cmd::POWER_OFF:             return "POWER_OFF";
            case Cmd::RUN:                   return "RUN";
            case Cmd::PAUSE:                 return "PAUSE";
            case Cmd::WARP_ON:               return "WARP_ON";
            case Cmd::WARP_OFF:              return "WARP_OFF";
            case Cmd::HALT:                  return "HALT";

            case Cmd::ALARM_ABS:             return "ALARM_ABS";
            case Cmd::ALARM_REL:             return "ALARM_REL";
            case Cmd::INSPECTION_TARGET:     return "INSPECTION_TARGET";
                
            case Cmd::GUARD_SET_AT:          return "GUARD_SET_AT";
            case Cmd::GUARD_MOVE_NR:         return "GUARD_MOVE_TO";
            case Cmd::GUARD_IGNORE_NR:       return "GUARD_IGNORE_NR";
            case Cmd::GUARD_REMOVE_NR:       return "GUARD_REMOVE_NR";
            case Cmd::GUARD_REMOVE_AT:       return "GUARD_REMOVE_AT";
            case Cmd::GUARD_REMOVE_ALL:      return "GUARD_REMOVE_ALL";
            case Cmd::GUARD_ENABLE_NR:       return "GUARD_ENABLE_NR";
            case Cmd::GUARD_ENABLE_AT:       return "GUARD_ENABLE_AT";
            case Cmd::GUARD_ENABLE_ALL:      return "GUARD_ENABLE_ALL";
            case Cmd::GUARD_DISABLE_NR:      return "GUARD_DISABLE_NR";
            case Cmd::GUARD_DISABLE_AT:      return "GUARD_DISABLE_AT";
            case Cmd::GUARD_DISABLE_ALL:     return "GUARD_DISABLE_ALL";
                
            case Cmd::KEY_PRESS:             return "KEY_PRESS";
            case Cmd::KEY_RELEASE:           return "KEY_RELEASE";
            case Cmd::KEY_RELEASE_ALL:       return "KEY_RELEASE_ALL";
            case Cmd::KEY_TOGGLE:            return "KEY_TOGGLE";
                
            case Cmd::MOUSE_MOVE_ABS:        return "MOUSE_MOVE_ABS";
            case Cmd::MOUSE_MOVE_REL:        return "MOUSE_MOVE_REL";
            case Cmd::MOUSE_BUTTON:          return "MOUSE_BUTTON";
                
            case Cmd::JOY_EVENT:             return "JOY_EVENT";
                
            case Cmd::DSK_TOGGLE_WP:         return "DSK_TOGGLE_WP";
            case Cmd::DSK_MODIFIED:          return "DSK_MODIFIED";
            case Cmd::DSK_UNMODIFIED:        return "DSK_UNMODIFIED";
                
            case Cmd::RSH_EXECUTE:           return "RSH_EXECUTE";
                
            case Cmd::FUNC:                  return "FUNC";
            case Cmd::FOCUS:                 return "FOCUS";
                
        }
        return "???";
    }
    
    static const char *help(Cmd value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct
{
    Opt option;
    i64 value;
    isize id;
}
ConfigCommand;

typedef struct
{
    KeyCode keycode;
    double delay;
}
KeyCommand;

typedef struct
{
    isize port;
    double x;
    double y;
}
CoordCommand;

typedef struct
{
    isize port;
    GamePadAction action;
}
GamePadCommand;

typedef struct
{
    i64 cycle;
    i64 value;
}
AlarmCommand;

/*
 typedef struct
 {
 const char *command;
 }
 ShellCmd;
 */

struct Command
{
    // Header
    Cmd type;
    
    // Sender
    void *sender;
    
    // Payload
    union {
        
        struct { i64 value; i64 value2; };
        ConfigCommand config;
        KeyCommand key;
        GamePadCommand action;
        CoordCommand coord;
        AlarmCommand alarm;
    };
        
    Command() { }
    Command(Cmd type, i64 v1 = 0, i64 v2 = 0) : type(type), value(v1), value2(v2) { }
    Command(Cmd type, void *s, i64 v1 = 0, i64 v2 = 0) : type(type), sender(s), value(v1), value2(v2) { }
    Command(Cmd type, const ConfigCommand &cmd) : type(type), config(cmd) { }
    Command(Cmd type, const KeyCommand &cmd) : type(type), key(cmd) { }
    Command(Cmd type, const GamePadCommand &cmd) : type(type), action(cmd) { }
    Command(Cmd type, const CoordCommand &cmd) : type(type), coord(cmd) { }
    Command(Cmd type, const AlarmCommand &cmd) : type(type), alarm(cmd) { }
};

}
