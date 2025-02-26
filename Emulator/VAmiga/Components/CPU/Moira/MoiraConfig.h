// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

/* Set to true to enable precise timing mode (68000 and 68010 only).
 *
 * If disabled, Moira calls function 'sync' at the end of each instruction
 * with the number of elapsed cycles as argument. In precise timing mode,
 * 'sync' is called prior to each memory access. This enables the client to
 * emulate the surrounding hardware up the point where the memory access
 * actually happens.
 *
 * Precise timing mode is only available in 68000 or 68010 emulation. For
 * all other supported models, this setting has no effect.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#define PRECISE_TIMING true

/* Set to true to implement the CPU interface as virtual functions.
 *
 * To communicate with the environment (e.g., for reading data from memory),
 * the CPU calls an appropriate function that has to be implemented by the
 * client. If this option is set to true, all API functions are declared
 * virtual which corresponds to the standard OOP pradigm. Because virtual
 * functions impose a performance penalty, Moira allows to link the client API
 * statically by setting this option to false.
 *
 * Enable to follow the standard OOP paradigm, disable to gain speed.
 */
#define VIRTUAL_API false

/* Set to true to enable address error checking.
 *
 * The 68000 and 68010 signal an address error violation if a word or long word
 * is accessed at an odd memory location.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#define EMULATE_ADDRESS_ERROR true

/* Set to true to emulate function code pins FC0 - FC2.
 *
 * Whenever memory is accessed, the function code pins enable external hardware
 * to inspect the access type. If used, these pins are usually connected to an
 * external memory management unit (MMU).
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#define EMULATE_FC true

/* Set to true to enable the disassembler.
 *
 * The disassembler requires a jump table which consumes about 1MB of memory.
 *
 * Disable to save space.
 */
#define ENABLE_DASM true

/* Set to true to build the InstrInfo lookup table.
 *
 * The instruction info table stores information about the instruction
 * (Instr I), the addressing mode (Mode M), and the size attribute (Size S) for
 * all 65536 opcode words. The table is meant to provide data for, e.g.,
 * external debuggers. It is not needed by Moira itself and therefore disabled
 * by default.
 *
 * Disable to save space.
 */
#define BUILD_INSTR_INFO_TABLE false

/* Set to true to run Moira in a special Musashi compatibility mode.
 *
 * The compatibility mode is used by the test runner application to compare
 * Moira against Musashi.
 *
 * Disable to improve accuracy.
 */
#define MIMIC_MUSASHI false

/* The following macro appears at the beginning of each instruction handler.
 * Moira will call 'willExecute(...)' for all listed instructions.
 */
#define WILL_EXECUTE I == Instr::STOP || I == Instr::TAS || I == Instr::BKPT
/*
#define WILL_EXECUTE \
I == Instr::FABS || \
I == Instr::FADD || \
I == Instr::FBcc || \
I == Instr::FCMP || \
I == Instr::FDBcc || \
I == Instr::FDIV || \
I == Instr::FMOVE || \
I == Instr::FMOVEM || \
I == Instr::FMUL || \
I == Instr::FNEG || \
I == Instr::FNOP || \
I == Instr::FRESTORE || \
I == Instr::FSAVE || \
I == Instr::FScc || \
I == Instr::FSQRT || \
I == Instr::FSUB || \
I == Instr::FTRAPcc || \
I == Instr::FTST || \
I == Instr::FSABS || \
I == Instr::FDABS || \
I == Instr::FSADD || \
I == Instr::FDADD || \
I == Instr::FSDIV || \
I == Instr::FDDIV || \
I == Instr::FSMOVE || \
I == Instr::FDMOVE || \
I == Instr::FSMUL || \
I == Instr::FDMUL || \
I == Instr::FSNEG || \
I == Instr::FDNEG || \
I == Instr::FSSQRT || \
I == Instr::FDSQRT || \
I == Instr::FSSUB || \
I == Instr::FDSUB || \
I == Instr::FACOS || \
I == Instr::FASIN || \
I == Instr::FATAN || \
I == Instr::FATANH || \
I == Instr::FCOS || \
I == Instr::FCOSH || \
I == Instr::FETOX || \
I == Instr::FETOXM1 || \
I == Instr::FGETEXP || \
I == Instr::FGETMAN || \
I == Instr::FINT || \
I == Instr::FINTRZ || \
I == Instr::FLOG10 || \
I == Instr::FLOG2 || \
I == Instr::FLOGN || \
I == Instr::FLOGNP1 || \
I == Instr::FMOD || \
I == Instr::FMOVECR || \
I == Instr::FREM || \
I == Instr::FSCAL || \
I == Instr::FSGLDIV || \
I == Instr::FSGLMUL || \
I == Instr::FSIN || \
I == Instr::FSINCOS || \
I == Instr::FSINH || \
I == Instr::FTAN || \
I == Instr::FTANH || \
I == Instr::FTENTOX || \
I == Instr::FTWOTOX
*/

/* The following macro appear at the end of each instruction handler.
 * Moira will call 'didExecute(...)' for all listed instructions.
 */
#define DID_EXECUTE     I == Instr::RESET
