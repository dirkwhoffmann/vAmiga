// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Mouse::Mouse(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Mouse");

    config.pullUpResistors = true;
}

void
Mouse::_powerOn()
{
}

void Mouse::_reset()
{
    RESET_SNAPSHOT_ITEMS

    leftButton = false;
    rightButton = false;
    mouseX = 0;
    mouseY = 0;
    oldMouseX = 0;
    oldMouseY = 0;
    targetX = 0;
    targetY = 0;
}

void
Mouse::_dump()
{
    msg(" leftButton = %d\n", leftButton);
    msg("rightButton = %d\n", rightButton);
    msg("     mouseX = %lld\n", mouseX);
    msg("     mouseY = %lld\n", mouseY);
    msg("  oldMouseX = %lld\n", oldMouseX);
    msg("  oldMouseY = %lld\n", oldMouseY);
    msg("    targetX = %d\n", targetX);
    msg("    targetY = %d\n", targetY);
    msg("   dividerX = %d\n", dividerX);
    msg("   dividerY = %d\n", dividerY);
    msg("     shiftX = %d\n", shiftX);
    msg("     shiftY = %d\n", shiftY);
}

void
Mouse::changePotgo(int port, u16 &potgo)
{
    u16 mask = (port == 1) ? 0x0400 : 0x4000;

    if (rightButton) {
        potgo &= ~mask;
    } else if (config.pullUpResistors) {
        potgo |= mask;
    }
}

void
Mouse::changePra(int port, u8 &pra)
{
    u16 mask = (port == 1) ? 0x40 : 0x80;

    if (leftButton) {
        pra &= ~mask;
    } else if (config.pullUpResistors) {
        pra |= mask;
    }
}

i64
Mouse::getDeltaX()
{
    execute();

    i64 result = mouseX - oldMouseX;
    oldMouseX = mouseX;

    return result;
}

i64
Mouse::getDeltaY()
{
    execute();

    i64 result = mouseY - oldMouseY;
    oldMouseY = mouseY;

    return result;
}

u16
Mouse::getXY()
{
    // Update mouseX and mouseY
    execute();
    
    // Assemble the result
    return HI_LO(mouseY & 0xFF, mouseX & 0xFF);
}

void
Mouse::setXY(i64 x, i64 y)
{
    // debug("setXY(%lld,%lld)\n", x, y);
    
    targetX = x / dividerX;
    targetY = y / dividerY;

    // debug("targetX = %lld targetY = %lld\n", targetX, targetY);
}

void
Mouse::setLeftButton(bool value)
{
    debug(PORT_DEBUG, "setLeftButton(%d)\n", value);
    leftButton = value;
}

void
Mouse::setRightButton(bool value)
{
    debug(PORT_DEBUG, "setRightButton(%d)\n", value);
    rightButton = value;
}

void
Mouse::trigger(GamePadAction event)
{
    assert(isGamePadAction(event));

    debug(PORT_DEBUG, "trigger(%d)\n", event);

    switch (event) {

        case PRESS_LEFT: setLeftButton(true); break;
        case RELEASE_LEFT: setLeftButton(false); break;
        case PRESS_RIGHT: setRightButton(true); break;
        case RELEASE_RIGHT: setRightButton(false); break;
        default: break;
    }
}

void
Mouse::execute()
{
    mouseX = targetX;
    mouseY = targetY;
}
