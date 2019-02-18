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

#include "BreakpointManager.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class CPU : public HardwareComponent {
    
public:
    
    //
    // Debugging tools
    //
    
    // A breakpoint manager for handling forced interruptions
    BreakpointManager bpManager;
    
    // A ring buffer recording all recently executed instructions
    static const size_t traceBufferSize = 256;
    RecordedInstruction traceBuffer[traceBufferSize];
    
    // The trace buffer read pointer
    unsigned readPtr;
    
    // The trace buffer write pointer
    unsigned writePtr;
    
    
    // DEPRECATED
    //  map<uint32_t, Breakpoint> breakpoints;
    
public:
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    CPU();
    ~CPU();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
   
public:
    
    // Returns the current value of the program counter.
    uint32_t getPC();

    // Returns the current value of the status register.
    uint32_t getSP();

    // Returns the current value of the instruction register.
    uint32_t getIR();

    // Returns the start address of the following instruction.
    uint32_t getNextPC() { return getPC() + lengthOInstruction(); }

    // Collects the data shown in the GUI's debug panel.
    CPUInfo getInfo();
    
    
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
    // Tracing the program execution
    //
    
    // Clears the trace buffer.
    void clearTraceBuffer() { readPtr = writePtr = 0; }
    
    // Returns the number of recorded instructions.
    unsigned recordedInstructions();
    
    // Records an instruction.
    void recordInstruction();
    
    // Reads a recorded instruction from the trace buffer.
    RecordedInstruction readRecordedInstruction(long offset);
    
    
public:
    
    uint64_t executeNextInstruction();

    
};

#endif
