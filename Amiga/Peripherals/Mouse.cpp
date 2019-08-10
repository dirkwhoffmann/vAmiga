// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Mouse::Mouse()
{
    setDescription("Mouse");
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
    plainmsg(" leftButton = %d\n", leftButton);
    plainmsg("rightButton = %d\n", rightButton);
    plainmsg("     mouseX = %lld\n", mouseX);
    plainmsg("     mouseY = %lld\n", mouseY);
    plainmsg("  oldMouseX = %lld\n", oldMouseX);
    plainmsg("  oldMouseY = %lld\n", oldMouseY);
    plainmsg("    targetX = %d\n", targetX);
    plainmsg("    targetY = %d\n", targetY);
    plainmsg("   dividerX = %d\n", dividerX);
    plainmsg("   dividerY = %d\n", dividerY);
    plainmsg("     shiftX = %d\n", shiftX);
    plainmsg("     shiftY = %d\n", shiftY);
}

int64_t
Mouse::getDeltaX()
{
    execute();

    int64_t result = mouseX - oldMouseX;
    oldMouseX = mouseX;

    return result;
}

int64_t
Mouse::getDeltaY()
{
    execute();

    int64_t result = mouseY - oldMouseY;
    oldMouseY = mouseY;

    return result;
}

uint16_t
Mouse::getXY()
{
    // Update mouseX and mouseY
    execute();
    
    // Assemble the result
    return HI_LO(mouseY & 0xFF, mouseX & 0xFF);
}

void
Mouse::setXY(int64_t x, int64_t y)
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
Mouse::execute()
{
    /*
    if (mouseX != targetX || mouseY != targetY) {
        debug("mouse = (%lld,%lld) target = (%lld,%lld)\n",
              mouseX, mouseY, targetX, targetY);
    }
    */
    
    /*
    // Jump directly to target coordinates if they are more than 8 shifts away.
    if (abs(targetX - mouseX) / 8 > shiftX) mouseX = targetX;
    if (abs(targetY - mouseY) / 8 > shiftY) mouseY = targetY;
    
    // Move mouse coordinates towards target coordinates
    if (targetX < mouseX) mouseX -= MIN(mouseX - targetX, shiftX);
    else if (targetX > mouseX) mouseX += MIN(targetX - mouseX, shiftX);
    if (targetY < mouseY) mouseY -= MIN(mouseY - targetY, shiftY);
    else if (targetY > mouseY) mouseY += MIN(targetY - mouseY, shiftY);
    */
    mouseX = targetX;
    mouseY = targetY;
}
