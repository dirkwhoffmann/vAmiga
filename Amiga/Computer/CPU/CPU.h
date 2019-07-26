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

#include "HardwareComponent.h"
#include "BreakpointManager.h"

extern "C" {
#include "m68k.h"
#include "m68kcpu.h"
}

extern int64_t cpuInstrCount;

class CPU : public HardwareComponent {
    
    private:
    
    // Information shown in the GUI inspector panel
    CPUInfo info;
    
    
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
    BreakpointManager bpManager;
    
    // A buffer recording all recently executed instructions
    static const size_t traceBufferCapacity = 256;
    RecordedInstruction traceBuffer[traceBufferCapacity];
    
    // The trace buffer read pointer
    // int readPtr;
    
    // The trace buffer write pointer
    int writePtr = 0;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    CPU();
    ~CPU();
    

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
    }

    
    //
    // Methods from HardwareComponent
    //
    
    private:

    void _powerOn() override;
    void _reset() override { RESET_SNAPSHOT_ITEMS }
    void _inspect() override; 
    void _dump() override;
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
    // Querying registers
    //
    
    public:
    
    // Returns the current value of the program counter.
    uint32_t getPC();
    
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
    
    uint64_t executeNextInstruction();
};

#endif
