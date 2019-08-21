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

// CPU actions
#define CPU_SET_IRQ_LEVEL   0b0000'0001
#define CPU_ADD_WAIT_STATES 0b0000'0010


/* Recorded instruction
 * This data structure is used inside the trace ringbuffer. In trace mode,
 * the program counter and the status register are recorded. The instruction
 * string is computed on-the-fly due to speed reasons.
 * DEPRECATED
 */
typedef struct
{
    Cycle cycle;
    uint16_t vhcount;
    uint32_t pc;
    uint32_t sp;
    // char instr[63];
    // char flags[17];
}
RecordedInstruction;

// A disassembled instruction
typedef struct
{
    uint8_t bytes;  // Length of the disassembled command in bytes
    char addr[9];   // Textual representation of the instruction's address
    char data[33];  // Textual representation of the instruction's data bytes
    char flags[17]; // Textual representation of the status register (optional)
    char instr[65]; // Textual representation of the instruction
}
DisassembledInstruction;

#define CPUINFO_INSTR_COUNT 32

typedef struct
{
    // Registers
    uint32_t pc;
    uint32_t d[8];
    uint32_t a[8];
    uint32_t ssp;
    uint16_t flags;

    // Disassembled instructions starting at pc
    DisassembledInstruction instr[CPUINFO_INSTR_COUNT];

    // Disassembled instructions from the trace buffer
    DisassembledInstruction traceInstr[CPUINFO_INSTR_COUNT];
}
CPUInfo;

#endif
