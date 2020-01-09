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

#include "AmigaComponent.h"
#include "BreakpointManager.h"
#include "Moira.h"

//
// CPU wrapper class
//

class CPU : public AmigaComponent {

    CPUInfo info;

public:

    moira::Moira moiracpu;


    //
    // Debuging tools
    //

public:

    // A breakpoint manager for handling forced interruptions
    BreakpointManager bpManager = BreakpointManager(amiga);
    
    // A buffer recording all recently executed instructions
    static const size_t traceBufferCapacity = 256;
    RecInstr traceBuffer[traceBufferCapacity];
    
    // The trace buffer write pointer
    int writePtr = 0;


    //
    // Constructing and destructing
    //
    
public:
    
    CPU(Amiga& ref);
    ~CPU();

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        /*
        worker

        & clock;
        */
    }


    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override; 
    void _run() override;
    void _reset() override;
    void _inspect() override;
    void _dumpConfig() override;
    void _dump() override;
    size_t _size() override;
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(uint8_t *buffer) override;
    size_t didSaveToBuffer(uint8_t *buffer) override;

public:

    // Returns the result of the most recent call to inspect()
    CPUInfo getInfo();
    DisInstr getInstrInfo(long nr);
    DisInstr getTracedInstrInfo(long nr);


    //
    // Recording and restoring the CPU context
    //
    
public:

    /* Assign the Musashi core to this CPU.
     * Background: Because we only have one CPU core available, we need to
     * share this core among all emulator instances. This means that only one
     * emulator instance can run at a time and the other instances need to
     * be powered off or paused. When an emulator instance needs to acces the
     * CPU core, it has to make itself the 'active emulator instance' by
     * calling this function. It will bind the CPU to this emulator instance
     * by rerouting all memory accesses to this instance.
     * If another instance is currently active, it is put into pause mode
     * automatically.
     * DEPRECATED
     */
    void makeActiveInstance();


    //
    // Working with the clock
    //

    // The CPU has been emulated up to this cycle
    Cycle getClock() { return CPU_CYCLES(moiracpu.getClock()); }

    // Returns the clock in CPU cycles
    CPUCycle cycles() { return moiracpu.getClock(); }

    //
    // Querying registers and instructions
    //
    
public:
    
    // Getter and setter for the program counter.
    uint32_t getPC() { return moiracpu.getPC(); }
    void setPC(uint32_t value) { moiracpu.setPC(value); } 

    // Returns the current value of the status register.
    uint16_t getSR() { return moiracpu.getSR(); }
    
    // Returns the current value of the instruction register.
    uint32_t getIR() { return moiracpu.getIRD(); }
    
    // Returns the start address of the following instruction.
    // DEPRECATED
    uint32_t getNextPC() { return getPC() + lengthOInstruction(); }

    /* Returns the length of the instruction at the provided address in bytes.
     * Note: This function is slow, because it calls the disassembler
     * internally.
     */
    // DEPRECATED
    uint32_t lengthOfInstruction(uint32_t addr);
    
    // Returns the length of the currently executed instruction.
    // DEPRECATED
    uint32_t lengthOInstruction() { return lengthOfInstruction(getPC()); }


    //
    // Running the disassembler
    //

    int disassemble(uint32_t addr, char *str);
    void disassemble(uint32_t addr, DisInstr &result);
    void disassemble(uint32_t addr, uint16_t sr, DisInstr &result);
    void disassemble(RecInstr instr, DisInstr &result);

    //
    // Tracing the program execution
    //
    
    // Removes all elements from the trace buffer except the 'count' most recent ones.
    void truncateTraceBuffer(unsigned count);
    
    // Clears the trace buffer.
    void clearTraceBuffer() { truncateTraceBuffer(0); }

    // Records an instruction.
    void recordInstruction();
    

    //
    // Running the device
    //
    
public:

    // Executes the next instruction and returns the new CPU clock value
    Cycle executeInstruction();

    // Changes the interrupt level
    void setIrqLevel(int level) { moiracpu.setIPL(level); }
};

#endif
