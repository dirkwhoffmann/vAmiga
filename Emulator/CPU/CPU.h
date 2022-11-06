// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CPUTypes.h"
#include "SubComponent.h"
#include "RingBuffer.h"
#include "Moira.h"

// using namespace moira;

class CPU : public moira::Moira {

    friend class Moira;
    
    // The current configuration
    CPUConfig config = {};

    // Result of the latest inspection
    mutable CPUInfo info = {};

    // Recorded call stack
    CallstackRecorder callstack;


    //
    // Overclocking
    //

public:

    // Sub-cycle counter
    i64 penalty;

    // Number of cycles that should be executed at normal speed
    i64 slowCycles;


    //
    // Initializing
    //

public:

    CPU(Amiga& ref);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "CPU"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;
    void _debugOn() override;
    void _debugOff() override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.revision
        << config.dasmRevision
        << config.overclocking
        << config.regResetVal;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << flags
            << clock
            << penalty
            << slowCycles
            
            << reg.pc
            << reg.pc0
            << reg.sr.t1
            << reg.sr.t0
            << reg.sr.s
            << reg.sr.m
            << reg.sr.x
            << reg.sr.n
            << reg.sr.z
            << reg.sr.v
            << reg.sr.c
            << reg.sr.ipl
            << reg.r
            << reg.usp
            << reg.isp
            << reg.msp
            << reg.ipl
            << reg.vbr
            << reg.sfc
            << reg.dfc
            << reg.cacr
            << reg.caar

            << queue.irc
            << queue.ird

            << ipl
            << fcl
            << fcSource
            << exception
            << cp;
        }
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override;
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
    
    //
    // Configuring
    //
    
public:
    
    const CPUConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
 
    
    //
    // Analyzing
    //
    
public:
    
    CPUInfo getInfo() const { return AmigaComponent::getInfo(info); }
        

    //
    // Working with the clock
    //

public:

    // Returns the clock in CPU cycles
    CPUCycle getCpuClock() const { return getClock(); }

    // Returns the clock in master cycles
    Cycle getMasterClock() const { return CPU_CYCLES(getClock()); }

    // Delays the CPU by a certain amout of master cycles
    void addWaitStates(Cycle cycles) { clock += AS_CPU_CYCLES(cycles); }
    
    // Resynchronizes an overclocked CPU with the Agnus clock
    void resyncOverclockedCpu();


    //
    // Running the disassembler
    //
    
    // Disassembles a recorded instruction from the log buffer
    const char *disassembleRecordedInstr(isize i, isize *len);
    const char *disassembleRecordedWords(isize i, isize len);
    const char *disassembleRecordedFlags(isize i);
    const char *disassembleRecordedPC(isize i);

    // Disassembles the instruction at the specified address
    const char *disassembleInstr(u32 addr, isize *len);
    const char *disassembleWords(u32 addr, isize len);
    const char *disassembleAddr(u32 addr);

    // Disassembles the currently executed instruction
    const char *disassembleInstr(isize *len);
    const char *disassembleWords(isize len);
    
    
    //
    // Changing state
    //
    
    // Continues program execution at the specified address
    void jump(u32 addr);
    
    
    //
    // Instruction delegates
    //

    void willExecute(const char *func, moira::Instr I, moira::Mode M, moira::Size S, u16 opcode);
    void didExecute(const char *func, moira::Instr I, moira::Mode M, moira::Size S, u16 opcode);
    void willExecute(moira::ExceptionType exc, u16 vector);
    void didExecute(moira::ExceptionType exc, u16 vector);

    
    //
    // Debugging
    //
    
    // Manages the breakpoint list
    void setBreakpoint(u32 addr) throws;
    void deleteBreakpoint(isize nr) throws;
    void enableBreakpoint(isize nr) throws;
    void disableBreakpoint(isize nr) throws;
    void ignoreBreakpoint(isize nr, isize count) throws;

    // Manages the watchpoint list
    void setWatchpoint(u32 addr) throws;
    void deleteWatchpoint(isize nr) throws;
    void enableWatchpoint(isize nr) throws;
    void disableWatchpoint(isize nr) throws;
    void ignoreWatchpoint(isize nr, isize count) throws;

    // Manages the catchpoint list
    void setCatchpoint(u8 vector) throws;
    void deleteCatchpoint(isize nr) throws;
    void enableCatchpoint(isize nr) throws;
    void disableCatchpoint(isize nr) throws;
    void ignoreCatchpoint(isize nr, isize count) throws;
};
