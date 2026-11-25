// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* Moira utilizes three layers to model dataflow inside the processor.
 *
 * Layer 1: Main entry points. Most instruction execution handlers call a
 *          layer 1 function to read or write their operands.
 *
 * Layer 2: Data providers. This layer provides functions for moving data back
 *          and forth between various locations.
 *
 * Layer 3: Memory interface. The functions from this layer perform the actual
 *          memory access.
 *
 * The following picture depicts the interplay between the different layers:
 *
 *                                      |
 * - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - - - - - - - - -
 * Layer 1:                             V
 *                                    readOp
 *                                   (writeOp)
 *                                      |
 *  Addressing Mode M = 0---1---2---3---4---5---6---7---8---9---A---B
 *                     /    |   |   |   |   |   |   |   |   |   |    \
 *                    /     |   ---------------------------------     \
 *                   /      |           |                              \
 *                  /       |           V                               \
 *                 /        |       computeEA -----------                \
 *                /         |           |               |                 \
 * - - - - - - - -|- - - - -|- - - - - -|- - - - - - - -|- - - - - - - - -|- - -
 * Layer 2:       |         |           |               |                 |
 *                V         V           V               V                 V
 *              readD     readA      readM           readExt            readI
 *             (writeD)  (writeA)   (writeM)
 *                                      |
 * - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - - - - - - - - -
 * Layer 3:                             |
 *                 Size S = B-----------W-----------L
 *                          |           |           |
 *                          V           V           V
 *                        read8       read16    2 x read16
 *                       (write8)    (write8)  (2 x write16)
 *
 */

/* Computes an effective address
 */
template <Core C, Mode M, Size S, Flags F = 0> u32 computeEA(u32 n);
template <Core C, Mode M, Size S, Flags F = 0> u32 computeEAbrief(u32 an);
template <Core C, Mode M, Size S, Flags F = 0> u32 computeEAfull(u32 an);

/* Reads an operand
 *
 * The source of the operand is determined by the addressing mode M. If the
 * operand is read from memory, the effective address is stored in variable ea.
 * If the source is a register or an immediate value, variable ea remains
 * untouched.
 */
template <Core C, Mode M, Size S = Word, Flags F = 0>
void readOp(int n, u32 *ea, u32 *result);

// Reads a 64-bit operand (used by the MMU)
template <Core C, Mode M, Flags F = 0> void readOp64(int n, u32 *ea, u64 *result);


/* Writes an operand
 *
 * If parameter ea is omitted, the destination of the operand is determined
 * by the addressing mode M.
 */
template <Core C, Mode M, Size S, Flags F = 0> void writeOp(int n, u32 val);
template <Core C, Mode M, Size S, Flags F = 0> void writeOp(int n, u32 ea, u32 val);

// Writes a 64-bit operand (used by the MMU)
template <Core C, Mode M, Flags F = 0> void writeOp64(int n, u64 val);

// Writes a status register value to memory (MOVE from SR/CCR on the 68010 and above).
// F is the frame type of the address error raised for a misaligned address.
template <Core C, Mode M, Size S, Flags F> void writeStatusEa(int n, u16 val);

// Emulates the address register modification for modes (An)+, (An)- (DEPRECATED)
template <Mode M, Size S> void updateAn(int n);
template <Mode M, Size S> void updateAnPI(int n);
template <Mode M, Size S> void updateAnPD(int n);
template <Mode M, Size S> void undoAn(int n);
template <Mode M, Size S> void undoAnPI(int n);
template <Mode M, Size S> void undoAnPD(int n);

// Emulates the address register modification for modes (An)+, (An)-
void updateAn(Mode M, Size S, int n);
void updateAnPI(Mode M, Size S, int n);
void updateAnPD(Mode M, Size S, int n);
void undoAn(Mode M, Size S, int n);
void undoAnPI(Mode M, Size S, int n);
void undoAnPD(Mode M, Size S, int n);

// Reads a value from program or data space, depending on the addressing mode
template <Core C, Mode M, Size S, Flags F = 0> u32 readM(u32 addr);

// Reads a value from a specific address space
template <Core C, AddrSpace AS, Size S, Flags F = 0> u32 read(u32 addr);

// Reads from memory without consulting the data cache
template <Core C, Size S, Flags F = 0> u32 readBus(u32 addr);

// Reads data via the data cache (68030)
template <Core C, Size S, Flags F = 0> u32 readDataCache(u32 addr);

// Reads an instruction word, either from memory or the instruction cache
template <Core C, Flags F = 0> u16 readInstr(u32 addr);


// Reads the instruction word discarded by MOVE to SR or CCR.
template <Core C> void readStatusRefill();

// Writes an operand to memory (without or with address error checking)
template <Core C, Mode M, Size S, Flags F = 0> void writeM(u32 addr, u32 val);

// Writes a value to a specific address space
template <Core C, AddrSpace AS, Size S, Flags F = 0> void write(u32 addr, u32 val);

// Writes to memory without updating the data cache
template <Core C, Size S, Flags F = 0> void writeBus(u32 addr, u32 val, bool *ci = nullptr);

// Writes data via the data cache (68030)
template <Core C, Size S, Flags F = 0> void writeDataCache(u32 addr, u32 val);

/* Bus transfer helpers used by read() and write() to split a wide access
 * into the bus cycles a narrower port requires. Factored out because the
 * same sub-patterns recur across the Word/Long, aligned/misaligned and
 * 68020/pre-68020 cases.
 */

// Reads/writes two consecutive words (used for Long accesses split in half)
template <Core C, Flags F = 0> u32 read2x16(u32 lo, u32 hi);
template <Core C, Flags F = 0> void write2x16(u32 lo, u32 hi, u32 valHi, u32 valLo);

// Reads/writes a word through an 8 bit port (two byte-sized bus cycles)
template <Core C, Flags F = 0> u32 read2x8(u32 addr);
template <Core C, Flags F = 0> void write2x8(u32 addr, u32 val);

// Reads/writes a longword through an 8 bit port (four byte-sized bus cycles)
template <Core C, Flags F = 0> u32 read4x8(u32 addr);
template <Core C, Flags F = 0> void write4x8(u32 addr, u32 val);

// Reads/writes an aligned longword, dispatching on the addressed port width
template <Core C, Flags F = 0> u32 readLongSplit(u32 addr);
template <Core C, Flags F = 0> void writeLongSplit(u32 addr, u32 val);

// Reads an immediate value from memory
template <Core C, Size S> u32 readI();

// Same as readI, but stacks the PC as MAME does if the refill fails
template <Core C, Size S> u32 readIFrame();

// Pushes a value onto the stack
template <Core C, Size S, Flags F = 0> void push(u32 value);

// Pops a value from the stack
template <Core C, Size S, Flags F = 0> u32 pop();

// Checks whether the provided address should trigger an address error
template <Core C, Size S = Word> bool misaligned(u32 addr);
template <Core C> bool misalignedPC(u32 addr);

// Creates an address error stack frame
template <Flags F = 0> StackFrame makeFrame(u32 addr, u32 pc, u16 sr, u16 ird);
template <Flags F = 0> StackFrame makeFrame(u32 addr, u32 pc);
template <Flags F = 0> StackFrame makeFrame(u32 addr);

// Adds the special status word to a bus error signaled by the host (68020 and later)
template <Size S, bool W> void completeBusError(BusError &e, bool instructionFetch = false);

// Creates an address error that is raised before the bus cycle has started
template <Core C> AddressError earlyAddressError(const StackFrame &frame);

// Raises an early address error with a frame of type F if the provided address is misaligned
template <Core C, Size S = Word, Flags F = 0> void checkEven(u32 addr);
template <Core C, Flags F = 0> void checkEvenPC(u32 addr);

// Raises an address error if the stack pointer is misaligned before a value is popped
template <Core C, Mode M> void checkEvenSP();

// Prefetches the next instruction
template <Core C, Flags F = 0> void prefetch();

// Early prefetch (68000): Reads the next word, but leaves the prefetch queue untouched
// until commitIrd is called. The real CPU replaces the IRD when the last bus cycle of the
// instruction starts. If ON is false, the call is an ordinary prefetch.
template <Core C, Flags F = 0, bool ON = true> void fetchIrc();
template <Core C> void commitIrd();
template <Core C, Flags F = 0, bool ON = true> void prefetchLate();

// Performs a full prefetch cycle
template <Core C, Flags F = 0, int delay = 0> void fullPrefetch();

// Jumps to a new address. MAME stacks the old PC if the first fetch fails.
// Jumps to newpc and fetches the first word. A failing fetch stacks framePc (68000)
template <Core C> void fetchFirstAt(u32 newpc, u32 framePc);
template <Core C, Flags F = 0> void fullPrefetchAt(u32 newpc);
template <Core C, Flags F = 0> void fullPrefetchAt(u32 newpc, u32 framePc);   // A failing first fetch stacks framePc (68000)

// prefetch replacement if loop mode is active
template <Core C> void noPrefetch(int delay = 0);

// Reads an extension word from memory
template <Core C, Flags F = 0> void readExt();
template <Core C> void readExtLast();

/* Reads and writes floating-point operands. Depending on the addressing mode,
 * the operand is located in a data register, in memory, or in the instruction
 * stream. The operand format is given as an FpuFormat. Operands in memory
 * advance the address register for the modes (An)+ and -(An).
 */
template <Core C, Mode M> MFloat80 readFpuOp(int n, FpuFormat fmt);
template <Core C> MFloat80 readFpuOpRg(int n, FpuFormat fmt);
template <Core C> MFloat80 readFpuOpIm(FpuFormat fmt);
template <Core C, Mode M> MFloat80 readFpuOpEa(int n, u32 ea, FpuFormat fmt);

template <Core C, Mode M> void writeFpuOp(int n, const MFloat80 &val, FpuFormat fmt, int k);
template <Core C> void writeFpuOpRg(int n, const MFloat80 &val, FpuFormat fmt);
template <Core C, Mode M> void writeFpuOpEa(int n, u32 ea, const MFloat80 &val, FpuFormat fmt, int k);

// Computes the effective address of a floating-point operand
template <Core C, Mode M> u32 computeFpuEA(int n, FpuFormat fmt);

// Jumps to an exception vector
template <Core C, Flags F = 0> void jumpToVector(int nr);

// Computes the number of extension words provided in full extension format
int baseDispWords(u16 ext) const;
int outerDispWords(u16 ext) const;

// Computes the cycle penalty for extended 68020 addressing modes
template <Core C, Mode M, Size S> int penaltyCycles(u16 ext) const;
