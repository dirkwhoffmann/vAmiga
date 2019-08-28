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
    if (activeAmiga && activeAmiga->mem.chipRam) {

        Memory *mem = &activeAmiga->mem;

        /* When we reach here, we expect that memory has been initialised
         * already. If that's the case, the first memory page is mapped to
         * either Boot Rom or Kickstart Rom.
         */
        assert(mem->memSrc[0x0] == MEM_BOOT || mem->memSrc[0x0] == MEM_KICK);

        result = activeAmiga->mem.spypeek32(ADDRESS_68K(REG_PC));
    }
    REG_PC += 4;

    return result;
}

extern "C" uint32_t read_sp_on_reset(void) { return read_on_reset(0); }
extern "C" uint32_t read_pc_on_reset(void) { return read_on_reset(0); }


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
}

void
CPU::_reset()
{
    debug(CPU_DEBUG, "CPU::_reset()\n");

    RESET_SNAPSHOT_ITEMS
    irqLevel = -1;

    // Reset the Musashi CPU core
    memset(&m68ki_cpu, 0, sizeof(m68ki_cpu));
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);
    m68k_set_int_ack_callback(interrupt_handler);
    m68k_pulse_reset();

    // Remove all previously recorded instructions
    clearTraceBuffer();

    _dumpMusashi(); 
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
    plainmsg("                int_cycles : %d\n", m68ki_cpu.int_cycles);
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
          fnv_1a(buffer, reader.ptr - buffer), reader.ptr - buffer);

    return reader.ptr - buffer;
}

size_t
CPU::didSaveToBuffer(uint8_t *buffer) const
{
    SerWriter writer(buffer);

    uint8_t context[m68k_context_size()];
    m68k_get_context(context);
    writer.copy(context, m68k_context_size());

    debug("(actions) = %d\n",  (actions));
    debug("(irqLevel) = %d\n",  (irqLevel));
    debug("(waitStates) = %d\n",  (waitStates));

    debug(SNAP_DEBUG, "CPU state checksum: %x (%d bytes)\n",
          fnv_1a(buffer, writer.ptr - buffer), writer.ptr - buffer);

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

uint32_t
CPU::getPC()
{
    return m68k_get_reg(NULL, M68K_REG_PC);
}

void
CPU::setPC(uint32_t value)
{
    m68k_set_reg(M68K_REG_PC, value);
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
    instr.cycle = amiga->getMasterClock();
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

uint64_t
CPU::executeInstruction()
{
    int cycles = 0;

    // Check actions flags
    if (actions) {

        if (actions & CPU_SET_IRQ_LEVEL) {
            debug(IRQ_DEBUG, "Changing IRQ level to %d\n", irqLevel);
            m68k_set_irq(irqLevel);
        }

        if (actions & CPU_ADD_WAIT_STATES) {
            debug(CPU_DEBUG, "Adding %d wait states\n", waitStates);
            cycles += waitStates;
            waitStates = 0;
        }

        actions = 0;
    }

    cycles += m68k_execute(1);
    return cycles;
}

void
CPU::setIrqLevel(int level)
{
    if (irqLevel != level)
    {
        debug(IRQ_DEBUG, "IRQ level changed from %d to %d\n", irqLevel, level);

        irqLevel = level;
        actions |= CPU_SET_IRQ_LEVEL;
    }
}

void
CPU::addWaitStates(Cycle number)
{
    if (number) {
        waitStates += number;
        actions |= CPU_ADD_WAIT_STATES;
    }
}

unsigned int
CPU::interruptHandler(unsigned int irqLevel)
{
    debug(CPU_DEBUG, "interruptHandler(%d)\n");

    // Do nothing here. I.e., don't automatically clear the interrupt

    return M68K_INT_ACK_AUTOVECTOR;
}
