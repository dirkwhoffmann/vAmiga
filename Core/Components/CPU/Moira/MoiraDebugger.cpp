// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "MoiraConfig.h"
#include "MoiraTypes.h"
#include "Moira.h"
#include "MoiraMacros.h"

#include <cassert>
#include <cstring>
#include <cstdio>

namespace MOIRA_NAMESPACE {


//
// Guard
//

bool
Guard::eval(u32 addr, Size S)
{
    if (this->addr >= addr && this->addr < addr + u32(S) && this->enabled) {

        if (!ignore) return true;
        ignore--;
    }
    return false;
}


//
// Guards
//

Guards::~Guards()
{
    assert(guards);
    delete [] guards;
}

Guard *
Guards::guardNr(isize nr) const
{
    return nr >= 0 && nr < count ? &guards[nr] : nullptr;
}

Guard *
Guards::guardAt(u32 addr) const
{
    for (int i = 0; i < count; i++) {
        if (guards[i].addr == addr) return &guards[i];
    }

    return nullptr;
}

std::optional<u32>
Guards::guardAddr(isize nr) const
{
    if (nr >= 0 && nr < count) return guards[nr].addr;
    return { };
}

void
Guards::setAt(u32 addr, isize ignores)
{
    if (isSetAt(addr)) return;

    if (count >= capacity) {

        Guard *newguards = new Guard[2 * capacity];
        for (isize i = 0; i < capacity; i++) newguards[i] = guards[i];
        delete [] guards;
        guards = newguards;
        capacity *= 2;
    }

    guards[count].addr = addr;
    guards[count].ignore = ignores;
    count++;

    setNeedsCheck(true);
}

void
Guards::remove(isize nr)
{
    if (nr >= 0 && nr < count) removeAt(guards[nr].addr);
}

void
Guards::removeAt(u32 addr)
{
    for (int i = 0; i < count; i++) {

        if (guards[i].addr == addr) {

            for (int j = i; j + 1 < count; j++) guards[j] = guards[j + 1];
            count--;
            break;
        }
    }
    setNeedsCheck(count != 0);
}

void
Guards::replace(isize nr, u32 addr)
{
    if (nr < 0 || nr >= count || isSetAt(addr)) return;

    guards[nr].addr = addr;
}

bool
Guards::isEnabled(isize nr) const
{
    Guard *guard = guardNr(nr);
    return guard != nullptr && guard->enabled;
}

bool
Guards::isEnabledAt(u32 addr) const
{
    Guard *guard = guardAt(addr);
    return guard != nullptr && guard->enabled;
}

bool
Guards::isDisabled(isize nr) const
{
    Guard *guard = guardNr(nr);
    return guard != nullptr && !guard->enabled;
}

bool
Guards::isDisabledAt(u32 addr) const
{
    Guard *guard = guardAt(addr);
    return guard != nullptr && !guard->enabled;
}

void
Guards::setEnable(isize nr, bool val)
{
    Guard *guard = guardNr(nr);
    if (guard) guard->enabled = val;
}

void
Guards::setEnableAt(u32 addr, bool val)
{
    Guard *guard = guardAt(addr);
    if (guard) guard->enabled = val;
}

void 
Guards::setEnableAll(bool val)
{
    for (int i = 0; i < count; i++) guards[i].enabled = val;
}

void
Guards::ignore(isize nr, isize count)
{
    Guard *guard = guardNr(nr);
    if (guard) guard->ignore = count;
}

bool
Guards::eval(u32 addr, Size S)
{
    for (int i = 0; i < count; i++) {

        if (guards[i].eval(addr, S)) {

            hit = guards[i];
            return true;
        }
    }
    return false;
}

void
Breakpoints::setNeedsCheck(bool value)
{
    if (value) {
        moira.flags |= State::CHECK_BP;
    } else {
        moira.flags &= ~State::CHECK_BP;
    }
}

void
Watchpoints::setNeedsCheck(bool value)
{
    if (value) {
        moira.flags |= State::CHECK_WP;
    } else {
        moira.flags &= ~State::CHECK_WP;
    }
}

void
Catchpoints::setNeedsCheck(bool value)
{
    if (value) {
        moira.flags |= State::CHECK_CP;
    } else {
        moira.flags &= ~State::CHECK_CP;
    }
}

u16
SoftwareTraps::create(u16 instr)
{
    // Seek an unsed LINE-A instruction
    u16 key = 0xA000;
    while (traps.contains(key)) key++;

    return create(key, instr);
}

u16
SoftwareTraps::create(u16 key, u16 instr)
{
    assert(Debugger::isLineAInstr(key));
    assert(traps.size() < 512);
    assert(!traps.contains(key));

    traps[key] = SoftwareTrap { .instruction = instr };
    return key;
}

u16
SoftwareTraps::resolve(u16 instr) const
{
    return traps.contains(instr) ? traps.at(instr).instruction : instr;
}

void
Debugger::reset()
{
    breakpoints.setNeedsCheck(breakpoints.elements() != 0);
    watchpoints.setNeedsCheck(watchpoints.elements() != 0);
}

void
Debugger::stepInto()
{
    softStop = -1;
    breakpoints.setNeedsCheck(true);
}

void
Debugger::stepOver()
{
    u32 pc = moira.getPC();
    softStop = pc + moira.instrLength(pc);
    breakpoints.setNeedsCheck(true);
}

bool
Debugger::softstopMatches(u32 addr)
{
    if (softStop && (*softStop < 0 || *softStop == addr)) {

        // Soft breakpoints are deleted when reached
        softStop = { };
        breakpoints.setNeedsCheck(breakpoints.elements() != 0);
        return true;
    }
    return false;
}

bool
Debugger::breakpointMatches(u32 addr)
{
    return breakpoints.eval(addr);
}

bool
Debugger::watchpointMatches(u32 addr, Size S)
{
    return watchpoints.eval(addr, S);
}

bool
Debugger::catchpointMatches(u32 vectorNr)
{
    return catchpoints.eval(vectorNr);
}

void
Debugger::enableLogging()
{
    moira.flags |= State::LOGGING;
}

void
Debugger::disableLogging()
{
    moira.flags &= ~State::LOGGING;
}

isize
Debugger::loggedInstructions() const
{
    return logCnt < logBufferCapacity ? logCnt : logBufferCapacity;
}

void
Debugger::logInstruction()
{
    logBuffer[logCnt % logBufferCapacity] = moira.reg;
    logCnt++;

    moira.didLogInstruction(moira.reg);
}

const Registers &
Debugger::logEntryRel(isize n) const
{
    assert(n < loggedInstructions());
    return logBuffer[(logCnt - 1 - n) % logBufferCapacity];
}

const Registers &
Debugger::logEntryAbs(isize n) const
{
    assert(n < loggedInstructions());
    return logEntryRel(loggedInstructions() - n - 1);
}

std::string
Debugger::disassembleRecorded(const std::string &fmt, isize nr, int *len) const
{
    auto &entry = logEntryAbs(nr);
    return moira.disassemble(fmt, entry.pc0, entry, len);
}

void
Debugger::dumpLogBuffer(std::ostream &os, isize count, const std::string &fmt) const
{
    isize num = loggedInstructions();

    for (isize i = num - count; i < num ; i++) {

        if (i >= 0) os << "   " << disassembleRecorded(fmt, i) << std::endl;
    }
}

void
Debugger::dumpLogBuffer(std::ostream &os, const std::string &fmt) const
{
    dumpLogBuffer(os, loggedInstructions(), fmt);
}

void
Debugger::disassembleRange(std::ostream &os, u32 addr, isize count, const std::string &fmt) const
{
    disassembleRange(os, std::pair<u32, u32>(addr, UINT32_MAX), count, fmt);
}

void
Debugger::disassembleRange(std::ostream &os, std::pair<u32, u32> range, isize max, const std::string &fmt) const
{
    u32 addr = range.first;
    int numBytes = 0;
    auto pc = moira.getPC0();

    for (isize i = 0; i < max && addr <= range.second; i++, addr += u32(numBytes)) {

        auto line = moira.disassemble(fmt, addr, &numBytes);

        os << (addr == pc ? "->" : "  ");

        if (breakpoints.isDisabledAt(addr)) {
            os << "b";
        } else if (breakpoints.isSetAt(addr)) {
            os << "B";
        } else {
            os << " ";
        }

        os << line << std::endl;
    }
}

std::string
Debugger::vectorName(u8 vectorNr)
{
    if (vectorNr >= 12 && vectorNr <= 14) {
        return "Reserved";
    }
    if (vectorNr >= 16 && vectorNr <= 23) {
        return "Reserved";
    }
    if (vectorNr >= 48 && vectorNr <= 63) {
        return "Reserved";
    }
    if (vectorNr >= 25 && vectorNr <= 31) {
        return "Level " + std::to_string(vectorNr - 24) + " interrupt";
    }
    if (vectorNr >= 32 && vectorNr <= 47) {
        return "Trap #" + std::to_string(vectorNr - 32);
    }
    if (vectorNr >= 64) {
        return "User interrupt vector";
    }
    switch (vectorNr) {

        case 0:     return "Reset SP";
        case 1:     return "Reset PC";
        case 2:     return "Bus error";
        case 3:     return "Address error";
        case 4:     return "Illegal instruction";
        case 5:     return "Division by zero";
        case 6:     return "CHK instruction";
        case 7:     return "TRAPV instruction";
        case 8:     return "Privilege violation";
        case 9:     return "Trace";
        case 10:    return "Line A instruction";
        case 11:    return "Line F instruction";
        case 15:    return "Uninitialized IRQ vector";
        case 24:    return "Spurious interrupt";

        default:
            fatalError;
    }
}

void
Debugger::jump(u32 addr)
{
    moira.jump(addr);
}

}
