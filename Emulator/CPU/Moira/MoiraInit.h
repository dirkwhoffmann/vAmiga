// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This file declares all instruction handlers. All handlers follow a common
 * naming scheme:
 *
 *    execXXX : Handler for executing an instruction
 *    dasmXXX : Handler for disassembling an instruction
 */

#define MOIRA_DECLARE_EXEC_IMS(x) \
template<Instr I, Mode M, Size S> void exec##x(u16 op);

#define MOIRA_DECLARE_EXEC_CIMS(x) \
template<Type CPU, Instr I, Mode M, Size S> void exec##x(u16 op);

#define MOIRA_DECLARE_DASM_IMS(x) \
template<Instr I, Mode M, Size S> void dasm##x(StrWriter &str, u32 &addr, u16 op);

#define MOIRA_DECLARE_IMS(x) \
MOIRA_DECLARE_EXEC_IMS(x) \
MOIRA_DECLARE_DASM_IMS(x)

#define MOIRA_DECLARE_CIMS(x) \
MOIRA_DECLARE_EXEC_CIMS(x) \
MOIRA_DECLARE_DASM_IMS(x)

#define MOIRA_DECLARE_SIMPLE(x) \
void dasm##x(StrWriter &str, u32 &addr, u16 op); \
void exec##x(u16 op);

//
// Handlers (68000+)
//

MOIRA_DECLARE_SIMPLE(LineA)
MOIRA_DECLARE_SIMPLE(LineF)
MOIRA_DECLARE_SIMPLE(Illegal)

MOIRA_DECLARE_IMS(ShiftRg)
MOIRA_DECLARE_IMS(ShiftIm)
MOIRA_DECLARE_IMS(ShiftEa)

MOIRA_DECLARE_IMS(Abcd)
MOIRA_DECLARE_IMS(AddEaRg)
MOIRA_DECLARE_IMS(AddRgEa)
MOIRA_DECLARE_IMS(Adda)
MOIRA_DECLARE_CIMS(AddiRg)
MOIRA_DECLARE_IMS(AddiEa)
MOIRA_DECLARE_IMS(AddqDn)
MOIRA_DECLARE_IMS(AddqAn)
MOIRA_DECLARE_IMS(AddqEa)
MOIRA_DECLARE_IMS(AddxRg)
MOIRA_DECLARE_IMS(AddxEa)
MOIRA_DECLARE_IMS(AndEaRg)
MOIRA_DECLARE_IMS(AndRgEa)
MOIRA_DECLARE_CIMS(AndiRg)
MOIRA_DECLARE_IMS(AndiEa)
MOIRA_DECLARE_CIMS(Andiccr)
MOIRA_DECLARE_CIMS(Andisr)

MOIRA_DECLARE_IMS(Bcc)
MOIRA_DECLARE_IMS(BitDxEa)
MOIRA_DECLARE_IMS(BitImEa)
MOIRA_DECLARE_IMS(Bkpt)
MOIRA_DECLARE_IMS(Bsr)

MOIRA_DECLARE_CIMS(Chk)
MOIRA_DECLARE_CIMS(Clr)
MOIRA_DECLARE_IMS(Cmp)
MOIRA_DECLARE_IMS(Cmpa)
MOIRA_DECLARE_CIMS(CmpiRg)
MOIRA_DECLARE_IMS(CmpiEa)
MOIRA_DECLARE_IMS(Cmpm)

MOIRA_DECLARE_CIMS(Dbcc)
MOIRA_DECLARE_CIMS(Div)

MOIRA_DECLARE_IMS(ExgDxDy)
MOIRA_DECLARE_IMS(ExgAxDy)
MOIRA_DECLARE_IMS(ExgAxAy)
MOIRA_DECLARE_IMS(Ext)

MOIRA_DECLARE_IMS(Jmp)
MOIRA_DECLARE_IMS(Jsr)

MOIRA_DECLARE_IMS(Lea)
MOIRA_DECLARE_IMS(Link)

MOIRA_DECLARE_IMS(Move0)
MOIRA_DECLARE_IMS(Move2)
MOIRA_DECLARE_IMS(Move3)
MOIRA_DECLARE_IMS(Move4)
MOIRA_DECLARE_IMS(Move5)
MOIRA_DECLARE_IMS(Move6)
MOIRA_DECLARE_IMS(Move7)
MOIRA_DECLARE_IMS(Move8)
MOIRA_DECLARE_IMS(Movea)
MOIRA_DECLARE_IMS(MovecRcRx)
MOIRA_DECLARE_IMS(MovecRxRc)
MOIRA_DECLARE_IMS(MovemEaRg)
MOIRA_DECLARE_IMS(MovemRgEa)
MOIRA_DECLARE_IMS(MovepDxEa)
MOIRA_DECLARE_IMS(MovepEaDx)
MOIRA_DECLARE_IMS(Moveq)
MOIRA_DECLARE_IMS(MoveFromCcrRg)
MOIRA_DECLARE_IMS(MoveFromCcrEa)
MOIRA_DECLARE_IMS(MoveToCcr)
MOIRA_DECLARE_CIMS(MoveFromSrRg)
MOIRA_DECLARE_CIMS(MoveFromSrEa)
MOIRA_DECLARE_IMS(Moves)
MOIRA_DECLARE_IMS(MoveToSr)
MOIRA_DECLARE_IMS(MoveUspAn)
MOIRA_DECLARE_IMS(MoveAnUsp)
MOIRA_DECLARE_CIMS(Mul)

MOIRA_DECLARE_IMS(Nbcd)
MOIRA_DECLARE_IMS(NegRg)
MOIRA_DECLARE_IMS(NegEa)
MOIRA_DECLARE_IMS(Nop)

MOIRA_DECLARE_IMS(Pea)

MOIRA_DECLARE_CIMS(Reset)
MOIRA_DECLARE_IMS(Rtd)
MOIRA_DECLARE_CIMS(Rte)
MOIRA_DECLARE_IMS(Rtr)
MOIRA_DECLARE_IMS(Rts)

MOIRA_DECLARE_IMS(SccRg)
MOIRA_DECLARE_IMS(SccEa)
MOIRA_DECLARE_IMS(Stop)
MOIRA_DECLARE_IMS(Swap)

MOIRA_DECLARE_IMS(TasRg)
MOIRA_DECLARE_IMS(TasEa)
MOIRA_DECLARE_IMS(Trap)
MOIRA_DECLARE_IMS(Trapv)
MOIRA_DECLARE_IMS(Tst)

MOIRA_DECLARE_IMS(Unlk)


// Musashi compatibility mode
template<Type CPU, Instr I, Mode M, Size S> void execMulMusashi(u16 op);
template<Type CPU, Instr I, Mode M, Size S> void execDivMusashi(u16 op);
