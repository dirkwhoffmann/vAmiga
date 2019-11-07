// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _KEYBOARD_T_INC
#define _KEYBOARD_T_INC

typedef enum : long
{
    KB_A500,
    KB_A1000,
    KB_A2000
}
KeyboardModel;

inline bool isKeyboardModel(long value)
{
    return value >= KB_A500 && value <= KB_A2000;
}

inline const char *KeyboardModelName(KeyboardModel model)
{
    assert(isKeyboardModel(model));

    switch (model) {
        case KB_A500:  return "KB_A500";
        case KB_A1000: return "KB_A1000";
        case KB_A2000: return "KB_A2000";
        default:       return "???";
    }
}

typedef enum : long
{
    KB_GENERIC,
    KB_US,
    KB_GE,
    KB_IT
}
KeyboardLanguage;

inline bool isKeyboardLanguage(long value)
{
    return value >= KB_GENERIC && value <= KB_IT;
}

inline const char *KeyboardLanguageName(KeyboardLanguage model)
{
    assert(isKeyboardModel(model));

    switch (model) {
        case KB_GENERIC: return "KB_GENERIC";
        case KB_US:      return "KB_US";
        case KB_GE:      return "KB_GE";
        case KB_IT:      return "KB_IT";
        default:         return "???";
    }
}

typedef struct
{
    KeyboardModel model;
    KeyboardLanguage language;
}
KeyboardConfig;

#endif
