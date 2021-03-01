// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

// Converts a certain unit to master cycles
#define USEC(delay)           ((delay) * 28)
#define MSEC(delay)           ((delay) * 28000)
#define SEC(delay)            ((delay) * 28000000)

#define CPU_CYCLES(cycles)    ((cycles) << 2)
#define CIA_CYCLES(cycles)    ((cycles) * 40)
#define DMA_CYCLES(cycles)    ((cycles) << 3)

// Converts master cycles to a certain unit
#define AS_USEC(delay)        ((delay) / 28)
#define AS_MSEC(delay)        ((delay) / 28000)
#define AS_SEC(delay)         ((delay) / 28000000)

#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)

#define IS_CPU_CYCLE(cycles)  ((cycles) & 3 == 0)
#define IS_CIA_CYCLE(cycles)  ((cycles) % 40 == 0)
#define IS_DMA_CYCLE(cycles)  ((cycles) & 7 == 0)

// Converts kilo and mega bytes to bytes
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)

// Converts kilo and mega Hertz to Hertz
#define KHz(x) ((x) * 1000)
#define MHz(x) ((x) * 1000000)

// Controls the '<<' stream operator
#define DEC std::dec
#define HEX8 std::hex << "0x" << std::setw(2) << std::setfill('0')
#define HEX16 std::hex << "0x" << std::setw(4) << std::setfill('0')
#define HEX32 std::hex << "0x" << std::setw(8) << std::setfill('0')
#define HEX64 std::hex << "0x" << std::setw(16) << std::setfill('0')
#define TAB(x) std::left << std::setw(x)
#define YESNO(x) ((x) ? "yes" : "no")
#define ONOFF(x) ((x) ? "on" : "off")
#define HILO(x) ((x) ? "high" : "low")
#define ISENABLED(x) ((x) ? "enabled" : "disabled")
#define ISSET(x) ((x) ? "set" : "not set")
#define EMULATED(x) ((x) ? "emulated" : "not emulated")
#define DUMP(x) std::setw(24) << std::right << std::setfill(' ') << (x) << " : "
