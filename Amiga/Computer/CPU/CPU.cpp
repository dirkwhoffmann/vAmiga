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

//
// Interface to Musashi
//

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
    activeAmiga->mem.poke8(addr, value);
}

extern "C" void m68k_write_memory_16(unsigned int addr, unsigned int value)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke16<BUS_CPU>(addr, value);
}

extern "C" void m68k_write_memory_32(unsigned int addr, unsigned int value)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke32(addr, value);
}

extern "C" int interrupt_handler(int irqLevel)
{
    assert(activeAmiga != NULL);
    return activeAmiga->cpu.interruptHandler(irqLevel);
}

extern "C" uint32_t read_on_reset(uint32_t defaultValue)
{
    uint32_t result = defaultValue;

    // Check for attached memory
    if (activeAmiga && activeAmiga->mem.chip) {

        Memory *mem = &activeAmiga->mem;

        /* When we reach here, we expect memory to be initialised already.
         * If that's the case, the first memory page is mapped to Rom.
         */
        assert(mem->memSrc[0x0] == MEM_ROM ||
               mem->memSrc[0x0] == MEM_EXT);

        result = activeAmiga->mem.spypeek32(ADDRESS_68K(REG_PC));
    }
    REG_PC += 4;

    return result;
}

extern "C" uint32_t read_sp_on_reset(void) { return read_on_reset(0); }
extern "C" uint32_t read_pc_on_reset(void) { return read_on_reset(0); }

//
// Interface to Moira
//

namespace moira {

void
Moira::sync(int cycles)
{
    clock += cycles;
}

u8
Moira::read8(u32 addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.peek8(addr);
}

u16
Moira::read16(u32 addr)
{
     assert(activeAmiga != NULL);
     return activeAmiga->mem.peek16<BUS_CPU>(addr);
}

u16
Moira::read16Dasm(u32 addr)
{
    assert(activeAmiga != NULL);
    return activeAmiga->mem.spypeek16(addr);
}

u16
Moira::read16OnReset(u32 addr)
{
    u16 result = 0;

    if (activeAmiga && activeAmiga->mem.chip) result = read16(addr);

    printf("read16OnReset(%x) = %x\n", addr, result);
    return result;
}

void
Moira::write8(u32 addr, u8 val)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke8(addr, val);
}

void
Moira::write16 (u32 addr, u16 val)
{
    assert(activeAmiga != NULL);
    activeAmiga->mem.poke16<BUS_CPU>(addr, val);
}

void
Moira::willPollIrq()
{
}

}


//
// CPU class
//

CPU::CPU(Amiga& ref) : SubComponent(ref)
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
CPU::_initialize()
{
    debug(CPU_DEBUG, "CPU::_initialize()\n");

    // Initialize the Musashi CPU core
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_int_ack_callback(interrupt_handler);
}

void
CPU::_powerOn()
{
    debug(CPU_DEBUG, "CPU::_powerOn()\n");

    // Grab the Musashi core
    makeActiveInstance();
}

void
CPU::_powerOff()
{
    if (activeAmiga == &amiga) {
        debug("Stop being the active emulator instance\n");
        activeAmiga = NULL;
    }
}

void
CPU::_run()
{
    debug(CPU_DEBUG, "CPU::_run()\n");

    // Grab the Musashi core
    makeActiveInstance();
}

void
CPU::_reset()
{
    debug(CPU_DEBUG, "CPU::_reset()\n");

    // REMOVE ASAP
    makeActiveInstance();

    RESET_SNAPSHOT_ITEMS

    moiracpu.reset();

    // Remove all previously recorded instructions
    clearTraceBuffer();
}

void
CPU::_inspect()
{
    uint32_t pc;

    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    pc = getPC();

    // Registers
    info.pc = pc;

    for (int i = 0; i < 8; i++) {
        info.d[i] = moiracpu.getD(i);
        info.a[i] = moiracpu.getA(i);
    }
    info.ssp = moiracpu.getSSP();
    info.flags = moiracpu.getSR();

    // Disassemble the program starting at the program counter
    for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
        disassemble(pc, info.instr[i]);
        pc += info.instr[i].bytes;
    }

    // Disassemble the most recent entries in the trace buffer

    /* The last element in the trace buffer is the instruction that will be
     * be executed next. Because we don't want to show this element yet, we
     * don't dissassemble it.
     */
    for (unsigned i = 1; i <= CPUINFO_INSTR_COUNT; i++) {
        unsigned offset = (writePtr + traceBufferCapacity - 1 - i) % traceBufferCapacity;
        RecInstr recInstr = traceBuffer[offset];
        disassemble(recInstr, info.traceInstr[CPUINFO_INSTR_COUNT - i]);
    }

    pthread_mutex_unlock(&lock);
}

void
CPU::_dumpConfig()
{
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

DisInstr
CPU::getInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisInstr result;
    
    pthread_mutex_lock(&lock);
    result = info.instr[index];
    pthread_mutex_unlock(&lock);
    
    return result;
}

DisInstr
CPU::getTracedInstrInfo(long index)
{
    assert(index < CPUINFO_INSTR_COUNT);
    
    DisInstr result;
    
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

    return counter.count;
}

size_t
CPU::didLoadFromBuffer(uint8_t *buffer)
{
    SerReader reader(buffer);

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, reader.ptr - buffer), reader.ptr - buffer);

    return reader.ptr - buffer;
}

size_t
CPU::didSaveToBuffer(uint8_t *buffer)
{
    SerWriter writer(buffer);

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, writer.ptr - buffer), writer.ptr - buffer);

    return writer.ptr - buffer;
}

void
CPU::makeActiveInstance()
{
    // Return immediately if this emulator instance is the active instance
    if (activeAmiga == &amiga) return;

    /* Pause the currently active emulator instance (if any)
     * Because we're going to use the CPU core, we need to save the active
     * instance's CPU context. It will be restored when the other instance
     * becomes the active again (by calling this function).
     */
    if (activeAmiga != NULL) {
        activeAmiga->pause();
        // activeAmiga->cpu.recordContext();
    }

    // Restore the previously recorded CPU state (if any)
    // restoreContext();

    // Bind the CPU core to this emulator instance
    activeAmiga = &amiga;
}

uint32_t
CPU::getPC() { return moiracpu.getPC(); }

void
CPU::setPC(uint32_t value) { moiracpu.setPC(value); }

uint16_t
CPU::getSR() { return moiracpu.getSR(); }

uint32_t
CPU::getIR() { return moiracpu.getIRD(); }

uint32_t
CPU::lengthOfInstruction(uint32_t addr)
{
    char tmp[128];
    return moiracpu.disassemble(addr, tmp);
}

void
CPU::disassemble(uint32_t addr, DisInstr &result)
{
    result.bytes = moiracpu.disassemble(addr, result.instr);

    mem.hex(result.data, addr, result.bytes, sizeof(result.data));
    sprint24x(result.addr, addr);

    result.flags[0] = 0;
}

void
CPU::disassemble(RecInstr recInstr, DisInstr &result)
{
    uint32_t pc = recInstr.pc;
    uint16_t sr = recInstr.sr;

    disassemble(pc, result);
    
    result.flags[0]  = (sr & 0b1000000000000000) ? 'T' : 't';
    result.flags[1]  = '-';
    result.flags[2]  = (sr & 0b0010000000000000) ? 'S' : 's';
    result.flags[3]  = '-';
    result.flags[4]  = '-';
    result.flags[5]  = (sr & 0b0000010000000000) ? '1' : '0';
    result.flags[6]  = (sr & 0b0000001000000000) ? '1' : '0';
    result.flags[7]  = (sr & 0b0000000100000000) ? '1' : '0';
    result.flags[8]  = '-';
    result.flags[9]  = '-';
    result.flags[10] = '-';
    result.flags[11] = (sr & 0b0000000000010000) ? 'X' : 'x';
    result.flags[12] = (sr & 0b0000000000001000) ? 'N' : 'n';
    result.flags[13] = (sr & 0b0000000000000100) ? 'Z' : 'z';
    result.flags[14] = (sr & 0b0000000000000010) ? 'V' : 'v';
    result.flags[15] = (sr & 0b0000000000000001) ? 'C' : 'c';
    result.flags[16] = 0;
}

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
    RecInstr instr;
    
    // Setup record
    instr.pc = getPC();
    instr.sr = getSR();

    // Store record
    assert(writePtr < traceBufferCapacity);
    traceBuffer[writePtr] = instr;

    // Advance write pointer
    writePtr = (writePtr + 1) % traceBufferCapacity;

    // DisassembledInstruction diss = disassemble(instr.pc);
    // plainmsg("%X: %s\n", diss.addr, diss.instr);
}

int trace = 0;
long traceCnt = 0;
long instrCount = 0;

Cycle
CPU::executeInstruction()
{
    char str[64];
    instrCount++;

    uint32_t pc = getPC();

    if (pc == 0xFC30C2) trace = 0;
    if (trace && traceCnt++ > 1000) trace = 0;

    if (trace) {
        moiracpu.disassemble(pc, str);
        printf("%ld [%lld] %x: %s\n", instrCount, getClock(), pc, str);
    }
    moiracpu.execute();
    clock = CPU_CYCLES(moiracpu.getClock());

    return clock;
}

void
CPU::setIrqLevel(int level)
{
    assert(level < 8);

    if (irqLevel != level)
    {
        debug(INT_DEBUG, "IRQ level changed from %d to %d\n", irqLevel, level);

        irqLevel = level;
        moiracpu.setIPL(level);
    }
}

void
CPU::addWaitStates(Cycle number)
{
    waitStates += number;
    /*
    if (number) {
        waitStates += number;
        actions |= CPU_ADD_WAIT_STATES;
    }
    */
}

unsigned int
CPU::interruptHandler(unsigned int irqLevel)
{
    debug(INT_DEBUG, "interruptHandler(%d)\n", irqLevel);

    // Do nothing here. I.e., don't automatically clear the interrupt

    return M68K_INT_ACK_AUTOVECTOR;
}
