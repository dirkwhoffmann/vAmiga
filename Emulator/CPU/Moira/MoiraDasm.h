// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Auxiliary functions
//

// Reads a word from memory and increments addr
template <Size S> u32 dasmRead(u32 &addr);

// Assembles an operand
template <Mode M, Size S> Ea<M,S> Op(u16 reg, u32 &pc);
