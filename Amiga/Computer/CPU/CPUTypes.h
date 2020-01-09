// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _CPU_T_INC
#define _CPU_T_INC

// CPU engine
typedef enum : long
{
    CPU_MUSASHI
}
CPUEngine;

inline bool isCPUEngine(long value)
{
    return value == CPU_MUSASHI;
}

/* Recorded instruction
 * This data structure is used inside the trace ringbuffer.
 */
typedef struct
{
    uint32_t pc;
    uint32_t sr;
}
RecInstr;

// A disassembled instruction
typedef struct
{
    uint8_t bytes;  // Length of the disassembled command in bytes
    char addr[9];   // Textual representation of the instruction's address
    char data[33];  // Textual representation of the instruction's data bytes
    char flags[17]; // Textual representation of the status register (optional)
    char instr[65]; // Textual representation of the instruction
}
DisInstr;

#define CPUINFO_INSTR_COUNT 255

typedef struct
{
    // Registers
    uint32_t pc;
    uint32_t d[8];
    uint32_t a[8];
    uint32_t usp;
    uint32_t ssp;
    uint16_t flags;

    // Disassembled instructions starting at pc
    char addr[CPUINFO_INSTR_COUNT][9];
    char data[CPUINFO_INSTR_COUNT][33];
    char dasm[CPUINFO_INSTR_COUNT][65];

    DisInstr instr[CPUINFO_INSTR_COUNT]; // DEPRECATED

    // Disassembled instructions from the trace buffer
    DisInstr traceInstr[CPUINFO_INSTR_COUNT];
}
CPUInfo;

#endif
