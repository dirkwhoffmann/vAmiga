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
        
    switch (offset) {
            
        case 0x0: return type();
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
    
    trace(ACG_DEBUG, "peekAutoconf8(%06x) = %02x\n", offset, result);
    return result;
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
