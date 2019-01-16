// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef VA_CONSTANTS_H
#define VA_CONSTANTS_H

// Clock rate of the master clock (PAL)
static const double masterClockFrequency = 28.37516;

// Clock rate of the Motorola 68000 CPU (7.09379 MHz)
static const double cpuClockFrequency = masterClockFrequency / 4.0;

// Frame rate (PAL)
// IS THIS 100% CORRECT? E.G., THE C64 DIFFERS SLIGHTLHY FROM 50
static const double frameRate = 50.0;

// Number of master clock cycles per frame
static const double masterCyclesPerFrame = (masterClockFrequency * 1000000.0) / frameRate;

#endif 
