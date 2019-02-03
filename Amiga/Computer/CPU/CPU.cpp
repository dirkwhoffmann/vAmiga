// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
extern "C" {
    #include "m68k.h"
}

// Reference to the active Amiga instance
Amiga *activeAmiga = NULL;

extern "C" unsigned int m68k_read_memory_8(unsigned int address)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek8(address);
}

extern "C" unsigned int m68k_read_memory_16(unsigned int address)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek16(address);
}

extern "C" unsigned int m68k_read_memory_32(unsigned int address)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek32(address);
}

extern "C" unsigned int m68k_read_disassembler_16 (unsigned int address)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek16(address);
}

extern "C" unsigned int m68k_read_disassembler_32 (unsigned int address)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek32(address);
}

extern "C" void m68k_write_memory_8(unsigned int address, unsigned int value)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke8(address, value);
}

extern "C" void m68k_write_memory_16(unsigned int address, unsigned int value)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke16(address, value);
}

extern "C" void m68k_write_memory_32(unsigned int address, unsigned int value)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke32(address, value);
}

//
// CPU class
//

CPU::CPU()
{
    setDescription("CPU");
    
    // Register snapshot items
    /*
    registerSnapshotItems(vector<SnapshotItem> {
        { &clock, sizeof(clock), 0 },
    });
    */
}

CPU::~CPU()
{

}

void
CPU::_powerOn()
{

}

void
CPU::_powerOff()
{
    
}

void
CPU::_reset()
{
    
}

void
CPU::_ping()
{
    
}

void
CPU::_dump()
{
    
}

uint64_t
CPU::executeNextInstruction()
{
    return 7; // Consumed cycles
}
