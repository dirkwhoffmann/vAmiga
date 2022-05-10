// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ControlPortTypes.h"
#include "SubComponent.h"
#include "Joystick.h"
#include "Mouse.h"

class ControlPort : public SubComponent {
    
    // The represented control port
    isize nr;

    // The result of the latest inspection
    mutable ControlPortInfo info = {};
    
    // The connected device
    ControlPortDevice device = CPD_NONE;
    
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
    // Sub components
    //

public:
    
    Mouse mouse = Mouse(amiga, *this);
    Joystick joystick = Joystick(amiga, *this);


    //
    // Initializing
    //
    
public:
    
    ControlPort(Amiga& ref, isize nr);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }
    void _inspect() const override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker

        << mouseCounterX
        << mouseCounterY
        << chargeDX
        << chargeDY;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Analyzing
    //

public:
    
    ControlPortInfo getInfo() const { return AmigaComponent::getInfo(info); }    

    bool isPort1() const { return nr == PORT_1; }
    bool isPort2() const { return nr == PORT_2; }

    
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
};
