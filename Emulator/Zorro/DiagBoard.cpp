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

    // trace(ZOR_DEBUG, "peek8(%06x) = %02x\n", addr, result);
    trace(true, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
DiagBoard::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    // trace(ZOR_DEBUG, "peek16(%06x) = %04x\n", addr, result);
    trace(true, "peek16(%06x) = %04x\n", addr, result);
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
    
    switch (offset) {
            
        default:
            
            // Return Rom code
            return offset < rom.size ? HI_LO(rom[offset], rom[offset + 1]) : 0;
    }
}

void
DiagBoard::poke8(u32 addr, u8 value)
{
    // trace(ZOR_DEBUG, "poke8(%06x,%02x)\n", addr, value);
    trace(true, "poke8(%06x,%02x)\n", addr, value);
}

void
DiagBoard::poke16(u32 addr, u16 value)
{
    // trace(ZOR_DEBUG, "poke16(%06x,%04x)\n", addr, value);
    trace(true, "poke16(%06x,%04x)\n", addr, value);

    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();

    switch (offset) {
            
        default:

            warn("Invalid addr: %x\n", addr);
            break;
    }
}

void
DiagBoard::process()
{
    printf("process\n");
}
