// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MouseTypes.h"
#include "Joystick.h"
#include "SubComponent.h"
#include "Chrono.h"

class ShakeDetector {
    
    // Horizontal position
    double x = 0.0;
    
    // Moved distance
    double dxsum = 0.0;

    // Direction (1 or -1)
    double dxsign = 1.0;
    
    // Number of turns
    isize dxturns = 0;
    
    // Time stamps
    u64 lastTurn = 0;
    util::Time lastShake;
    
public:
    
    // Feeds in new coordinates and checks for a shake
    bool isShakingAbs(double x);
    bool isShakingRel(double dx);
};

class Mouse : public SubComponent {

    // Reference to the control port this device belongs to
    ControlPort &port;
    
    // Current configuration
    MouseConfig config = {};

    // Shake detector
    class ShakeDetector shakeDetector;
    
public:
    
    // Mouse button states
    bool leftButton = false;
    bool rightButton = false;
    
private:
    
    // The current mouse position
    double mouseX = 0.0;
    double mouseY = 0.0;

    // Recorded mouse position in getDeltaX() and getDeltaY()
    double oldMouseX = 0.0;
    double oldMouseY = 0.0;

    /* The target mouse position. In order to achieve a smooth mouse movement,
     * a new mouse coordinate is not written directly into mouseX and mouseY.
     * Instead, these variables are set. In execute(), mouseX and mouseY are
     * shifted smoothly towards the target positions.
     */
    double targetX = 0.0;
    double targetY = 0.0;
    
    // Scaling factors applied to the raw mouse coordinates in setXY()
    double scaleX = 1.0;
    double scaleY = 1.0;
    
    // Mouse movement in pixels per execution step
    double shiftX = 31;
    double shiftY = 31;


    //
    // Initializing
    //
    
public:
    
    Mouse(Amiga& ref, ControlPort& pref);
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
        
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker << config.pullUpResistors;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:
    
    static MouseConfig getDefaultConfig();
    const MouseConfig &getConfig() const { return config; }
    void resetConfig() override;

    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);
    
private:
    
    void updateScalingFactors();
    

    //
    // Accessing
    //

public:

    // Modifies the POTGOR bits according to the current button state
    void changePotgo(u16 &potgo) const;

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra) const;

    
    //
    // Using the mouse
    //
    
public:
    
    // Returns a horizontal or vertical position change
    i64 getDeltaX();
    i64 getDeltaY();

    // Returns the mouse coordinates as they appear in the JOYDAT register
    u16 getXY();
    
    // Runs the shake detector
    bool detectShakeXY(double x, double y);
    bool detectShakeDxDy(double dx, double dy);

    // Emulates a mouse movement
    void setXY(double x, double y);
    void setDxDy(double dx, double dy);

    // Presses or releases a mouse button
    void setLeftButton(bool value);
    void setRightButton(bool value);

    // Triggers a gamepad event
    void trigger(GamePadAction event);

    // Performs periodic actions for this device
    void execute();
};
