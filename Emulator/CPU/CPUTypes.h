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

// A disassembled instruction
typedef struct
{
    u8 bytes;       // Length of the disassembled command in bytes
    char addr[9];   // Textual representation of the instruction's address
    char data[33];  // Textual representation of the instruction's data bytes
    char sr[17];    // Textual representation of the status register (optional)
    char instr[65]; // Textual representation of the instruction
}
DisassembledInstr;

#define CPUINFO_INSTR_COUNT 256

typedef struct
{
    // Registers
    // u32 pc;
    u32 pc0;
    u32 d[8];
    u32 a[8];
    u32 usp;
    u32 ssp;
    u16 sr;

    // Disassembled instructions starting at pc
    DisassembledInstr instr[CPUINFO_INSTR_COUNT];

    // Disassembled instructions from the log buffer
    DisassembledInstr loggedInstr[CPUINFO_INSTR_COUNT];
}
CPUInfo;

#endif
