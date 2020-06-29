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

#include "AmigaComponent.h"

class Mouse : public AmigaComponent {
    
    // The control port this device is connected to
    const PortNr nr;

    // Current configuration
    MouseConfig config;

public:
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
private:
    
    // The current mouse position
    i64 mouseX;
    i64 mouseY;

    // Recorded mouse position in getDeltaX() and getDeltaY()
    i64 oldMouseX;
    i64 oldMouseY;

    /* The target mouse position
     * In order to achieve a smooth mouse movement, a new mouse coordinate is
     * not written directly into mouseX and mouseY. Instead, these variables
     * are set. In execute(), mouseX and mouseY are shifted smoothly towards
     * the target positions.
     */
    i64 targetX;
    i64 targetY;
    
    // Dividers applied to raw coordinates in setXY()
    int dividerX = 128;
    int dividerY = 128;
    
    // Mouse movement in pixels per execution step
    i64 shiftX = 31;
    i64 shiftY = 31;


    //
    // Constructing and serializing
    //
    
public:
    
    Mouse(PortNr n, Amiga& ref);

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker & config.pullUpResistors;
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
    void _reset(bool hard) override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Managing registers
    //

public:

    // Modifies the POTGOR bits according to the current button state
    void changePotgo(u16 &potgo);

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra);

    
    //
    // Operating the mouse
    //
    
public:
    
    // Returns a horizontal or vertical position change
    i64 getDeltaX();
    i64 getDeltaY();

    // Returns the mouse coordinates as they appear in the JOYDAT register
    u16 getXY();
    
    // Emulates a mouse movement
    void setXY(i64 x, i64 y);
    
    // Presses or releases a mouse button
    void setLeftButton(bool value);
    void setRightButton(bool value);

    // Triggers a gamepad event
    void trigger(GamePadAction event);

    // Performs periodic actions for this device
    void execute();
};

#endif

