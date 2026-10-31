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
#include "Moira.h"
#include "utl/storage.h"
#include "utl/wrappers.h"

namespace vamiga {

class CPU : public moira::Moira
{
    friend class Moira;

    Descriptions descriptions = {{

        .type           = Class::CPU,
        .name           = "CPU",
        .description    = "Central Processing Unit",
        .shell          = "cpu"
    }};

    Options options = {

        Opt::CPU_REVISION,
        Opt::CPU_DASM_REVISION,
        Opt::CPU_DASM_SYNTAX,
        Opt::CPU_DASM_NUMBERS,
        Opt::CPU_OVERCLOCKING,
        Opt::CPU_RESET_VAL,
        Opt::CPU_START_DELAY
    };

    // The current configuration
    CPUConfig config = {};

    // The frame tracer's locking bit to turn instruction logging on and off
    static constexpr isize TRACE_SOURCE = 6;

public:

    // The current state
    utl::Backed<CPUInfo> info;

    // Breakpoints, Watchpoints, Catchpoints
    GuardList breakpoints = GuardList(amiga, debugger.breakpoints, GuardType::BREAKPOINT);
    GuardList watchpoints = GuardList(amiga, debugger.watchpoints, GuardType::WATCHPOINT);
    GuardList catchpoints = GuardList(amiga, debugger.catchpoints, GuardType::CATCHPOINT);

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

        // CLONE(instrStyle)
        // CLONE(dataStyle)
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

        CLONE_ARRAY(iCache.cache)
        CLONE(iCache.latch.addr)
        CLONE(iCache.latch.data)
        CLONE(budget.cp)
        CLONE(budget.accumulated)
        CLONE(budget.stalled)

        CLONE(irqMode)
        CLONE(ipl)
        CLONE(fcl)
        CLONE(fcSource)
        CLONE(exception)
        CLONE(loopModeDelay)
        CLONE(readBuffer)
        CLONE(writeBuffer)
        CLONE(flags)

        CLONE(config)

        if (cpuModel != other.cpuModel) {

            setModel((moira::Model)other.config.revision,
                     (moira::Model)other.config.dasmRevision);
        }

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
        << budget.cp
        << budget.accumulated
        << budget.stalled

        << irqMode
        << ipl
        << fcl
        << fcSource
        << exception
        << loopModeDelay
        << readBuffer
        << writeBuffer
        << flags;

        // Instruction cache
        worker << iCache.latch.addr << iCache.latch.data;
        for (isize i = 0; i < 64; ++i) {

            worker << iCache.cache[i].data;
            worker << iCache.cache[i].tag;
            worker << iCache.cache[i].valid;
        }
        
        if (isResetter(worker)) return;

        worker

        // Persistent items
        << config.revision
        << config.dasmRevision
        << config.overclocking
        << config.regResetVal
        << config.startDelay;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

private:

    void _dump(Category category, std::ostream &os) const override;


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
    // Analyzing
    //

public:

    CPUInfo cacheInfo() const;


    //
    // Methods from Configurable
    //

public:
    
    const CPUConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
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
    // Frame boundaries
    //

public:

    // Called by Agnus at the end of each frame
    void eofHandler();


    //
    // Running the disassembler
    //
    
    /* Disassembles an instruction according to a format string. The
     * following directives are supported:
     *
     *   %p  Instruction address
     *   %b  Instruction words
     *   %i  Instruction
     *   %f  Status register flags
     *   %Nd Data register DN (N = 0 ... 7)
     *   %Na Address register AN (N = 0 ... 7)
     *
     * A number in front of %p, %b, %i, or %f pads the output with spaces to
     * the given width. The register directives print the registers of the
     * CPU state the instruction has been recorded in (or the current CPU state
     * if the instruction is not taken from the log buffer). Throws CoreError
     * (OPT_INV_ARG) if the format string is invalid. If 'len' is given, it
     * receives the length of the instruction in bytes.
     */
    string disass(const string &fmt, u32 addr, isize *len = nullptr) const;
    string disassRecorded(const string &fmt, isize nr, isize *len = nullptr) const;

private:

    string disass(const string &fmt, u32 addr, const moira::Registers &regs, isize *len) const;

public:

    // Dumps a portion of the log buffer
    void dumpLogBuffer(std::ostream &os, isize count) const;
    void dumpLogBuffer(std::ostream &os) const;

    // Disassembles a memory range
    void disassembleRange(std::ostream &os, u32 addr, isize count) const;
    void disassembleRange(std::ostream &os, std::pair<u32, u32> range, isize max = 255) const;


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
    void willExecute(moira::M68kException exc, u16 vector);
    void didExecute(moira::M68kException exc, u16 vector);

};

}
