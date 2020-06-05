// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Moira utilizes a three layer design to model dataflow inside the processor.
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
 * Layer 2:       |         |           |               V                 |
 *                |         |           |            readExt              |
 *                V         V           V                                 V
 *              readD     readA      readM -----> addressError          readI
 *             (writeD)  (writeA)   (writeM)
 *                                      |
 *                                  updateAn()
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

/* Reads an operand
 *
 * The source of the operand is determined by the addressing mode M. If the
 * operand is read from memory, the effective address is stored in variable ea.
 * If the source is a register or an immediate value, variable ea remains
 * untouched.
 */
template<Mode M, Size S> bool readOp(int n, u32 &ea, u32 &result);

/* Writes an operand
 *
 * If parameter ea is omitted, the destination of the operand is determined
 * by the addressing mode M. Parameter 'last' indicates if this function is
 * initiates the last memory bus cycle of an instruction.
 */
template<Mode M, Size S, bool last = false> bool writeOp(int n, u32 val);
template<Mode M, Size S, bool last = false> void writeOp(int n, u32 ea, u32 val);

// Computes an effective address
template<Mode M, Size S, bool skip = false> u32 computeEA(u32 n);

// Emulates the address register modification for modes (An)+, (An)-
template<Mode M, Size S> void updateAn(int n);
template<Mode M, Size S> void updateAnPD(int n);
template<Mode M, Size S> void undoAnPD(int n);
template<Mode M, Size S> void updateAnPI(int n);

// Reads an operand from memory (without or with address error checking)
template<Size S, bool last = false> u32 readM(u32 addr);
template<Size S, bool last = false> u32 readM(u32 addr, bool &error);

// Writes an operand to memory (without or with address error checking)
template<Size S, bool last = false> void writeM(u32 addr, u32 val);
template<Size S, bool last = false> void writeM(u32 addr, u32 val, bool &error);

// Writes an operand to memory in reversed memory access order
template<Size S, bool last = false> void writeMrev(u32 addr, u32 val);
template<Size S, bool last = false> void writeMrev(u32 addr, u32 val, bool &error);

// Reads an immediate value from memory
template<Size S> u32 readI();

// Pushes a value onto the stack
template<Size S, bool last = false> void push(u32 value);
template<Size S, bool last = false> void push(u32 value, bool &error);

// Checks whether the provided address should trigger an address error
template<Size S = Word> bool misaligned(u32 addr);

// Creates an address error stack frame
AEStackFrame makeFrame(u32 addr, u32 pc, u16 sr, u16 ird, bool write = false);
AEStackFrame makeFrame(u32 addr, u32 pc, bool write = false);
AEStackFrame makeFrame(u32 addr, bool write = false);

//
/* Checks for an address error (DEPRECATED)
 * An address error occurs if the CPU tries to access a word or a long word
 * that is located at an odd address. If an address error is encountered,
 * the function calls execAddressError to initiate exception processing.
 */
/*
template<Size S, int delay = 0> bool addressReadError(u32 addr, u32 pc);
template<Size S, int delay = 0> bool addressWriteError(u32 addr, u32 pc);
template<Size S, int delay = 0> bool addressReadError(u32 addr);
template<Size S, int delay = 0> bool addressWriteError(u32 addr);
*/

// Prefetches the next instruction
template<bool last = false, int delay = 0> void prefetch();

// Prefetches the next instruction
// TODO: This function is going to replace the old replace function.
// TODO: It increases the pc while the the old one did not
template<bool last = false, int delay = 0> void newPrefetch();
void compensateNewPrefetch(); // WILL GO AWAY

// Performs a full prefetch cycle
template<bool last = false, int delay = 0> void fullPrefetch();

// Reads an extension word from memory
template<bool skip = false> void readExt();

// Jumps to an exception vector
void jumpToVector(int nr);
