// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RamExpansion.h"
#include "Memory.h"

void
RamExpansion::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
        
    ZorroBoard::_dump(category, os);
    
    if (category & dump::State) {
    
    }
}

void
RamExpansion::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {

        if (mem.fastRamSize() == 0) {
            
            // If no FastRam is present, disable the board
            state = STATE_SHUTUP;
        
        } else {
            
            // Setup the board descriptor
            descriptor.type         = 0xE0;
            descriptor.product      = 0x67;
            descriptor.flags        = 0x80;
            descriptor.manufacturer = 0x07B9;
            descriptor.serialNumber = 0x5041554C;
            
            switch (mem.fastRamSize()) {
                    
                case KB(64):  descriptor.type |= 0b001; break;
                case KB(128): descriptor.type |= 0b010; break;
                case KB(256): descriptor.type |= 0b011; break;
                case KB(512): descriptor.type |= 0b100; break;
                case MB(1):   descriptor.type |= 0b101; break;
                case MB(2):   descriptor.type |= 0b110; break;
                case MB(4):   descriptor.type |= 0b111; break;
                case MB(8):   descriptor.type |= 0b000; break;
                    
                default:
                    fatalError;
            }
        }
    }
}

void
RamExpansion::poke8(u32 addr, u8 value)
{
    trace(ACF_DEBUG, "poke8(%06x,%02x)\n", addr, value);
    
    switch (addr & 0xFFFF) {
                        
        case 0x48: // Base address (A23 - A20, 0x--X-0000)
            
            baseAddr |= (value & 0xF0) << 16;
            state = STATE_ACTIVE;

            trace(ACF_DEBUG, "FastRam mapped to $%06x\n", baseAddr);
            return;
            
        case 0x4A: // ec_BaseAddress (A19 - A16, 0x---X0000)
            
            baseAddr |= (value & 0xF0) << 12;
            return;
    }
}

const BoardDescriptor &
RamExpansion::getDescriptor() const
{
    return descriptor;
}
