// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

//
// Auxiliary functions
//

// Reads a data value from memory without side-effects
u32 dasmRead(u32 addr, Size S = Word) const;

// Increments addr and reads a data value from memory without side-effects
u32 dasmIncRead(u32 &addr, Size S = Word) const;

// Assembles an operand
Ea Op(Mode M, Size S, u16 reg, u32 &pc) const;
