// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef MOIRA_CONFIG_H
#define MOIRA_CONFIG_H

/* Set to true to enable address error checking
 * Recommended setting: true
 */
#define MOIRA_EMULATE_ADDRESS_ERROR true

/* Set to true to emulate the function code pins FC0 - FC2
 * Recommended setting: false
 */
#define EMULATE_FC true


/* Capacity of the debug logging buffer
 * Recommended setting: Any power of 2
 */
#define LOG_BUFFER_CAPACITY 256

/* Set to true to run Moira in a special Musashi compatibility mode.
 * Recommended setting: false
 */
#define MIMIC_MUSASHI false

#endif
