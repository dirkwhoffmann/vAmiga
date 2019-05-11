/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ControlPort::ControlPort(int nr)
{
    assert(nr == 1 || nr == 2);
    
    this->nr = nr;
    setDescription(nr == 1 ? "ControlPort1" : "ControlPort2");
}

uint16_t
ControlPort::potgor()
{
    switch (device) {
            
        case CPD_MOUSE:
            return _amiga->mouse.rightButton ? 0xF7FF : 0xFFFF;
            
        default:
            return 0xFFFF;
    }
}

uint16_t
ControlPort::joydat()
{
    switch (device) {
            
        case CPD_MOUSE:
            return _amiga->mouse.getXY();
            
        default:
            return 0;
    }
}

uint8_t
ControlPort::ciapa()
{
    switch (device) {
            
        case CPD_MOUSE:
            if (_amiga->mouse.leftButton) {
                return (nr == 1) ? 0xBF : 0x7F;
            } else {
                return 0xFF;
            }
        default:
            return 0xFF;
    }
}

void
ControlPort::connectDevice(ControlPortDevice device)
{
    assert(isControlPortDevice(device));
    this->device = device;
}
