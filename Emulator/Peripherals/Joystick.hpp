// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.hpp"

namespace va {

#include "Joystick.h"

//
// Reflection APIs
//

struct GamePadActionEnum : Reflection<GamePadActionEnum, GamePadAction> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= RELEASE_RIGHT;
    }
    
    static const char *prefix() { return nullptr; }
    static const char *key(GamePadAction value)
    {
        switch (value) {
                
            case PULL_UP:        return "PULL_UP";
            case PULL_DOWN:      return "PULL_DOWN";
            case PULL_LEFT:      return "PULL_LEFT";
            case PULL_RIGHT:     return "PULL_RIGHT";
            case PRESS_FIRE:     return "PRESS_FIRE";
            case PRESS_LEFT:     return "PRESS_LEFT";
            case PRESS_RIGHT:    return "PRESS_RIGHT";
            case RELEASE_X:      return "RELEASE_X";
            case RELEASE_Y:      return "RELEASE_Y";
            case RELEASE_XY:     return "RELEASE_XY";
            case RELEASE_FIRE:   return "RELEASE_FIRE";
            case RELEASE_LEFT:   return "RELEASE_LEFT";
            case RELEASE_RIGHT:  return "RELEASE_RIGHT";
        }
        return "???";
    }
};

class Joystick : public AmigaComponent {

    // Reference to control port this device belongs to
    ControlPort &port;

    // Button state
    bool button = false;
    
    // Horizontal joystick position (-1 = left, 1 = right, 0 = released)
    int axisX = 0;
    
    // Vertical joystick position (-1 = up, 1 = down, 0 = released)
    int axisY = 0;
    
    // Indicates whether multi-shot mode is enabled
    bool autofire = false;
    
    // Number of bullets per gun volley
    int autofireBullets = -3;
    
    // Autofire frequency in Hz
    float autofireFrequency = 2.5;
    
    // Bullet counter used in multi-fire mode
    i64 bulletCounter = 0;
    
    // Next frame to auto-press or auto-release the fire button
    i64 nextAutofireFrame = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    Joystick(Amiga& ref, ControlPort& pref) : AmigaComponent(ref), port(pref) { };

    const char *getDescription() const override;
    
private:
    
    void _reset(bool hard) override;

    
    //
    // Analyzing
    //
    
private:
    
    void _dump(Dump::Category category, std::ostream& os) const override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
    
    //
    // Accessing
    //
    
public:
    
    // Configures autofire mode
    bool getAutofire() const { return autofire; }
    void setAutofire(bool value);
    
    // Configures the bullets per gun volley (negative value = infinite)
    int getAutofireBullets() const { return autofireBullets; }
    void setAutofireBullets(int value);
    
    // Configures the autofire frequency
    float getAutofireFrequency() const { return autofireFrequency; }
    void setAutofireFrequency(float value) { autofireFrequency = value; }

private:
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();

public:

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra) const;


    //
    // Using the device
    //
    
public:

    // Callback handler for function ControlPort::joydat()
    u16 joydat() const;

    // Callback handler for function ControlPort::ciapa()
    u8 ciapa() const;
    
    // Triggers a gamepad event
    void trigger(GamePadAction event);

    /* Execution function for this control port. This method needs to be
     * invoked at the end of each frame to make the auto-fire mechanism work.
     */
    void execute();
};

}
