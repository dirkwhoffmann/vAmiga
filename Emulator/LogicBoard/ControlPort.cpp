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

namespace vamiga {

ControlPort::ControlPort(Amiga& ref, isize nr) : SubComponent(ref), nr(nr)
{
    assert(nr == PORT1 || nr == PORT2);

    subComponents = std::vector<AmigaComponent *> { &mouse, &joystick };
}

const char *
ControlPort::getDescription() const
{
    return nr == PORT1 ? "Port1" : "Port2";
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
        info.potdat = (nr == PORT1) ? paula.peekPOTxDAT<0>() : paula.peekPOTxDAT<1>();
    }
}

void
ControlPort::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Inspection) {
        
        os << tab("Control port");
        os << dec(nr) << std::endl;
        os << tab("Detected device type");
        os << ControlPortDeviceEnum::key(device) << std::endl;
    }

    if (category == Category::Debug) {

        os << tab("Control port");
        os << dec(nr) << std::endl;

        if (nr == PORT1) {

            os << tab("potCntX0") << dec(paula.potCntX0) << std::endl;
            os << tab("potCntY0") << dec(paula.potCntY0) << std::endl;
            os << tab("chargeX0") << flt(paula.chargeX0) << std::endl;
            os << tab("chargeY0") << flt(paula.chargeY0) << std::endl;
        }
        if (nr == PORT2) {

            os << tab("potCntX1") << dec(paula.potCntX1) << std::endl;
            os << tab("potCntY1") << dec(paula.potCntY1) << std::endl;
            os << tab("chargeX1") << flt(paula.chargeX1) << std::endl;
            os << tab("chargeY1") << flt(paula.chargeY1) << std::endl;
        }
        os << tab("Charge DX");
        os << flt(chargeDX) << std::endl;
        os << tab("Charge DY");
        os << flt(chargeDY) << std::endl;

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
    switch (device) {

        case CPD_MOUSE:     mouse.changePotgo(potgo); break;
        case CPD_JOYSTICK:  joystick.changePotgo(potgo); break;

        default:
            break;
    }
}

void
ControlPort::changePra(u8 &pra) const
{
    switch (device) {

        case CPD_MOUSE:     mouse.changePra(pra); break;
        case CPD_JOYSTICK:  joystick.changePra(pra); break;

        default:
            break;
    }
}

}
