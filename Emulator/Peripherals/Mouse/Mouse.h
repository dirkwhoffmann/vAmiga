// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MouseTypes.h"
#include "AgnusTypes.h"
#include "Joystick.h"
#include "SubComponent.h"
#include "Chrono.h"

namespace vamiga {

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

class Mouse final : public SubComponent {

    Descriptions descriptions = {
        {
            .type           = Class::Mouse,
            .name           = "Mouse1",
            .description    = "Mouse in Port 1",
            .shell          = "mouse1"
        },
        {
            .type           = Class::Mouse,
            .name           = "Mouse2",
            .description    = "Mouse in Port 2",
            .shell          = "mouse2"
        }
    };

    ConfigOptions options = {

        Opt::MOUSE_PULLUP_RESISTORS,
        Opt::MOUSE_SHAKE_DETECTION,
        Opt::MOUSE_VELOCITY
    };

    // Reference to the control port this device belongs to
    ControlPort &port;
    
    // Current configuration
    MouseConfig config = {};

    // Shake detector
    class ShakeDetector shakeDetector;
    
public:
    
    // Mouse button states
    bool leftButton = false;
    bool middleButton = false;
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
    
    Mouse& operator= (const Mouse& other) {

        CLONE(leftButton)
        CLONE(middleButton)
        CLONE(rightButton)
        CLONE(mouseX)
        CLONE(mouseY)
        CLONE(oldMouseX)
        CLONE(oldMouseY)
        CLONE(targetX)
        CLONE(targetY)

        CLONE(config)

        return *this;
    }


    //
    // Methods from CoreObject
    //
    
private:
    
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //

private:
    
    template <class T>
    void serialize(T& worker)
    {
        if (isResetter(worker)) {

            worker 

            << leftButton
            << middleButton
            << rightButton
            << mouseX
            << mouseY
            << oldMouseX
            << oldMouseY
            << targetX
            << targetY;

        } else {

            worker

            << config.pullUpResistors;
        }

    } SERIALIZERS(serialize, override);

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:
    
    const MouseConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;
    
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
    void setMiddleButton(bool value);
    void setRightButton(bool value);

    // Triggers a gamepad event
    void trigger(GamePadAction event);

    // Performs periodic actions for this device
    void execute();
    
    
    //
    // Scheduling and servicing events
    //
    
public:
    
    // Simulates a complete press-and-release cycle for a button
    void pressAndReleaseLeft(Cycle duration = SEC(0.5), Cycle delay = 0);
    void pressAndReleaseMiddle(Cycle duration = SEC(0.5), Cycle delay = 0);
    void pressAndReleaseRight(Cycle duration = SEC(0.5), Cycle delay = 0);

    template <EventSlot s> void serviceMouseEvent();
};

}
