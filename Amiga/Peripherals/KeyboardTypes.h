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
    KB,
    DRIVE_35_DD_PC,
    DRIVE_35_HD,
    DRIVE_35_HD_PC,
    DRIVE_525_SD
}
DriveType;
