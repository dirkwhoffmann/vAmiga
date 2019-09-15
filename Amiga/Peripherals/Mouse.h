// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MOUSE_INC
#define _MOUSE_INC

#include "SubComponent.h"

class Mouse : public SubComponent {
    
public:
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
private:
    
    // The current mouse position
    int64_t mouseX;
    int64_t mouseY;

    // Recorded mouse position in getDeltaX() and getDeltaY()
    int64_t oldMouseX;
    int64_t oldMouseY;

    /* The target mouse position
     * In order to achieve a smooth mouse movement, a new mouse coordinate is
     * not written directly into mouseX and mouseY. Instead, these variables
     * are set. In execute(), mouseX and mouseY are shifted smoothly towards
     * the target positions.
     */
    int64_t targetX;
    int64_t targetY;
    
    // Dividers applied to raw coordinates in setXY()
    int dividerX = 128;
    int dividerY = 128;
    
    // Mouse movement in pixels per execution step
    int64_t shiftX = 31;
    int64_t shiftY = 31;


    //
    // Constructing and destructing
    //
    
public:
    
    Mouse(Amiga& ref);

    
    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }


    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _reset() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Operating the mouse
    //
    
public:
    
    // Returns a horizontal or vertical position change
    int64_t getDeltaX();
    int64_t getDeltaY();

    // Returns the mouse coordinates as they appear in the JOYDAT register.
    uint16_t getXY();
    
    // Emulates a mouse movement event.
    void setXY(int64_t x, int64_t y);
    
    // Emulates a mouse button event.
    void setLeftButton(bool value);
    void setRightButton(bool value);
    
    // Performs periodic actions for this device.
    void execute();
};

#endif

