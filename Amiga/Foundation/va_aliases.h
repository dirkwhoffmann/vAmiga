// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VA_ALIASES_INC
#define _VA_ALIASES_INC

//
// Cycle number type aliases
//

typedef int64_t Frame;

typedef int64_t Cycle;    // Cycle in master cycle units
typedef int64_t CPUCycle; // Cycle in CPU cycle units
typedef int64_t CIACycle; // Cycle in CIA cycle units
typedef int64_t DMACycle; // Cycle in DMA cycle units

#define CPU_CYCLES(cycles) ((cycles) << 2)
#define CIA_CYCLES(cycles) ((cycles) * 40)
#define DMA_CYCLES(cycles) ((cycles) << 3)

#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)

#define USEC(delay) (delay * 28)
#define MSEC(delay) (delay * 28000)
#define SEC(delay)  (delay * 28000000)

#define AS_USEC(delay) (delay / 28)
#define AS_MSEC(delay) (delay / 28000)
#define AS_SEC(delay)  (delay / 28000000)


//
// Position number type aliases
//

typedef int16_t PixelPos;


//
// Floppy disk type aliases
//

typedef int16_t Side;
typedef int16_t Cylinder;
typedef int16_t Track;
typedef int16_t Sector;


#endif 
