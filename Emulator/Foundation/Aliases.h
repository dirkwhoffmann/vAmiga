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

#include <stdint.h>

//
// Basic types
//

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

//
// Cycle counts
//

typedef i64 Cycle;            // Master cycle units
typedef i64 CPUCycle;         // CPU cycle units
typedef i64 CIACycle;         // CIA cycle units
typedef i64 DMACycle;         // DMA cycle units

// Converts from a certain cycle unit to master cycles
#define CPU_CYCLES(cycles)    ((cycles) << 2)
#define CIA_CYCLES(cycles)    ((cycles) * 40)
#define DMA_CYCLES(cycles)    ((cycles) << 3)

#define USEC(delay)           (delay * 28)
#define MSEC(delay)           (delay * 28000)
#define SEC(delay)            (delay * 28000000)

// Converts from master cycles to a certain cycle unit
#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)

#define IS_CPU_CYCLE(cycles)  ((cycles) & 3 == 0)
#define IS_CIA_CYCLE(cycles)  ((cycles) % 40 == 0)
#define IS_DMA_CYCLE(cycles)  ((cycles) & 7 == 0)

#define AS_USEC(delay)        (delay / 28)
#define AS_MSEC(delay)        (delay / 28000)
#define AS_SEC(delay)         (delay / 28000000)

//
// Positions
//

typedef i16 PixelPos;

//
// Floppy disk
//

typedef i16 Side;
typedef i16 Cylinder;
typedef i16 Track;
typedef i16 Sector;

#endif
