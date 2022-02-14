// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ZorroBoard.h"
#include "IOUtils.h"
#include "Memory.h"

void
ZorroBoard::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
            
    if (category & dump::State) {
    
        os << tab("Type");
        os << hex(type());
        os << tab("Product");
        os << hex(product());
        os << tab("Flags");
        os << hex(flags());
        os << tab("Manufacturer");
        os << hex(manufacturer());
        os << tab("Serial number");
        os << hex(serialNumber());
    }
}

u8
ZorroBoard::getDescriptorByte(isize offset) const
{
    assert((usize)offset <= 15);
        
    auto sizeBits = [&]() {
        
        switch (pages()) {
                
            case 0x01: return (u8)0b001;
            case 0x02: return (u8)0b010;
            case 0x04: return (u8)0b011;
            case 0x08: return (u8)0b100;
            case 0x10: return (u8)0b101;
            case 0x20: return (u8)0b110;
            case 0x40: return (u8)0b111;
            case 0x80: return (u8)0b000;
                
            default:
                fatalError;
        }
    };
    
    switch (offset) {
            
        case 0x0: return type() | sizeBits();
        case 0x1: return product();
        case 0x2: return flags();
        case 0x3: return 0;
        case 0x4: return BYTE1(manufacturer());
        case 0x5: return BYTE0(manufacturer());
        case 0x6: return BYTE3(serialNumber());
        case 0x7: return BYTE2(serialNumber());
        case 0x8: return BYTE1(serialNumber());
        case 0x9: return BYTE0(serialNumber());
        case 0xA: return BYTE1(initDiagVec());
        case 0xB: return BYTE0(initDiagVec());

        default:
            return 0;
    }
}

u8
ZorroBoard::peekAutoconf8(u32 addr) const
{
    u8 result = 0xFF;
    u8 offset = addr & 0xFF;
            
    if (IS_EVEN(offset) && offset < 0x40) {

        result = getDescriptorByte(offset >> 2);

        result = (offset & 2) ? LO_NIBBLE(result) : HI_NIBBLE(result);
        result = (offset < 4) ? (u8)(result << 4) : (u8)~(result << 4);
    
    } else if (offset == 0x40 || offset == 0x42) {
        
        // Interrupt pending register
        result = 0x00;
    }
    
    trace(ZOR_DEBUG, "peekAutoconf8(%06x) = %02x\n", offset, result);
    return result;
}

void
ZorroBoard::pokeAutoconf8(u32 addr, u8 value)
{
    trace(ZOR_DEBUG, "pokeAutoconf8(%06x,%02x)\n", addr, value);
    
    switch (addr & 0xFFFF) {
                        
        case 0x48: // Base address (A23 - A20, 0x--X-0000)
            
            baseAddr |= (value & 0xF0) << 16;

            // Activate the board
            state = STATE_ACTIVE;
            
            // Update the memory map
            mem.updateMemSrcTables();

            trace(ZOR_DEBUG, "Device mapped to $%06x\n", baseAddr);
            return;
            
        case 0x4A: // ec_BaseAddress (A19 - A16, 0x---X0000)
            
            baseAddr |= (value & 0xF0) << 12;
            return;
    }
}

void
ZorroBoard::activate()
{
    state = STATE_ACTIVE;
}

void
ZorroBoard::shutup()
{
    state = STATE_SHUTUP;
}