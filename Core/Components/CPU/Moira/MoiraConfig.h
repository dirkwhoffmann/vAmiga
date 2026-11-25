// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

// Provides the name lookup that the headers of the surrounding core rely on
#include "vaconfig.h"

/* Name of the namespace that contains all of Moira.
 *
 * Compile with -DMOIRA_NAMESPACE=<name> to build several differently configured
 * Moira libraries that can be linked into the same program.
 */
#ifndef MOIRA_NAMESPACE
#define MOIRA_NAMESPACE vamiga::moira
#endif

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
#ifndef MOIRA_PRECISE_TIMING
#define MOIRA_PRECISE_TIMING true
#endif

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
#ifndef MOIRA_VIRTUAL_API
#define MOIRA_VIRTUAL_API false
#endif

/* Set to true to enable address error checking.
 *
 * The 68000 and 68010 trigger an address error violation if a word or longword
 * is accessed at an odd memory address. On all models, an odd program counter
 * triggers an address error.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#ifndef MOIRA_EMULATE_ADDRESS_ERROR
#define MOIRA_EMULATE_ADDRESS_ERROR true
#endif

/* Set to true to enable bus error emulation.
 *
 * A client signals a bus error by throwing a BusError exception from one of
 * the memory access functions. The 68000 and 68010 always process such an
 * exception at no extra cost. The 68020 and later models can restart the
 * faulted instruction after the exception handler returns with RTE. To
 * support this, Moira saves the registers before each instruction, which
 * forces these models onto the slow execution path. If this option is
 * disabled, the 68020 family does not save the registers. A BusError that
 * is thrown anyway is still processed, but the instruction cannot be
 * restarted. This option has no effect while an MMU is enabled, because an
 * address translation fault is a bus error that must be restartable.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#ifndef MOIRA_EMULATE_BUS_ERROR
#define MOIRA_EMULATE_BUS_ERROR false
#endif

/* Set to true to report each data bus access to the delegate.
 *
 * If enabled, willAccessBus() is called once per operand before the bus
 * cycles of a data access are performed. A long word access that Moira splits
 * into several bus cycles is reported only once. The option is intended for
 * for hosts that need to trace operand accesses such as test environments.
 *
 * Disable to gain speed.
 */
#ifndef MOIRA_REPORT_BUS_ACCESS
#define MOIRA_REPORT_BUS_ACCESS false
#endif

/* Set to true to emulate function code pins FC0 - FC2.
 *
 * Whenever memory is accessed, the function code pins enable external hardware
 * to inspect the access type. If used, these pins are usually connected to an
 * external memory management unit (MMU).
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#ifndef MOIRA_EMULATE_FC
#define MOIRA_EMULATE_FC true
#endif

/* Set to true to emulate the instruction cache of the 68020 and 68030.
 *
 * The 68020 and 68030 cache instruction fetches in a small on-chip cache (see
 * Cache/Cache.h). When this option is disabled, every instruction fetch is
 * treated as a cache miss and goes straight to the bus, which is faster to
 * emulate but does not reproduce the cycle savings a real cache provides.
 * The 68000 and 68010 have no instruction cache. The caches of the 68040
 * models are not emulated, so the setting has no effect for them.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#ifndef MOIRA_EMULATE_ICACHE
#define MOIRA_EMULATE_ICACHE true
#endif

/* Set to true to emulate the 68030 data cache.
 *
 * The 68030 caches data accesses in a small on-chip cache (see Cache/Cache.h). When
 * this option is disabled, every data access goes straight to the bus. The
 * other models have no data cache, except for the 68040 models, whose caches
 * are not emulated. The setting has no effect for them.
 *
 * Enable to improve accuracy, disable to gain speed.
 */
#ifndef MOIRA_EMULATE_DCACHE
#define MOIRA_EMULATE_DCACHE true
#endif

/* Set to true to enable the disassembler.
 *
 * The disassembler requires a jump table which consumes about 1MB of memory,
 * plus 0.25MB for the instruction info table storing information about the
 * instruction (Instr I), the addressing mode (Mode M), and the size attribute
 * (Size S) for all 65536 opcode words.
 *
 * Disable to save space.
 */
#ifndef MOIRA_ENABLE_DASM
#define MOIRA_ENABLE_DASM true
#endif

/* Selects the compatibility mode.
 *
 * Moira can mimic some of the behavior of other 68k emulators where they differ
 * from each other (e.g., the order of bus accesses). The following values are
 * available:
 *
 *   DEFAULT: Moira's own behavior.
 *   MUSASHI: Adopts the instruction timing of the Musashi emulator. This mode
 *            is used by the test runner to compare Moira against Musashi.
 *            Musashi's functional flaws are not replicated (the test runner
 *            excludes the affected cases instead).
 *   UAE:     Compatibility with the CPU core of the UAE Amiga emulator (as used
 *            in WinUAE and Amiberry). This mode is used by the test runner when
 *            UAE is selected as the reference implementation.
 *   MAME:    Compatibility with MAME's microcoded 68000 core, which generated the
 *            SingleStepTests. This mode is used by the SingleStep runner.
 */
#ifndef MOIRA_COMPATIBILITY
#define MOIRA_COMPATIBILITY DEFAULT
#endif

/* The following macro appears at the beginning of each instruction handler.
 * Moira will call 'willExecute(...)' for all listed instructions.
 */
#ifndef MOIRA_WILL_EXECUTE
#define MOIRA_WILL_EXECUTE I == Instr::STOP || I == Instr::TAS || I == Instr::BKPT
#endif

/* The following macro appear at the end of each instruction handler.
 * Moira will call 'didExecute(...)' for all listed instructions.
 */
#ifndef MOIRA_DID_EXECUTE
#define MOIRA_DID_EXECUTE I == Instr::RESET
#endif
