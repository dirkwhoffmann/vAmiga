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

//
// Enumerations
//

typedef enum : long
{
    KB_SELFTEST,
    KB_SYNC,
    KB_STRM_ON,
    KB_STRM_OFF,
    KB_SEND
}
KeyboardState;

inline bool isKeyboardState(long value) {
    return value >= 0 && value <= KB_SEND;
}

//
// Structures
//

typedef struct
{
    bool accurate;
}
KeyboardConfig;

#endif
