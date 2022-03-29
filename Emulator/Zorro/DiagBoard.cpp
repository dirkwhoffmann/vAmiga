// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiagBoard.h"
#include "DiagBoardRom.h"
#include "Memory.h"
#include "OSDebugger.h"

DiagBoard::DiagBoard(Amiga& ref) : ZorroBoard(ref)
{

}

void
DiagBoard::_dump(Category category, std::ostream& os) const
{
    using namespace util;
        
    ZorroBoard::_dump(category, os);
}

void
DiagBoard::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    if (hard) {
        
        // Burn Rom
        rom.init(debug_exprom, DEBUG_EXPROM_SIZE);
        
        // Set initial state
        state = pluggedIn() ? STATE_AUTOCONF : STATE_SHUTUP;
    }
}

bool
DiagBoard::pluggedIn() const
{
    return bool(DIAG_BOARD);
}

void
DiagBoard::updateMemSrcTables()
{
    // Only proceed if this board has been configured
    if (baseAddr == 0) return;
    
    // Map in this device
    mem.cpuMemSrc[firstPage()] = MEM_ZOR;
}

u8
DiagBoard::peek8(u32 addr)
{
    auto result = spypeek8(addr);

    trace(ZOR_DEBUG, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
DiagBoard::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    trace(ZOR_DEBUG, "peek16(%06x) = %04x\n", addr, result);
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
    trace(ZOR_DEBUG, "poke8(%06x,%02x)\n", addr, value);
}

void
DiagBoard::poke16(u32 addr, u16 value)
{
    trace(ZOR_DEBUG, "poke16(%06x,%04x)\n", addr, value);

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
                case 4: processLoadSeg(pointer1, pointer2); break;
                    
                default:
                    warn("Invalid value: %x\n", value);
                    break;
            }
            break;

        default:

            warn("Invalid addr: %x\n", addr);
            break;
    }
}

void
DiagBoard::processInit(u32 ptr1)
{
    try {
        
        debug(ZOR_DEBUG, "processInit\n");
        
        auto exec = osDebugger.getExecBase();
        tasks.push_back(exec.ThisTask);

    } catch (...) {

        warn("processInit failed\n");
    }
}

void
DiagBoard::processAddTask(u32 ptr1)
{
    try {
        
        debug(ZOR_DEBUG, "processAddTask\n");

        // Read task
        os::Task task;
        osDebugger.read(ptr1, &task);

        // Check if the task has already been added
        auto it = std::find(tasks.begin(), tasks.end(), ptr1);
        if (it != tasks.end()) {
            
            warn("AddTask: Already added: %x\n", ptr1);
            return;
        }

        // Read task name
        string name;
        osDebugger.read(task.tc_Node.ln_Name, name);

        // Read task type
        auto type = task.tc_Node.ln_Type;
        if (type != os::NT_TASK && type != os::NT_PROCESS) {

            warn("AddTask %x (%s): Wrong type: %d\n", ptr1, name.c_str(), type);
            return;
        }
        const char* typeName = (type == os::NT_TASK) ? "Task" : "Process";

        // Add task
        tasks.push_back(ptr1);
        debug(true, "Added %s '%s'\n", typeName, name.c_str());

        // if (type == NT_PROCESS) check_wait_process(task_ptr, task_name);
    
    } catch (...) {

        warn("processAddTask failed\n");
    }
}

void
DiagBoard::processRemTask(u32 ptr1)
{
    try {
        
        debug(ZOR_DEBUG, "processRemTask\n");
        
        // Read task
        os::Task task;
        osDebugger.read(ptr1, &task);

        // Read task name
        string name;
        osDebugger.read(task.tc_Node.ln_Name, name);

        // Check if the task is under observation
        auto it = std::find(tasks.begin(), tasks.end(), ptr1);
        if (it == tasks.end()) {
            
            warn("RemTask: '%s' (%x) not found\n", name.c_str(), ptr1);
            return;
        }
                        
        // Remove task
        tasks.erase(it);
        debug(true, "Removed '%s'\n", name.c_str());
        
    } catch (...) {
        
        warn("processRemTask failed\n");
    }
}

void
DiagBoard::processLoadSeg(u32 ptr1, u32 ptr2)
{
    try {
        
        debug(ZOR_DEBUG, "processLoadSeg(%x,%x)\n", ptr1, ptr2);
                
        // Read task name
        string name;
        osDebugger.read(ptr1, name);
 
        debug(true, "LoadSeg: '%s' (%x)\n", name.c_str(), ptr2);
        
    } catch (...) {
        
        warn("processLoadSeg failed\n");
    }
}
