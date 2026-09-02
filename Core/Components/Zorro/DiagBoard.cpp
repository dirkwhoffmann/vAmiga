// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "DiagBoardTypes.h"
#include "DiagBoard.h"
#include "DiagBoardRom.h"
#include "Amiga.h"
#include <algorithm>

namespace vamiga {

void
DiagBoard::_dump(Category category, std::ostream &os) const
{
    ZorroBoard::_dump(category, os);
}

void
DiagBoard::_didReset(bool hard)
{    
    if (hard) {
        
        // Burn Expansion Rom
        rom.init(debug_exprom, DEBUG_EXPROM_SIZE);
        
        // Patch Kickstart Rom (1.2 only)
        mem.patchExpansionLib();

        // Set initial state
        state = pluggedIn() ? BoardState::AUTOCONF : BoardState::SHUTUP;
    }
}

i64
DiagBoard::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::DIAG_BOARD: return config.enabled;

        default:
            fatalError;
    }
}

void
DiagBoard::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::DIAG_BOARD:

            if (!isPoweredOff()) {
                throw CoreError(CoreError::OPT_LOCKED);
            }
            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
DiagBoard::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::DIAG_BOARD:

            config.enabled = value;
            return;
            
        default:
            fatalError;
    }
}

bool
DiagBoard::pluggedIn() const
{
    return config.enabled;
}

void
DiagBoard::updateMemSrcTables()
{
    // Only proceed if this board has been configured
    if (baseAddr == 0) return;
    
    // Map in this device
    mem.cpuMemSrc[firstPage()] = MemSrc::ZOR;
}

u8
DiagBoard::peek8(u32 addr)
{
    auto result = spypeek8(addr);

    logmsg(LOG_ZOR, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
DiagBoard::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    logmsg(LOG_ZOR, "peek16(%06x) = %04x\n", addr, result);
    return result;
}

u8
DiagBoard::spypeek8(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    return offset < rom.size ? rom[offset] : 0;
}

u16
DiagBoard::spypeek16(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    return offset < rom.size ? HI_LO(rom[offset], rom[offset + 1]) : 0;
}

void
DiagBoard::poke8(u32 addr, u8 value)
{
    logmsg(LOG_ZOR, "poke8(%06x,%02x)\n", addr, value);
}

void
DiagBoard::poke16(u32 addr, u16 value)
{
    logmsg(LOG_ZOR, "poke16(%06x,%04x)\n", addr, value);

    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();

    switch (offset) {
            
        case 0:
            
            pointer1 = REPLACE_HI_WORD(pointer1, value);
            break;
            
        case 2:

            pointer1 = REPLACE_LO_WORD(pointer1, value);
            break;

        case 4:

            pointer2 = REPLACE_HI_WORD(pointer2, value);
            break;

        case 6:

            pointer2 = REPLACE_LO_WORD(pointer2, value);
            break;

        case 16:
            
            switch (value) {
                    
                case 1: processInit(pointer1); break;
                case 2: processAddTask(pointer1); break;
                case 3: processRemTask(pointer1); break;
                case 4: processLoadSeg(pointer1, pointer2, false); break;
                case 5: processLoadSeg(pointer1, pointer2, true); break;
                    
                default:
                    logmsg(LOG_WARN, "Invalid value: %x\n", value);
                    break;
            }
            break;

        default:

            logmsg(LOG_WARN, "Invalid addr: %x\n", addr);
            break;
    }
}

void
DiagBoard::processInit(u32 ptr1)
{
    try {
        
        logmsg(LOG_DBD, "processInit\n");
        
        auto exec = osDebugger.getExecBase();
        tasks.push_back(exec.ThisTask);

    } catch (...) {

        logmsg(LOG_WARN, "processInit failed\n");
    }
}

void
DiagBoard::processAddTask(u32 ptr1)
{
    try {
        
        logmsg(LOG_DBD, "processAddTask\n");

        // Read task
        os::Task task;
        osDebugger.read(ptr1, &task);

        // Read task name
        string name;
        osDebugger.read(task.tc_Node.ln_Name, name);

        // Read task type
        auto type = task.tc_Node.ln_Type;
        if (type != os::NT_TASK && type != os::NT_PROCESS) {

            logmsg(LOG_WARN, "AddTask %x (%s): Wrong type: %d\n", ptr1, name.c_str(), type);
            return;
        }

        // Check if the task has already been added
        auto it = std::find(tasks.begin(), tasks.end(), ptr1);
        if (it != tasks.end()) {
            
            logmsg(LOG_WARN, "AddTask: %s '%s' already added\n",
                 type == os::NT_TASK ? "task" : "process", name.c_str());
            return;
        }

        // Add task
        tasks.push_back(ptr1);
        logmsg(LOG_DBD, "Added %s '%s'\n",
              type == os::NT_TASK ? "task" : "process", name.c_str());

    } catch (...) {

        logmsg(LOG_WARN, "processAddTask failed\n");
    }
}

void
DiagBoard::processRemTask(u32 ptr1)
{
    try {
        
        logmsg(LOG_DBD, "processRemTask\n");
        
        // Read task
        os::Task task;
        osDebugger.read(ptr1, &task);

        // Read task name
        string name;
        osDebugger.read(task.tc_Node.ln_Name, name);

        // Check if the task is under observation
        auto it = std::find(tasks.begin(), tasks.end(), ptr1);
        if (it == tasks.end()) {
            
            logmsg(LOG_WARN, "RemTask: '%s' (%x) not found\n", name.c_str(), ptr1);
            return;
        }

        // Remove task
        tasks.erase(it);
        logmsg(LOG_DBD, "Removed '%s'\n", name.c_str());
        
    } catch (...) {
        
        logmsg(LOG_WARN, "processRemTask failed\n");
    }
}

void
DiagBoard::processLoadSeg(u32 ptr1, u32 ptr2, bool bstr)
{
    try {
        
        logmsg(LOG_DBD, "processLoadSeg(%x,%x)\n", ptr1, ptr2);

        // Read task name
        string name;
        if (bstr) {
            auto length = (isize)mem.spypeek8 <Accessor::CPU> (4 * ptr1);
            logmsg(LOG_DBD, "Length = %ld\n", length);
            osDebugger.read(4 * ptr1 + 1, name, length);
        } else {
            osDebugger.read(ptr1, name);
        }
        logmsg(LOG_DBD, "LoadSeg: '%s' (%x)\n", name.c_str(), ptr2);
        
        auto it = std::find(targets.begin(), targets.end(), name);
        if (it != targets.end()) {

            targets.erase(it);
            auto addr = 4 * (ptr2 + 1);
            cpu.debugger.breakpoints.setAt(addr);
            logmsg(LOG_DBD, "Setting breakpoint at %x\n", addr);
        }

    } catch (...) {
        
        logmsg(LOG_WARN, "processLoadSeg failed\n");
    }
}

void
DiagBoard::catchTask(const string &name)
{
    if (!diagBoard.pluggedIn()) {
        throw CoreError(CoreError::OSDB, "Diagnose board is not plugged in.");
    }
    if (std::find(targets.begin(), targets.end(), name) == targets.end()) {
        targets.push_back(name);
    }
}

}
