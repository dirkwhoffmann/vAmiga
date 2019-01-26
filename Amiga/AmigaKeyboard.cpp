// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AmigaKeyboard::AmigaKeyboard()
{
    setDescription("AmigaKeyboard");
}

void
AmigaKeyboard::_powerOn()
{
    memset(keyDown, 0, sizeof(keyDown));
}

void
AmigaKeyboard::_dump()
{
    for (unsigned i = 0; i < 128; i++) {
        if (keyIsPressed(i)) {
            msg("Key %02X is pressed.\n");
        }
    }
}

void
AmigaKeyboard::setMapCmdKeys(bool value)
{
    if (value != mapCmdKeys) {
        
        mapCmdKeys = value;
        amiga->putMessage(value ? MSG_MAP_CMD_KEYS : MSG_UNMAP_CMD_KEYS);
    }
}

bool
AmigaKeyboard::keyIsPressed(long keycode)
{
    assert(keycode < 0x80);
    return keyDown[keycode];
}

void
AmigaKeyboard::pressKey(long keycode)
{
    assert(keycode < 0x80);
    
    // Check if the left or the right Command key (the 'Amiga' key) is pressed
    if (keycode == 0x66 || keycode == 0x67) {
        if (!mapCmdKeys) return;
    }
    
    if (!keyDown[keycode]) {
        debug("Pressing Amiga key %02X\n", keycode);
    }
    keyDown[keycode] = true;
}

void
AmigaKeyboard::releaseKey(long keycode)
{
    assert(keycode < 0x80);

    if (keyDown[keycode]) {
        debug("Releasing Amiga key %02X\n", keycode);
    }
    keyDown[keycode] = false;
}

void
AmigaKeyboard::releaseAllKeys()
{
    for (unsigned i = 0; i < 0x80; i++) {
        releaseKey(i);
    }
}
