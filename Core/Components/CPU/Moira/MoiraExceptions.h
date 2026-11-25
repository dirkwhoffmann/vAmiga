// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* Creates a stack frame of a certain format
 *
 * Format  Description                  68000  68010  68020  68030
 * ----------------------------------------------------------------------------
 *  AEBE   Address error / Bus error      X
 *  0000   Short format                   X      X      X      X
 *  0001   Throwaway                                    X      X
 *  0010   Instruction exception                        X      X
 *  0011   Floating-point post-instruction              (68040 only)
 *  1000   Bus fault                             X
 *  1001   Coprocessor mid-instruction                  X      X
 *  1010   Short bus fault                              X      X
 *  1011   Long bus fault                               X      X
 */
template <Core C> void writeStackFrameAEBE(StackFrame &frame);
template <Core C> void writeStackFrame0000(u16 sr, u32 pc, u16 nr);
template <Core C> void writeStackFrame0001(u16 sr, u32 pc, u16 nr);
template <Core C> void writeStackFrame0010(u16 sr, u32 pc, u32 ia, u16 nr);
template <Core C> void writeStackFrame0011(u16 sr, u32 pc, u32 ea, u16 nr);
template <Core C> void writeStackFrame0111(u16 sr, u32 pc, u16 nr, u16 ssw, u32 addr, u32 dataOut);
template <Core C> void writeStackFrame1000(StackFrame &frame, u16 sr, u32 pc, u32 ia, u16 nr, u32 addr);
template <Core C> void writeStackFrame1001(u16 sr, u32 pc, u32 ia, u16 nr);
template <Core C> void writeStackFrame1010(u16 sr, u32 pc, u16 nr, u16 ssw = 0, u32 addr = 0, u32 dataOut = 0, u16 stageC = 0, u16 stageB = 0);
template <Core C> void writeStackFrame1011UAE(u16 sr, u16 nr, u32 pc, u16 ssw, u32 addr, u32 end, bool write);
template <Core C> void writeStackFrame1011(u16 sr, u32 pc, u32 ia, u16 nr, u16 ssw = 0, u32 addr = 0, u32 dataOut = 0, u16 stageC = 0, u16 stageB = 0);

// Emulates an exception other than address errors, bus errors, and interrupts
// (nr is the trap number for TRAP and the effective address for FPU_UNIMPLEMENTED)
void execException(M68kException exc, int nr = 0);
template <Core C> void execException(M68kException exc, int nr = 0);

// Raises the 68040's unimplemented floating-point instruction exception
template <Core C, Mode M> void execFUnimplemented(int n, FpuFormat fmt, bool rm);

// Raises a floating-point arithmetic exception (vectors 48 - 54). A pre-instruction
// exception is taken before an FPU instruction starts and stacks its address
// (format 0). A post-instruction exception (68040, FMOVE to memory) is taken
// after the instruction and stacks the address of the next instruction and the
// effective address (format 3).
template <Core C> void execFpuTrap(u16 vector, bool post = false, u32 ea = 0);

// Takes a pending floating-point arithmetic exception (returns false if none is pending)
template <Core C> bool takePendingFpuTrap();

// Completes an FPU instruction that writes a data register: restores the old value
// if an enabled exception suppresses the result and records the pending exception
void finishFpuOp(int dst, const MFloat80 &old);

// Emulates an address or bus error
template <Core C> void execAddressError(StackFrame frame, int delay = 0);
template <Core C> void execBusError(StackFrame frame, int delay = 0, const Registers *saved = nullptr);

// Emulates an interrupt
void execInterrupt(u8 level);
template <Core C> void execInterrupt(u8 level);

// Processes an exception that was caught during execution. If the exception
// was raised inside an instruction whose start state has been saved, saved
// points to that state (nullptr otherwise).
void processException(const std::exception &exception, const Registers *saved = nullptr);

// Processes an exception for a specific CPU core type
template <Core C> void processException(const std::exception &exception, const Registers *saved = nullptr);

// Rolls the registers back to the state saved at the start of the instruction
void restoreRegisters(const Registers &saved);

// Takes back the predecrement of the operand that caused a bus error
void undoPredecrement(const StackFrame &frame, const Registers &saved, u16 ird);

// Rethrows a bus error that has been caught inside an instruction handler.
// Instructions that have already changed some condition codes when the access
// fails (in the reference CPUs) call this function to make those flags visible
// in the register file and in the stacked status register. Only the bits in
// 'mask' are taken from 'ccr'. A stack frame supplied by the host (see
// BusError::custom) is not modified. Must be called from within a catch block.
template <Core C> [[noreturn]] void rethrow(BusError &e, u8 mask, u8 ccr);

// Variant for handlers that have already set the flags in the status register.
// The current flags are shown in the stacked status register.
template <Core C> [[noreturn]] void rethrow(BusError &e);
template <Core C, Size S> [[noreturn]] void rethrowD(BusError &e, int n, u32 value);
template <Core C, Size S = Long> [[noreturn]] void rethrowA(BusError &e, int n, u32 value);

// Returns the condition codes that MAME shows while the prefetch of a register
// shift is pending. They are computed from the unshifted operand.
u8 shiftPendingCCR(Instr I, Size S, u32 operand) const;
