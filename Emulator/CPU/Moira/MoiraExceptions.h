// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Saves information to stack for group 0 exceptions
void saveToStack(AEStackFrame &frame);

// Saves information to stack for group 1 and group 2 exceptions
void saveToStackBrief(u16 nr, u16 sr) { saveToStackBrief(nr, sr, reg.pc); }
void saveToStackBrief(u16 nr, u16 sr, u32 pc);
void saveToStackBrief68000(u16 nr, u16 sr, u32 pc);
void saveToStackBrief68010(u16 nr, u16 sr, u32 pc);

// Emulates an address error
void execAddressError(AEStackFrame frame, int delay = 0);

// Emulates a format error (68010+)
void execFormatError();

// Emulates the execution of unimplemented and illegal instructions
void execUnimplemented(int nr);

// Emulates a trace exception
void execTraceException();

// Emulates a trap exception
void execTrapException(int nr);

// Emulates a priviledge exception
void execPrivilegeException();

// Emulates an interrupt exception
void execIrqException(u8 level);
template <CPUModel C> void execIrqException(u8 level);
