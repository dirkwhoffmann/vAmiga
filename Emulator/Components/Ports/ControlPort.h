// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ControlPortTypes.h"
#include "CmdQueueTypes.h"
#include "SubComponent.h"
#include "Joystick.h"
#include "Mouse.h"

namespace vamiga {

class ControlPort final : public SubComponent, public Inspectable<ControlPortInfo> {

    Descriptions descriptions = {
        {
            .type           = Class::ControlPort,
            .name           = "ControlPort1",
            .description    = "Control Port 1",
            .shell          = "port1"
        },
        {
            .type           = Class::ControlPort,
            .name           = "ControlPort2",
            .description    = "Control Port 2",
            .shell          = "port2"
        }
    };


    ConfigOptions options = {

    };

private:

    // The connected device
    ControlPortDevice device = ControlPortDevice::NONE;

    // The two mouse position counters
    i64 mouseCounterX = 0;
    i64 mouseCounterY = 0;

    // The position of the connected mouse
    i64 mouseX = 0;
    i64 mouseY = 0;

    // Resistances on the potentiometer lines (specified as a delta charge)
    double chargeDX;
    double chargeDY;


    //
    // Subcomponents
    //

public:

    Mouse mouse = Mouse(amiga, *this);
    Joystick joystick = Joystick(amiga, *this);


    //
    // Initializing
    //

public:

    ControlPort(Amiga& ref, isize nr);

    ControlPort& operator= (const ControlPort& other) {

        CLONE(mouse)
        CLONE(joystick)

        CLONE(device)
        CLONE(mouseCounterX)
        CLONE(mouseCounterY)
        CLONE(mouseX)
        CLONE(mouseY)
        CLONE(chargeDX)
        CLONE(chargeDY)

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
        worker

        << mouseCounterX
        << mouseCounterY
        << chargeDX
        << chargeDY;

    } SERIALIZERS(serialize, override);

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(ControlPortInfo &result) const override;

    bool isPort1() const { return objid == 0; }
    bool isPort2() const { return objid == 1; }


    //
    // Accessing
    //

public:

    // Changes the connected device type
    void setDevice(ControlPortDevice value) { device = value; }

    // Getter for the delta charges
    i16 getChargeDX() const { return (i16)chargeDX; }
    i16 getChargeDY() const { return (i16)chargeDY; }

    // Called by the mouse when it's position has changed
    void updateMouseXY(i64 x, i64 y);

    // Returns the control port bits showing up in the JOYxDAT register
    u16 joydat() const;

    // Emulates a write access to JOYTEST
    void pokeJOYTEST(u16 value);

    // Modifies the POTGOR bits according to the connected device
    void changePotgo(u16 &potgo) const;

    // Modifies the PRA bits of CIA A according to the connected device
    void changePra(u8 &pra) const;


    //
    // Processing commands and events
    //

public:

    // Processes a datasette command
    void processCommand(const struct Command &cmd);
};

}
