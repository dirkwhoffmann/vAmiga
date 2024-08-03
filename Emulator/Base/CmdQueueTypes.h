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
#include "OptionTypes.h"
#include "JoystickTypes.h"
#include "KeyboardTypes.h"

//
// Enumerations
//

/// Emulator command
enum_long(CMD_TYPE)
{
    CMD_NONE,                   ///< None

    // Emulator
    CMD_CONFIG,                 ///< Configure the emulator
    CMD_CONFIG_ALL,             ///< Configure the emulator

    // Amiga
    CMD_ALARM_ABS,              ///< Schedule an alarm (absolute cycle)
    CMD_ALARM_REL,              ///< Schedule an alarm (relative cycle)
    CMD_INSPECTION_TARGET,      ///< Sets the auto-inspection mask

    // CPU (Breakpoints, Watchpoints, Catchpoints)
    CMD_GUARD_SET_AT,
    CMD_GUARD_REMOVE_NR,        ///< Remove the n-th guard
    CMD_GUARD_MOVE_NR,          ///< Change the address of a guard
    CMD_GUARD_IGNORE_NR,        ///< Change the ignore count of a guard
    CMD_GUARD_REMOVE_AT,        ///< Remove the guard at an address
    CMD_GUARD_REMOVE_ALL,       ///< Remove all guards
    CMD_GUARD_ENABLE_NR,        ///< Enable the n-th guard
    CMD_GUARD_ENABLE_AT,        ///< Enable the guard at an address
    CMD_GUARD_ENABLE_ALL,       ///< Enable all guards
    CMD_GUARD_DISABLE_NR,       ///< Disable the n-th guard
    CMD_GUARD_DISABLE_AT,       ///< Disable the guard at an address
    CMD_GUARD_DISABLE_ALL,      ///< Disable all guards

    // Keyboard
    CMD_KEY_PRESS,              ///< Press a key on the C64 keyboard
    CMD_KEY_RELEASE,            ///< Release a key on the C64 keyboard
    CMD_KEY_RELEASE_ALL,        ///< Clear the keyboard matrix
    CMD_KEY_TOGGLE,             ///< Press or release a key on the C64 keyboard

    // Mouse
    CMD_MOUSE_MOVE_ABS,         ///< Signal a mouse movement (absolute)
    CMD_MOUSE_MOVE_REL,         ///< Signal a mouse movement (relative)
    CMD_MOUSE_EVENT,            ///< Signal a mouse button event

    // Joystick
    CMD_JOY_EVENT,              ///< Signal a joystick button event

    // Floppy disk
    CMD_DSK_TOGGLE_WP,          ///< Toggle write-protection
    CMD_DSK_MODIFIED,           ///< Signal that the disk has been saved
    CMD_DSK_UNMODIFIED,         ///< Signan that the disk needs saving

    // RetroShell
    CMD_RSH_EXECUTE,            ///< Execute a script command

    // Experimental
    CMD_FUNC,

    // Host machine
    CMD_FOCUS                   ///< The emulator windows got or lost focus
};
typedef CMD_TYPE CmdType;

#ifdef __cplusplus
struct CmdTypeEnum : vamiga::util::Reflection<CmdTypeEnum, CmdType> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = CMD_FOCUS;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }

    static const char *prefix() { return "CMD"; }
    static const char *_key(long value)
    {
        switch (value) {

            case CMD_NONE:                  return "NONE";

            case CMD_CONFIG:                return "CONFIG";
            case CMD_CONFIG_ALL:            return "CONFIG_ALL";

            case CMD_ALARM_ABS:             return "ALARM_ABS";
            case CMD_ALARM_REL:             return "ALARM_REL";
            case CMD_INSPECTION_TARGET:     return "INSPECTION_TARGET";

            case CMD_GUARD_SET_AT:          return "GUARD_SET_AT";
            case CMD_GUARD_MOVE_NR:         return "GUARD_MOVE_TO";
            case CMD_GUARD_IGNORE_NR:       return "GUARD_IGNORE_NR";
            case CMD_GUARD_REMOVE_NR:       return "GUARD_REMOVE_NR";
            case CMD_GUARD_REMOVE_AT:       return "GUARD_REMOVE_AT";
            case CMD_GUARD_REMOVE_ALL:      return "GUARD_REMOVE_ALL";
            case CMD_GUARD_ENABLE_NR:       return "GUARD_ENABLE_NR";
            case CMD_GUARD_ENABLE_AT:       return "GUARD_ENABLE_AT";
            case CMD_GUARD_ENABLE_ALL:      return "GUARD_ENABLE_ALL";
            case CMD_GUARD_DISABLE_NR:      return "GUARD_DISABLE_NR";
            case CMD_GUARD_DISABLE_AT:      return "GUARD_DISABLE_AT";
            case CMD_GUARD_DISABLE_ALL:     return "GUARD_DISABLE_ALL";

            case CMD_KEY_PRESS:             return "KEY_PRESS";
            case CMD_KEY_RELEASE:           return "KEY_RELEASE";
            case CMD_KEY_RELEASE_ALL:       return "KEY_RELEASE_ALL";
            case CMD_KEY_TOGGLE:            return "KEY_TOGGLE";

            case CMD_MOUSE_MOVE_ABS:        return "MOUSE_MOVE_ABS";
            case CMD_MOUSE_MOVE_REL:        return "MOUSE_MOVE_REL";
            case CMD_MOUSE_EVENT:           return "MOUSE_EVENT";

            case CMD_JOY_EVENT:             return "JOY_EVENT";

            case CMD_DSK_TOGGLE_WP:         return "DSK_TOGGLE_WP";
            case CMD_DSK_MODIFIED:          return "DSK_MODIFIED";
            case CMD_DSK_UNMODIFIED:        return "DSK_UNMODIFIED";

            case CMD_RSH_EXECUTE:           return "RSH_EXECUTE";

            case CMD_FUNC:                  return "FUNC";
            case CMD_FOCUS:                 return "FOCUS";

        }
        return "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    Option option;
    i64 value;
    isize id;
}
ConfigCmd;

typedef struct
{
    KeyCode keycode;
    double delay;
}
KeyCmd;

typedef struct
{
    isize port;
    double x;
    double y;
}
CoordCmd;

typedef struct
{
    isize port;
    GamePadAction action;
}
GamePadCmd;

typedef struct
{
    i64 cycle;
    i64 value;
}
AlarmCmd;

/*
typedef struct
{
    const char *command;
}
ShellCmd;
*/

struct Cmd
{
    // Header
    CmdType type;

    // Sender
    void *sender;

    // Payload
    union {

        struct { i64 value; i64 value2; };
        ConfigCmd config;
        KeyCmd key;
        GamePadCmd action;
        CoordCmd coord;
        AlarmCmd alarm;
        /*
        ShellCmd shell;
        */
    };

#ifdef __cplusplus

    Cmd() { }
    Cmd(CmdType type, i64 v1 = 0, i64 v2 = 0) : type(type), value(v1), value2(v2) { }
    Cmd(CmdType type, void *s, i64 v1 = 0, i64 v2 = 0) : type(type), sender(s), value(v1), value2(v2) { }
    Cmd(CmdType type, const ConfigCmd &cmd) : type(type), config(cmd) { }
    Cmd(CmdType type, const KeyCmd &cmd) : type(type), key(cmd) { }
    Cmd(CmdType type, const GamePadCmd &cmd) : type(type), action(cmd) { }
    Cmd(CmdType type, const CoordCmd &cmd) : type(type), coord(cmd) { }
    Cmd(CmdType type, const AlarmCmd &cmd) : type(type), alarm(cmd) { }
    /*
    Cmd(CmdType type, const ShellCmd &cmd) : type(type), shell(cmd) { }
    */
#endif
};

