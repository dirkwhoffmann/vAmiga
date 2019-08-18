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

int64_t cpuInstrCount = 0;

// Reference to the active Amiga instance
Amiga *activeAmiga = NULL;

extern "C" unsigned int m68k_read_memory_8(unsigned int addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek8(addr);
}

extern "C" unsigned int m68k_read_memory_16(unsigned int addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek16<BUS_CPU>(addr);
}

extern "C" unsigned int m68k_read_memory_32(unsigned int addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek32(addr);
}

extern "C" unsigned int m68k_read_disassembler_16 (unsigned int addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek16(addr);
}

extern "C" unsigned int m68k_read_disassembler_32 (unsigned int addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek32(addr);
}

extern "C" void m68k_write_memory_8(unsigned int addr, unsigned int value)
{
    assert(activeAmiga != NULL);
    /*
    if (addr == 0x15150 || (addr >= 0xB160 && addr <= 0xB180)) printf("CPU::poke8(%x), %x\n", addr, value);
    */
    activeAmiga->mem.poke8(addr, value);
}

extern "C" void m68k_write_memory_16(unsigned int addr, unsigned int value)
{
    assert(activeAmiga != NULL);

    // if (addr >= 0xC2F3A0 && addr <= 0xC2F3B0) printf("%X CPU::poke16(%x), %x\n", addr, value);

    activeAmiga->mem.poke16<BUS_CPU>(addr, value);
}

extern "C" void m68k_write_memory_32(unsigned int addr, unsigned int value)
{
    assert(activeAmiga != NULL);

    /*
    if (addr >= 0xC2F3A0 && addr <= 0xC2F3B0) printf("%X CPU::poke32(%x), %x\n", activeAmiga->cpu.getPC(), addr, value);
        if (addr == 0x15150) activeAmiga->runLoopCtrl |= RL_STOP;
    }
    */
    activeAmiga->mem.poke32(addr, value);
}

//
// CPU class
//

CPU::CPU()
{
    setDescription("CPU");
    
    subComponents = vector<HardwareComponent *> {
        
        &bpManager,
    };
}

CPU::~CPU()
{
    
}

void
CPU::_powerOn()
{
}

void
CPU::_reset()
{
    RESET_SNAPSHOT_ITEMS
    clearTraceBuffer();
}

void
CPU::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    uint32_t pc = getPC();
    
    // Registers
    info.pc = pc;
    
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
    info.flags = m68k_get_reg(NULL, M68K_REG_SR);
    
    // Disassemble the program starting at the program counter
    for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
        info.instr[i] = disassemble(pc);
        pc += info.instr[i].bytes;
    }
    
    // Disassemble the most recent entries in the trace buffer
    
    /* The last element in the trace buffer is the instruction that will be
     * be executed next. Because we don't want to show this element yet, we
     * don't dissassemble it.
     */
    for (unsigned i = 1; i <= CPUINFO_INSTR_COUNT; i++) {
        unsigned offset = (writePtr + traceBufferCapacity - 1 - i) % traceBufferCapacity;
        RecordedInstruction instr = traceBuffer[offset];
        info.traceInstr[CPUINFO_INSTR_COUNT - i] = disassemble(instr.pc, instr.sp);
    }
    
    pthread_mutex_unlock(&lock);
}

void
CPU::_dump()
{
    _inspect();
    
    plainmsg("      PC: %8X\n", info.pc);
    plainmsg(" D0 - D3: ");
    for (unsigned i = 0; i < 4; i++) plainmsg("%8X ", info.d[i]);
    plainmsg("\n");
    plainmsg(" D4 - D7: ");
    for (unsigned i = 4; i < 8; i++) plainmsg("%8X ", info.d[i]);
    plainmsg("\n");
    plainmsg(" A0 - A3: ");
    for (unsigned i = 0; i < 4; i++) plainmsg("%8X ", info.a[i]);
    plainmsg("\n");
    plainmsg(" A4 - A7: ");
    for (unsigned i = 4; i < 8; i++) plainmsg("%8X ", info.a[i]);
    plainmsg("\n");
    plainmsg("     SSP: %X\n", info.ssp);
    plainmsg("   Flags: %X\n", info.flags);
}

CPUInfo
CPU::getInfo()
{
    CPUInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

DisassembledInstruction
CPU::getInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisassembledInstruction result;
    
    pthread_mutex_lock(&lock);
    result = info.instr[index];
    pthread_mutex_unlock(&lock);
    
    return result;
}

DisassembledInstruction
CPU::getTracedInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisassembledInstruction result;
    
    pthread_mutex_lock(&lock);
    result = info.traceInstr[index];
    pthread_mutex_unlock(&lock);
    
    return result;
}

size_t
CPU::_size()
{
    SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);

    counter.count += m68k_context_size();

    debug(SNAP_DEBUG, "Snapshot size is %d bytes\n", counter.count);
    return counter.count;
}

size_t
CPU::didLoadFromBuffer(uint8_t *buffer)
{
    SerReader reader(buffer);

    uint8_t context[m68k_context_size()];
    reader.copy(context, m68k_context_size());
    m68k_set_context(context);

    return reader.ptr - buffer;
}

size_t
CPU::didSaveToBuffer(uint8_t *buffer) const
{
    SerWriter writer(buffer);

    uint8_t context[m68k_context_size()];
    m68k_get_context(context);
    writer.copy(context, m68k_context_size());

    return writer.ptr - buffer;
}

void
CPU::recordContext()
{
    // debug("recordContext: context = %p\n", context);
    assert(context == NULL);
    
    // Allocate memory
    context = new (std::nothrow) uint8_t[m68k_context_size()];
    
    // Save the current CPU context
    if (context) m68k_get_context(context);
}

void
CPU::restoreContext()
{
    // debug("restoreContext: context = %p\n", context);
    if (context) {
        
        // Load the recorded context into the CPU
        m68k_set_context(context);
        
        // Delete the recorded context
        delete[] context;
        context = NULL;
    }
}

uint32_t
CPU::getPC()
{
    return m68k_get_reg(NULL, M68K_REG_PC);
}

uint32_t
CPU::getSP()
{
    return m68k_get_reg(NULL, M68K_REG_SP);
}

uint32_t
CPU::getIR()
{
    return m68k_get_reg(NULL, M68K_REG_IR);
}

uint32_t
CPU::lengthOfInstruction(uint32_t addr)
{
    char tmp[128];
    return m68k_disassemble(tmp, addr, M68K_CPU_TYPE_68000);
}

DisassembledInstruction
CPU::disassemble(uint32_t addr)
{
    DisassembledInstruction result;
    
    if (addr <= 0xFFFFFF) {
        
        result.bytes = m68k_disassemble(result.instr, addr, M68K_CPU_TYPE_68000);
        amiga->mem.hex(result.data, addr, result.bytes, sizeof(result.data));
        sprint24x(result.addr, addr);
        
    } else {
        
        result.bytes = result.instr[0] = result.data[0] = result.addr[0] = 0;
    }

    // Flags ("" by default)
    result.flags[0] = 0;

    return result;
}

DisassembledInstruction
CPU::disassemble(uint32_t addr, uint16_t sp)
{
    DisassembledInstruction result = disassemble(addr);
    
    result.flags[0]  = (sp & 0b1000000000000000) ? '1' : '0';
    result.flags[1]  = '-';
    result.flags[2]  = (sp & 0b0010000000000000) ? '1' : '0';
    result.flags[3]  = '-';
    result.flags[4]  = '-';
    result.flags[5]  = (sp & 0b0000010000000000) ? '1' : '0';
    result.flags[6]  = (sp & 0b0000001000000000) ? '1' : '0';
    result.flags[7]  = (sp & 0b0000000100000000) ? '1' : '0';
    result.flags[8]  = '-';
    result.flags[9]  = '-';
    result.flags[10] = '-';
    result.flags[11] = (sp & 0b0000000000010000) ? '1' : '0';
    result.flags[12] = (sp & 0b0000000000001000) ? '1' : '0';
    result.flags[13] = (sp & 0b0000000000000100) ? '1' : '0';
    result.flags[14] = (sp & 0b0000000000000010) ? '1' : '0';
    result.flags[15] = (sp & 0b0000000000000001) ? '1' : '0';
    result.flags[16] = 0;

    return result;
}

/*
unsigned
CPU::recordedInstructions()
{
    return (traceBufferCapacity + writePtr - readPtr) % traceBufferCapacity;
}
*/

void
CPU::truncateTraceBuffer(unsigned count)
{
    for (unsigned i = writePtr; i < writePtr + traceBufferCapacity - count; i++) {
        traceBuffer[i % traceBufferCapacity].pc = UINT32_MAX; // mark element as unsed
    }
}

void
CPU::recordInstruction()
{
    RecordedInstruction instr;
    
    // Setup record
    instr.cycle = amiga->masterClock;
    instr.pc = getPC();
    instr.sp = getSP();

    // Store record
    assert(writePtr < traceBufferCapacity);
    traceBuffer[writePtr] = instr;

    // Advance write pointer
    writePtr = (writePtr + 1) % traceBufferCapacity;

    // DisassembledInstruction diss = disassemble(instr.pc);
    // plainmsg("%X: %s\n", diss.addr, diss.instr);
}

#if 0
RecordedInstruction
CPU::readRecordedInstruction(long offset)
{
    assert(offset < traceBufferCapacity);
    
    size_t i = (readPtr + offset) % traceBufferCapacity;
    
    /*
    if (traceBuffer[i].instr[0] == 0) {
        uint16_t sp = traceBuffer[i].sp;
        m68k_disassemble(traceBuffer[i].instr, traceBuffer[i].pc, M68K_CPU_TYPE_68000);
        traceBuffer[i].flags[0]  = (sp & 0b1000000000000000) ? '1' : '0';
        traceBuffer[i].flags[1]  = '-';
        traceBuffer[i].flags[2]  = (sp & 0b0010000000000000) ? '1' : '0';
        traceBuffer[i].flags[3]  = '-';
        traceBuffer[i].flags[4]  = '-';
        traceBuffer[i].flags[5]  = (sp & 0b0000010000000000) ? '1' : '0';
        traceBuffer[i].flags[6]  = (sp & 0b0000001000000000) ? '1' : '0';
        traceBuffer[i].flags[7]  = (sp & 0b0000000100000000) ? '1' : '0';
        traceBuffer[i].flags[8]  = '-';
        traceBuffer[i].flags[9]  = '-';
        traceBuffer[i].flags[10] = '-';
        traceBuffer[i].flags[11] = (sp & 0b0000000000010000) ? '1' : '0';
        traceBuffer[i].flags[12] = (sp & 0b0000000000001000) ? '1' : '0';
        traceBuffer[i].flags[13] = (sp & 0b0000000000000100) ? '1' : '0';
        traceBuffer[i].flags[14] = (sp & 0b0000000000000010) ? '1' : '0';
        traceBuffer[i].flags[15] = (sp & 0b0000000000000001) ? '1' : '0';
        traceBuffer[i].flags[16] = 0;
    }
    */
    
    return traceBuffer[i];
}
#endif

uint64_t
CPU::executeNextInstruction()
{
    // debug("PC = %X\n", getPC());
    int cycles = m68k_execute(1);
    // debug("Executed %d CPU cycles. New PC = %X\n", cycles, getPC());
    
    return cycles;
}
