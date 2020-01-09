// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <assert.h>

#include "Moira.h"

namespace moira {

#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExec_cpp.h"
#include "StrWriter_cpp.h"
#include "MoiraDasm_cpp.h"

Moira::Moira()
{
    createJumpTables();
}

void
Moira::reset()
{
    flags = 0;

    clock = -40; // REMOVE ASAP

    for(int i = 0; i < 8; i++) reg.d[i] = reg.a[i] = 0;
    reg.usp = 0;
    reg.ipl = 0;
    ipl = 0;
    
    sr.t = 0;
    sr.s = 1;
    sr.x = 0;
    sr.n = 0;
    sr.z = 0;
    sr.v = 0;
    sr.c = 0;
    sr.ipl = 7;

    sync(16);

    // Read the initial (supervisor) stack pointer from memory
    sync(2);
    reg.sp = read16OnReset(0);
    sync(4);
    reg.ssp = reg.sp = read16OnReset(2) | reg.sp << 16;
    sync(4);
    reg.pc = read16OnReset(4);
    sync(4);
    reg.pc = read16OnReset(6) | reg.pc << 16;

    // Fill the prefetch queue
    sync(4);
    queue.irc = read16OnReset(reg.pc & 0xFFFFFF);
    sync(2);
    prefetch();
}

void
Moira::execute()
{
    // Check for pending interrupts
    if (reg.ipl >= sr.ipl) {
        if (reg.ipl > sr.ipl || reg.ipl == 7) {

            assert(reg.ipl < 7);
            execIrqException(reg.ipl);
        }
    }

    // Check if the CPU is stopped or halted
    if (flags) {

        if (flags & FLAG_STOP) {
            pollIrq();
            sync(MIMIC_MUSASHI ? 1 : 2);
            return;
        }
    }

    reg.pc += 2;
    (this->*exec[queue.ird])(queue.ird);

    // Check if a breakpoint has been reached
    if (observer.breakpoints.elements() && observer.breakpoints.eval(reg.pc)) {
         breakpointReached(reg.pc);
     }

}

template<Size S> u32
Moira::readD(int n)
{
    return CLIP<S>(reg.d[n]);
}

template<Size S> u32
Moira::readA(int n)
{
    return CLIP<S>(reg.a[n]);
}

template<Size S> u32
Moira::readR(int n)
{
    return CLIP<S>(reg.r[n]);
}

template<Size S> void
Moira::writeD(int n, u32 v)
{
    reg.d[n] = WRITE<S>(reg.d[n], v);
}

template<Size S> void
Moira::writeA(int n, u32 v)
{
    reg.a[n] = WRITE<S>(reg.a[n], v);
}

template<Size S> void
Moira::writeR(int n, u32 v)
{
    reg.r[n] = WRITE<S>(reg.r[n], v);
}

u8
Moira::getCCR()
{
    return sr.c << 0 | sr.v << 1 | sr.z << 2 | sr.n << 3 | sr.x << 4;
}

void
Moira::setCCR(u8 val)
{
    sr.c = (val >> 0) & 1;
    sr.v = (val >> 1) & 1;
    sr.z = (val >> 2) & 1;
    sr.n = (val >> 3) & 1;
    sr.x = (val >> 4) & 1;
}

u16
Moira::getSR()
{
    return sr.t << 15 | sr.s << 13 | sr.ipl << 8 | getCCR();
}

void
Moira::setSR(u16 val)
{
    bool t = (val >> 15) & 1;
    bool s = (val >> 13) & 1;
    u8 ipl = (val >>  8) & 7;

    sr.ipl = ipl;
    sr.t = t;

    setCCR((u8)val);
    setSupervisorMode(s);
}

void
Moira::setSupervisorMode(bool enable)
{
    if (sr.s == enable) return;

    if (enable) {
        sr.s = 1;
        reg.usp = reg.a[7];
        reg.a[7] = reg.ssp;
    } else {
        sr.s = 0;
        reg.ssp = reg.a[7];
        reg.a[7] = reg.usp;
    }
}

int
Moira::getIrqVector(int level) {

    assert(level < 8);

    sync(4);

    switch (irqMode) {

        case IRQ_AUTO:          return 24 + level;
        case IRQ_USER:          return readIrqUserVector(level) & 0xFF;
        case IRQ_SPURIOUS:      return 24;
        case IRQ_UNINITIALIZED: return 15;
    }

    assert(false);
    return 0;
}

int
Moira::disassemble(u32 addr, char *str)
{
    u32 pc     = addr;
    u16 opcode = read16Dasm(pc);

    StrWriter writer(str, hex, upper);

    (this->*dasm[opcode])(writer, pc, opcode);
    writer << Finish{};

    return pc - addr + 2;
}

// Make sure the compiler generates certain instances of template functions
template u32 Moira::readD <Long> (int n);
template u32 Moira::readA <Long> (int n);
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}


