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
#include "Moira.h"

class CPU : public AmigaComponent, public moira::Moira {

    // Result of the latest inspection
    CPUInfo info;

public:

    //
    // Constructing and destructing
    //

    CPU(Amiga& ref);

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & flags
        & clock

        & reg.pc
        & reg.sr.t
        & reg.sr.s
        & reg.sr.x
        & reg.sr.n
        & reg.sr.z
        & reg.sr.v
        & reg.sr.c
        & reg.sr.ipl
        & reg.r
        & reg.usp
        & reg.ssp
        & reg.ipl

        & queue.irc
        & queue.ird

        & ipl
        & fcl;
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
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;
    size_t didSaveToBuffer(u8 *buffer) override;

public:
    
    // Returns the result of the most recent call to inspect()
    CPUInfo getInfo();
    DisassembledInstr getInstrInfo(long nr);
    DisassembledInstr getLoggedInstrInfo(long nr);

    //
    // Methods from Moira
    //

private:

    void sync(int cycles) override;
    u8 read8(u32 addr) override;
    u16 read16(u32 addr) override;
    u16 read16OnReset(u32 addr) override;
    u16 read16Dasm(u32 addr) override;
    void write8 (u32 addr, u8  val) override;
    void write16 (u32 addr, u16 val) override;
    int readIrqUserVector(u8 level) override { return 0; }
    void irqOccurred(u8 level) override;
    void breakpointReached(u32 addr) override;
    void watchpointReached(u32 addr) override;

    //
    // Working with the clock
    //

public:

    // Returns the clock in CPU cycles
    CPUCycle getCpuClock() { return getClock(); }

    // Returns the CPU clock measured in master cycles
    Cycle getMasterClock() { return CPU_CYCLES(getClock()); }

    // Delays the CPU by a certain number of cycles
    void addWaitStates(i64 cycles) { clock += cycles; }


    //
    // Using the disassembler
    //

    DisassembledInstr disassembleInstr(u32 addr);
};

#endif
