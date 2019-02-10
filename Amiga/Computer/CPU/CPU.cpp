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
    
    // Register sub components
    registerSubcomponents(vector<HardwareComponent *> {
        
        &bpManager,
    });
    
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

uint32_t
CPU::getPC()
{
    return m68k_get_reg(NULL, M68K_REG_PC);
}

CPUInfo
CPU::getInfo()
{
    CPUInfo info;
    
    info.pc = getPC();
    
    info.d[0] = m68k_get_reg(NULL, M68K_REG_D0);
    info.d[1] = m68k_get_reg(NULL, M68K_REG_D1);
    info.d[2] = m68k_get_reg(NULL, M68K_REG_D2);
    info.d[3] = m68k_get_reg(NULL, M68K_REG_D3);
    info.d[4] = m68k_get_reg(NULL, M68K_REG_D4);
    info.d[5] = m68k_get_reg(NULL, M68K_REG_D5);
    info.d[6] = m68k_get_reg(NULL, M68K_REG_D6);
    info.d[7] = m68k_get_reg(NULL, M68K_REG_D7);

    info.a[0] = m68k_get_reg(NULL, M68K_REG_A0);
    info.a[1] = m68k_get_reg(NULL, M68K_REG_A1);
    info.a[2] = m68k_get_reg(NULL, M68K_REG_A2);
    info.a[3] = m68k_get_reg(NULL, M68K_REG_A3);
    info.a[4] = m68k_get_reg(NULL, M68K_REG_A4);
    info.a[5] = m68k_get_reg(NULL, M68K_REG_A5);
    info.a[6] = m68k_get_reg(NULL, M68K_REG_A6);
    info.a[7] = m68k_get_reg(NULL, M68K_REG_A7);
    
    info.ssp = m68k_get_reg(NULL, M68K_REG_ISP);
  
    return info;
}

uint32_t
CPU::lengthOfInstruction(uint32_t addr)
{
    char tmp[128];
    return m68k_disassemble(tmp, addr, M68K_CPU_TYPE_68000);
}

uint64_t
CPU::executeNextInstruction()
{
    debug("PC = %X\n", getPC());
    int cycles = m68k_execute(1);
    debug("Executed %d CPU cycles. New PC = %X\n", cycles, getPC());
    
    return cycles;
}
