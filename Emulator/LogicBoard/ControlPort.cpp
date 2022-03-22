// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"

#include "ControlPort.h"
#include "IOUtils.h"
#include "Paula.h"

ControlPort::ControlPort(Amiga& ref, PortNr portNr) : SubComponent(ref), nr(portNr)
{
    assert_enum(PortNr, portNr);
    subComponents = std::vector<AmigaComponent *> { &mouse, &joystick };
}

const char *
ControlPort::getDescription() const
{
    return nr == PORT_1 ? "Port1" : "Port2";
}
    
void
ControlPort::_inspect() const
{
    {   SYNCHRONIZED
        
        info.joydat = joydat();
        
        // Extract pin values from joydat value
        bool x0 = GET_BIT(info.joydat, 0);
        bool x1 = GET_BIT(info.joydat, 1);
        bool y0 = GET_BIT(info.joydat, 8);
        bool y1 = GET_BIT(info.joydat, 9);
        info.m0v = y0 ^ !y1;
        info.m0h = x0 ^ !x1;
        info.m1v = !y1;
        info.m1h = !x1;
        
        info.potgo = paula.potgo;
        info.potgor = paula.peekPOTGOR();
        info.potdat = (nr == PORT_1) ? paula.peekPOTxDAT<0>() : paula.peekPOTxDAT<1>();
    }
}

void
ControlPort::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::State) {
        
        os << tab("Detected device type");
        os << ControlPortDeviceEnum::key(device);
        os << std::endl;
        os << tab("Mouse X counter");
        os << dec(mouseCounterX) << std::endl;
        os << tab("Mouse Y counter");
        os << dec(mouseCounterY) << std::endl;
    }
}

void
ControlPort::updateMouseXY(i64 x, i64 y)
{
    {   SYNCHRONIZED
        
        // Compute the delta movement
        i64 dx = x - mouseX;
        i64 dy = y - mouseY;
        
        // Store the mouse position
        mouseX = x;
        mouseY = y;
        
        // Update the mouse position counters
        mouseCounterX += dx;
        mouseCounterY += dy;
    }
}

u16
ControlPort::joydat() const
{
    // Compose the register bits
    u16 xxxxxx__xxxxxx__ = HI_LO(mouseCounterY & 0xFC, mouseCounterX & 0xFC);
    u16 ______xx______xx = 0;

    if (device == CPD_MOUSE) {
        ______xx______xx = HI_LO(mouseCounterY & 0x03, mouseCounterX & 0x03);
    }
    if (device == CPD_JOYSTICK) {
        ______xx______xx = joystick.joydat();
    }
    return xxxxxx__xxxxxx__ | ______xx______xx;
}

void
ControlPort::pokeJOYTEST(u16 value)
{
    {   SYNCHRONIZED
        
        mouseCounterY &= 0b00000011;
        mouseCounterY |= HI_BYTE(value) & 0b11111100;
        
        mouseCounterX &= 0b00000011;
        mouseCounterX |= LO_BYTE(value) & 0b11111100;
    }
}

void
ControlPort::changePotgo(u16 &potgo) const
{
    if (device == CPD_MOUSE) {
        mouse.changePotgo(potgo);
    }
}

void
ControlPort::changePra(u8 &pra) const
{
    if (device == CPD_MOUSE) {
        
        mouse.changePra(pra);
        return;
    }
    if (device == CPD_JOYSTICK) {
        
        joystick.changePra(pra);
        return;
    }
}
