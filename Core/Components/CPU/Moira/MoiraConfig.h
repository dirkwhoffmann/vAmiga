// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

/* Set to true to enable precise timing mode (68000 and 68010 only).
 *
 * When disabled, Moira calls the 'sync' function at the end of each instruction,
 * passing the number of elapsed cycles as an argument. In precise timing mode,
 * 'sync' is called before each memory access, allowing the client to emulate
 * the surrounding hardware up to the point where the memory access occurs.
 *
 * Precise timing mode is available only for 68000 and 68010 emulation. For
 * other supported models, this setting has no effect.
 *
 * Enable to improve accuracy, disable it to enhance performance.
 */
#define MOIRA_PRECISE_TIMING true

/* Set to true to implement the CPU interface as virtual functions.
 *
 * To interact with the environment (e.g., for reading data from memory),
 * the CPU calls an appropriate function that must be implemented by the client.
 * When this option is enabled, all API functions are declared virtual, following
 * the standard object-oriented programming (OOP) paradigm. However, virtual
 * functions introduce a performance penalty. Setting this option to false allows
 * the client API to be statically linked, improving performance.
 *
 * Enable to adhere to the standard OOP paradigm, disable to gain speed.
 */
#define MOIRA_VIRTUAL_API false

/* Set to true to enable address error checking.
 *
 * The 68000 and 68010 trigger an address error violation if a word or longword
 * is accessed at an odd memory address.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#define MOIRA_EMULATE_ADDRESS_ERROR true

/* Set to true to emulate function code pins FC0 - FC2.
 *
 * Whenever memory is accessed, the function code pins enable external hardware
 * to inspect the access type. If used, these pins are usually connected to an
 * external memory management unit (MMU).
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#define MOIRA_EMULATE_FC true

/* Set to true to enable the disassembler.
 *
 * The disassembler requires a jump table which consumes about 1.5MB of memory.
 *
 * Disable to save space.
 */
#define MOIRA_ENABLE_DASM true

/* Set to true to build the InstrInfo lookup table.
 *
 * The instruction info table stores information about the instruction
 * (Instr I), the addressing mode (Mode M), and the size attribute (Size S) for
 * all 65536 opcode words. The table is meant to provide data for, e.g.,
 * external debuggers.
 *
 * Note that the disassembler reads the very same attributes from this table,
 * so the table is built unconditionally when MOIRA_ENABLE_DASM is set. This
 * option only matters when the disassembler is switched off.
 *
 * Disable to save space.
 */
#define MOIRA_WANT_INSTR_INFO_TABLE false

/* Indicates whether the InstrInfo lookup table is built.
 *
 * Derived from the two options above; the disassembler cannot work without
 * the table. Use this macro, not MOIRA_WANT_INSTR_INFO_TABLE, to check whether
 * the table is available.
 */
#define MOIRA_BUILD_INSTR_INFO_TABLE (MOIRA_WANT_INSTR_INFO_TABLE || MOIRA_ENABLE_DASM)

/* Enables Musashi compatibility mode.
 *
 * When set to true, Moira runs in a special mode designed for compatibility
 * with the Musashi emulator. This mode is primarily used by the test runner
 * application to compare Moira's behavior against Musashi.
 *
 * Set to false for improved accuracy.
 */
#define MOIRA_MIMIC_MUSASHI false

/* The following macro appears at the beginning of each instruction handler.
 * Moira will call 'willExecute(...)' for all listed instructions.
 */
#define MOIRA_WILL_EXECUTE    I == Instr::STOP || I == Instr::TAS || I == Instr::BKPT

/* The following macro appear at the end of each instruction handler.
 * Moira will call 'didExecute(...)' for all listed instructions.
 */
#define MOIRA_DID_EXECUTE I == Instr::RESET
