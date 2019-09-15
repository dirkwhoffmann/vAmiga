// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CPU_INC
#define _CPU_INC

#include "SubComponent.h"
#include "BreakpointManager.h"

/* vAmiga utilizes the Musashi CPU core for emulating the Amiga CPU.
 *
 * To be compatible with vAmiga, the core had to be patched as follows:
 *
 * - In function void m68k_pulse_reset(void):
 *
 *       The two assignments
 *            REG_SP = m68ki_read_imm_32();
 *            REG_PC = m68ki_read_imm_32();
 *
 *       have been replaced by:
 *            REG_SP = read_sp_on_reset();
 *            REG_PC = read_pc_on_reset();
 *
 *  The change was necessary, because m68ki_read_imm_32() invokes vAmiga's
 *  standard Memory::peek32() function which requires the emulator to be
 *  running.
 */

extern "C" {
#include "m68k.h"
#include "m68kcpu.h"
}

extern "C" int interrupt_handler(int irqLevel);
extern "C" uint32_t read_on_reset(uint32_t defaultValue);
extern "C" uint32_t read_sp_on_reset(void);
extern "C" uint32_t read_pc_on_reset(void);


//
// CPU wrapper class
//

class CPU : public SubComponent {

    // The current configuration
    CPUConfig config;

    // Information shown in the GUI inspector panel
    CPUInfo info;


    //
    // Internal state
    //

public:

    // The CPU has been emulated up to this clock cycle
    Cycle clock;

private:

    // Action flags
    uint8_t actions;

    // The new interrupt level
    int irqLevel;

    // Number of wait states if the CPU can't access the bus
    CPUCycle waitStates;


    //
    // CPU state switching
    //
    
private:
    
    /* CPU context
     * This variable is usually NULL. When the user switches to another
     * emulator instance, it is used to store the current context. When the
     * user switches back, the previously saved state is restored.
     */
    uint8_t *context = NULL;
    
public:
    
    //
    // Debuging tools
    //
    
    // A breakpoint manager for handling forced interruptions
    BreakpointManager bpManager = BreakpointManager(amiga);
    
    // A buffer recording all recently executed instructions
    static const size_t traceBufferCapacity = 256;
    RecordedInstruction traceBuffer[traceBufferCapacity];
    
    // The trace buffer write pointer
    int writePtr = 0;
    

    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clock
        & actions
        & irqLevel
        & waitStates;
    }


    //
    // Constructing and configuring
    //
    
public:
    
    CPU(Amiga& ref);
    ~CPU();

    // Returns the current configuration
    CPUConfig getConfig() { return config; }

    // Configures the speed acceleration factor (1, 2, or 4)
    int getSpeed();
    void setSpeed(int factor);

    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _reset() override;
    void _inspect() override;
    void _dumpConfig() override;
    void _dump() override;
    void _dumpMusashi();
    size_t _size() override;
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(uint8_t *buffer) override;
    size_t didSaveToBuffer(uint8_t *buffer) const override;

    
    //
    // Reading the internal state
    //
    
public:
    
    // Returns the latest internal state recorded by inspect()
    CPUInfo getInfo();
    DisassembledInstruction getInstrInfo(long nr);
    DisassembledInstruction getTracedInstrInfo(long nr);


    //
    // Recording and restoring the CPU context
    //
    
public:
    
    // Returns true if a CPU context has been saved previously
    bool hasSavedContext() { return context != NULL; }
    
    // Records the current CPU context
    void recordContext();
    
    // Restores the current CPU context
    void restoreContext();


    //
    // Working with the clock
    //

    // Advances the clock by a certain number of CPU cycles
    void advance(CPUCycle cycles) { clock += cycles << config.shift; }

    // Returns the clock in CPU cycles
    CPUCycle cycles() { return clock >> config.shift; }


    //
    // Querying registers
    //
    
public:
    
    // Getter and setter for the program counter.
    uint32_t getPC();
    void setPC(uint32_t value); 

    // Returns the current value of the status register.
    uint32_t getSP();
    
    // Returns the current value of the instruction register.
    uint32_t getIR();
    
    // Returns the start address of the following instruction.
    uint32_t getNextPC() { return getPC() + lengthOInstruction(); }


    //
    // Querying instructions
    //
    
    /* Returns the length of the instruction at the provided address in bytes.
     * Note: This function is slow, because it calls the disassembler
     * internally.
     */
    uint32_t lengthOfInstruction(uint32_t addr);
    
    // Returns the length of the currently executed instruction.
    uint32_t lengthOInstruction() { return lengthOfInstruction(getPC()); }


    //
    // Running the disassembler
    //
    
    // Disassembles the instruction at the specified address
    DisassembledInstruction disassemble(uint32_t addr);
    DisassembledInstruction disassemble(uint32_t addr, uint16_t sp);


    //
    // Tracing program execution
    //
    
    // Removes all elements from the trace buffer except the 'count' most recent ones.
    void truncateTraceBuffer(unsigned count);
    
    // Clears the trace buffer.
    void clearTraceBuffer() { truncateTraceBuffer(0); }

    // Returns the number of recorded instructions.
    // unsigned recordedInstructions();
    
    // Records an instruction.
    void recordInstruction();
    
    // Reads a recorded instruction from the trace buffer.
    // RecordedInstruction readRecordedInstruction(long offset);
    
    
    //
    // Running the device
    //
    
public:

    // Executes the next instruction and returns the new CPU clock value
    Cycle executeInstruction();

    // Changes the interrupt level
    void setIrqLevel(int level);

    // Adds wait states to the CPU
    void addWaitStates(CPUCycle number);


    //
    // Handling interrupts
    //

    // Returns the currently installed irq handler (for debugging)
    void *getIrqHandler() { return (void *)CALLBACK_INT_ACK; }

    // Called by Musashi core when an interrupt occurs
    unsigned int interruptHandler(unsigned int irqLevel);
};

#endif
