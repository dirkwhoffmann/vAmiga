// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CPUTypes.h"
#include "SubComponent.h"
#include "CmdQueue.h"
#include "GuardList.h"
#include "RingBuffer.h"
#include "Moira.h"

namespace vamiga {

class CPU : public moira::Moira, public Inspectable<CPUInfo>
{
    friend class Moira;

    Descriptions descriptions = {{

        .type           = Class::CPU,
        .name           = "CPU",
        .description    = "Central Processing Unit",
        .shell          = "cpu"
    }};

    ConfigOptions options = {

        Opt::CPU_REVISION,
        Opt::CPU_DASM_REVISION,
        Opt::CPU_DASM_SYNTAX,
        Opt::CPU_DASM_NUMBERS,
        Opt::CPU_OVERCLOCKING,
        Opt::CPU_RESET_VAL
    };

    // The current configuration
    CPUConfig config = {};

public:
    
    // Breakpoints, Watchpoints, Catchpoints
    GuardList breakpoints = GuardList(emulator, debugger.breakpoints);
    GuardList watchpoints = GuardList(emulator, debugger.watchpoints);
    GuardList catchpoints = GuardList(emulator, debugger.catchpoints);

    // Sub-cycle counter (overclocking)
    i64 debt;

    // Number of cycles that should be executed at normal speed (overclocking)
    i64 slowCycles;


    //
    // Initializing
    //

public:

    CPU(Amiga& ref);

    CPU& operator= (const CPU& other) {

        CLONE(debt)
        CLONE(slowCycles)

        CLONE(clock)
        CLONE(reg.pc)
        CLONE(reg.pc0)
        CLONE(reg.sr.t1)
        CLONE(reg.sr.t0)
        CLONE(reg.sr.s)
        CLONE(reg.sr.m)
        CLONE(reg.sr.x)
        CLONE(reg.sr.n)
        CLONE(reg.sr.z)
        CLONE(reg.sr.v)
        CLONE(reg.sr.c)
        CLONE(reg.sr.ipl)
        CLONE_ARRAY(reg.r)
        CLONE(reg.usp)
        CLONE(reg.isp)
        CLONE(reg.msp)
        CLONE(reg.ipl)
        CLONE(reg.vbr)
        CLONE(reg.sfc)
        CLONE(reg.dfc)
        CLONE(reg.cacr)
        CLONE(reg.caar)

        CLONE(queue.irc)
        CLONE(queue.ird)

        CLONE(ipl)
        CLONE(fcl)
        CLONE(fcSource)
        CLONE(exception)
        CLONE(cp)
        CLONE(loopModeDelay)
        CLONE(readBuffer)
        CLONE(writeBuffer)
        CLONE(flags)

        CLONE(config)

        return *this;
    }

    
    //
    // Methods from Serializable
    //
    
private:

    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        // Items from CPU class
        << debt
        << slowCycles

        // Items from Moira class
        << clock
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
        << cp
        << loopModeDelay
        << readBuffer
        << writeBuffer
        << flags;

        if (isResetter(worker)) return;

        worker

        // Persistent items
        << config.revision
        << config.dasmRevision
        << config.overclocking
        << config.regResetVal;

    } SERIALIZERS(serialize, override);


    //
    // Methods from CoreComponent
    //

private:

    void _dump(Category category, std::ostream& os) const override;


    void _didLoad() override;
    void _trackOn() override;
    void _trackOff() override;

public:

    void _didReset(bool hard) override;

    
    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }
    

    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(CPUInfo &result) const override;


    //
    // Methods from Configurable
    //

public:
    
    const CPUConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt opt) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt opt, i64 value) override;


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
    const char *disassembleRecordedInstr(isize i, isize *len) const;
    const char *disassembleRecordedWords(isize i, isize len) const;
    const char *disassembleRecordedFlags(isize i) const;
    const char *disassembleRecordedPC(isize i) const;

    // Disassembles the instruction at the specified address
    const char *disassembleWord(u16 value) const;
    const char *disassembleAddr(u32 addr) const;
    const char *disassembleInstr(u32 addr, isize *len) const;
    const char *disassembleWords(u32 addr, isize len) const;

    // Disassembles the currently executed instruction
    const char *disassembleInstr(isize *len) const;
    const char *disassembleWords(isize len) const;
    const char *disassemblePC() const;

    // Dumps a portion of the log buffer
    void dumpLogBuffer(std::ostream& os, isize count) const;
    void dumpLogBuffer(std::ostream& os) const;

    // Disassembles a memory range
    void disassembleRange(std::ostream& os, u32 addr, isize count) const;
    void disassembleRange(std::ostream& os, std::pair<u32, u32> range, isize max = 255) const;


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
    // Processing commands
    //

public:

    // Processes a command from the command queue
    void processCommand(const Command &cmd);
};

}
