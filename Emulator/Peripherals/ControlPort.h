// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CONTROL_PORT_INC
#define _CONTROL_PORT_INC

#include "AmigaComponent.h"

class ControlPort : public AmigaComponent {

    // Represented control port
    PortNr nr;

    // Result of the latest inspection
    ControlPortInfo info;

    // Connected device
    ControlPortDevice device = CPD_NONE;
    
    // The two mouse position counters
    i64 mouseCounterX = 0;
    i64 mouseCounterY = 0;

    // Resistances on the potentiometer lines (specified as a delta charge)
    double chargeDX;
    double chargeDY;


    //
    // Constructing and destructing
    //
    
public:
    
    ControlPort(PortNr nr, Amiga& ref);


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
        worker

        & mouseCounterX
        & mouseCounterY
        & chargeDX
        & chargeDY;
    }


    //
    // Methods from HardwareComponent
    //

private:

    void _reset() override { RESET_SNAPSHOT_ITEMS }
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Reading the internal state
    //

public:

    // Returns the latest internal state recorded by inspect()
    ControlPortInfo getInfo();

    // Getter for the delta charges
    i16 getChargeDX() { return chargeDX; }
    i16 getChargeDY() { return chargeDY; }
    

    //
    // Managing registers
    //
    
    public:

    // Returns the control port bits showing up in the JOYxDAT register
    u16 joydat();

    // Emulates a write access to JOYTEST
    void pokeJOYTEST(u16 value);

    // Modifies the POTGOR bits according to the connected device
    void changePotgo(u16 &potgo);

    // Modifies the PRA bits of CIA A according to the connected device
    void changePra(u8 &pra);

    
    //
    // Connecting peripherals
    //
    
public:
    
    // Connects or disconnects a device
    void connectDevice(ControlPortDevice device);
};

#endif

