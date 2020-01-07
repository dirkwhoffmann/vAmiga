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

    // Grab the Musashi core
    makeActiveInstance();

    RESET_SNAPSHOT_ITEMS
    // irqLevel = -1;

    // Reset the Musashi CPU core
#ifdef HARD_RESET
    memset(&m68ki_cpu, 0, sizeof(m68ki_cpu));
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_int_ack_callback(interrupt_handler);
#endif
    m68k_pulse_reset();
    moiracpu.reset();

    // Remove all previously recorded instructions
    clearTraceBuffer();

    // _dumpMusashi(); 
}

void
CPU::_inspect()
{
    uint32_t pc;

    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    if (MUSASHI) {

        pc = getPC();

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

    } else {

        pc = getPC();

        // Registers
        info.pc = pc;

        for (int i = 0; i < 8; i++) {
            info.d[i] = moiracpu.getD(i);
            info.a[i] = moiracpu.getA(i);
        }
        info.ssp = moiracpu.getSSP();
        info.flags = moiracpu.getSR();
    }

    // Disassemble the program starting at the program counter
    for (unsigned i = 0; i < CPUINFO_INSTR_COUNT; i++) {
        // info.instr[i] = disassemble(pc);
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

void
CPU::_dumpMusashi()
{
    plainmsg("Musashi CPU:\n\n");

    plainmsg("                  cpu_type : %d\n", m68ki_cpu.cpu_type);
    plainmsg("                       dar : %d %d ...\n", m68ki_cpu.dar[0], m68ki_cpu.dar[1]);
    plainmsg("                       ppc : %d\n", m68ki_cpu.ppc);
    plainmsg("                        pc : %d\n", m68ki_cpu.pc);
    plainmsg("                        sp : %d %d ...\n", m68ki_cpu.sp[0], m68ki_cpu.sp[1]);
    plainmsg("                       vbr : %d\n", m68ki_cpu.vbr);
    plainmsg("                       sfc : %d\n", m68ki_cpu.sfc);
    plainmsg("                       dfc : %d\n", m68ki_cpu.dfc);
    plainmsg("                      cacr : %d\n", m68ki_cpu.cacr);
    plainmsg("                      caar : %d\n", m68ki_cpu.caar);
    plainmsg("                        ir : %d\n", m68ki_cpu.ir);
    plainmsg("                   t1_flag : %d\n", m68ki_cpu.t1_flag);
    plainmsg("                   t0_flag : %d\n", m68ki_cpu.t0_flag);
    plainmsg("                    s_flag : %d\n", m68ki_cpu.s_flag);
    plainmsg("                    m_flag : %d\n", m68ki_cpu.m_flag);
    plainmsg("                    x_flag : %d\n", m68ki_cpu.x_flag);
    plainmsg("                    n_flag : %d\n", m68ki_cpu.n_flag);
    plainmsg("                not_z_flag : %d\n", m68ki_cpu.not_z_flag);
    plainmsg("                    v_flag : %d\n", m68ki_cpu.v_flag);
    plainmsg("                    c_flag : %d\n", m68ki_cpu.c_flag);
    plainmsg("                  int_mask : %d\n", m68ki_cpu.int_mask);
    plainmsg("                 int_level : %d\n", m68ki_cpu.int_level);
    // plainmsg("                int_cycles : %d\n", m68ki_cpu.int_cycles);
    plainmsg("                   stopped : %d\n", m68ki_cpu.stopped);
    plainmsg("                 pref_addr : %d\n", m68ki_cpu.pref_addr);
    plainmsg("                 pref_data : %d\n", m68ki_cpu.pref_data);
    plainmsg("              address_mask : %d\n", m68ki_cpu.address_mask);
    plainmsg("                   sr_mask : %d\n", m68ki_cpu.sr_mask);
    plainmsg("                instr_mode : %d\n", m68ki_cpu.instr_mode);
    plainmsg("                  run_mode : %d\n", m68ki_cpu.run_mode);
    plainmsg("          cyc_bcc_notake_b : %d\n", m68ki_cpu.cyc_bcc_notake_b);
    plainmsg("          cyc_bcc_notake_w : %d\n", m68ki_cpu.cyc_bcc_notake_w);
    plainmsg("          cyc_dbcc_f_noexp : %d\n", m68ki_cpu.cyc_dbcc_f_noexp);
    plainmsg("            cyc_dbcc_f_exp : %d\n", m68ki_cpu.cyc_dbcc_f_exp);
    plainmsg("            cyc_scc_r_true : %d\n", m68ki_cpu.cyc_scc_r_true);
    plainmsg("               cyc_movem_w : %d\n", m68ki_cpu.cyc_movem_w);
    plainmsg("               cyc_movem_l : %d\n", m68ki_cpu.cyc_movem_l);
    plainmsg("                 cyc_shift : %d\n", m68ki_cpu.cyc_shift);
    plainmsg("                 cyc_reset : %d\n", m68ki_cpu.cyc_reset);
    plainmsg("           cyc_instruction : %p\n", m68ki_cpu.cyc_instruction);
    plainmsg("             cyc_exception : %p\n", m68ki_cpu.cyc_exception);
    plainmsg("\n");
    plainmsg("          int_ack_callback : %p\n", m68ki_cpu.int_ack_callback);
    plainmsg("         bkpt_ack_callback : %p\n", m68ki_cpu.bkpt_ack_callback);
    plainmsg("      reset_instr_callback : %p\n", m68ki_cpu.reset_instr_callback);
    plainmsg("       pc_changed_callback : %p\n", m68ki_cpu.pc_changed_callback);
    plainmsg("           set_fc_callback : %p\n", m68ki_cpu.set_fc_callback);
    plainmsg("       instr_hook_callback : %p\n", m68ki_cpu.instr_hook_callback);
    plainmsg("\n");
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

    counter.count += m68k_context_size();

    return counter.count;
}

size_t
CPU::didLoadFromBuffer(uint8_t *buffer)
{
    SerReader reader(buffer);

    uint8_t context[m68k_context_size()];
    reader.copy(context, m68k_context_size());
    m68k_set_context(context);

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, reader.ptr - buffer), reader.ptr - buffer);

    return reader.ptr - buffer;
}

size_t
CPU::didSaveToBuffer(uint8_t *buffer)
{
    SerWriter writer(buffer);

    uint8_t context[m68k_context_size()];
    m68k_get_context(context);
    writer.copy(context, m68k_context_size());

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a_64(buffer, writer.ptr - buffer), writer.ptr - buffer);

    return writer.ptr - buffer;
}

void
CPU::recordContext()
{
    debug(CPU_DEBUG, "recordContext: context = %p\n", context);
    assert(context == NULL);
    
    // Allocate memory
    context = new (std::nothrow) uint8_t[m68k_context_size()];
    
    // Save the current CPU context
    if (context) m68k_get_context(context);
}

void
CPU::restoreContext()
{
    debug(CPU_DEBUG, "restoreContext: context = %p\n", context);
    if (context) {
        
        // Load the recorded context into the CPU
        m68k_set_context(context);
        
        // Delete the recorded context
        delete[] context;
        context = NULL;
    }
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
        activeAmiga->cpu.recordContext();
    }

    // Restore the previously recorded CPU state (if any)
    restoreContext();

    // Bind the CPU core to this emulator instance
    activeAmiga = &amiga;
}

uint32_t
CPU::getPC()
{
    if (MUSASHI) {
        return m68k_get_reg(NULL, M68K_REG_PC);
    } else {
        return moiracpu.getPC();
    }
}

void
CPU::setPC(uint32_t value)
{
    if (MUSASHI) {
        m68k_set_reg(M68K_REG_PC, value);
    } else {
        moiracpu.setPC(value);
    }
}

uint16_t
CPU::getSR()
{
    if (MUSASHI) {
        return m68k_get_reg(NULL, M68K_REG_SR);
    } else {
        return moiracpu.getSR();
    }
}

uint32_t
CPU::getIR()
{
    if (MUSASHI) {
        return m68k_get_reg(NULL, M68K_REG_IR);
    } else {
        return moiracpu.getIRD();
    }
}

uint32_t
CPU::lengthOfInstruction(uint32_t addr)
{
    char tmp[128];

    if (MUSASHI) {
        return m68k_disassemble(tmp, addr, M68K_CPU_TYPE_68000);
    } else {
        return moiracpu.disassemble(addr, tmp);
    }
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

    if (actions) {

        // Check action flags
        if (actions & CPU_SET_IRQ_LEVEL1) {
            debug(INT_DEBUG, "Changing IRQ level to %d\n", irqLevel);
            if (MUSASHI) { m68k_set_irq(irqLevel); }
        }

        // Shift action flags
        actions = (actions << 1) & CPU_DELAY_MASK;
    }

    uint32_t pc = getPC();
    if (instrCount ==  1611000) {
        // printf("Tracing %s\n", MUSASHI ? "Musashi" : "Moira");
        // trace = 1;
    }
    if (pc == 0xFC30C2) trace = 0;
    if (trace && traceCnt++ > 1000) trace = 0;

    if (MUSASHI) {

        if (trace) {
            m68k_disassemble(str, pc, M68K_CPU_TYPE_68000);
            printf("%ld [%lld] %x: %s\n", instrCount, getClock(), pc, str);
        }

        clock += m68k_execute(1) << 2;
        // advance(m68k_execute(1));
        // if (waitStates) debug(CPU_DEBUG, "Adding %d wait states\n", waitStates);
        // clock += waitStates;
        waitStates = 0;

    } else {

        if (trace) {
            moiracpu.disassemble(pc, str);
            printf("%ld [%lld] %x: %s\n", instrCount, getClock(), pc, str);
        }
        moiracpu.execute();
        clock = CPU_CYCLES(moiracpu.getClock());
    }

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
        actions |= CPU_SET_IRQ_LEVEL0;

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
