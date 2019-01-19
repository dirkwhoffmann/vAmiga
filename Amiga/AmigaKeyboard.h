// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _KEYBOARD_INC
#define _KEYBOARD_INC

#include "HardwareComponent.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class AmigaKeyboard : public HardwareComponent {
    
public:

    // Indicates if a key is currently held down (array index = raw key code).
    bool keyDown[128];
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    AmigaKeyboard();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
     void _powerOn() override;
     /*
     void _powerOff() override;
     void _reset() override;
     void _ping() override;
     */
     void _dump() override;
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    bool keyIsPressed(long keycode);
    void pressKey(long keycode);
    void releaseKey(long keycode);
    void releaseAllKeys();
};

#endif
