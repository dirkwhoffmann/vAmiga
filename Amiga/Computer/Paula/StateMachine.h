// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _STATE_MACHINE_INC
#define _STATE_MACHINE_INC

#include "HardwareComponent.h"

class StateMachine : public HardwareComponent {

    friend class AudioUnit;

    // Quick-access references
    class Agnus *agnus;
    class Paula *paula; 

    //
    // Bookkeeping
    //

    // Information shown in the GUI inspector panel
    AudioChannelInfo info;

public:

    // The audio channel controlled by this state machine
    int8_t nr;

    // The current state of this machine
    int8_t state;

    // Audio length (AUDxLEN)
    uint16_t audlenLatch;
    uint16_t audlen;

    // Audio period (AUDxPER)
    uint16_t audperLatch;
    int32_t audper;

    // Audio volume (AUDxVOL)
    uint16_t audvolLatch;
    uint16_t audvol;

    // Audio data (AUDxDAT)
    uint16_t auddatLatch;
    uint16_t auddat;

    // Audio location (AUDxLC)
    uint32_t audlcLatch;


    //
    // Constructing and destructing
    //

public:

    StateMachine();

    // Sets the channel number (for debugging output only)
    void setNr(uint8_t nr);


    //
    // Methods from HardwareComponent
    //

private:

    void _initialize() override;
    void _inspect() override;

    //
    // Accessing properties
    //

public:

    // Returns the latest internal state recorded by inspect()
    AudioChannelInfo getInfo();


    //
    // Running the device
    //

private:

    // DMA on "x" indicates channel number (signal from DMACON).
    bool AUDxON();

    // Audio interrupt pending
    bool AUDxIP();
    
    // Reloads period counter from back-up latch
    void percntrld() { audper += audperLatch; }

    // Count period counter down
    void percount(int32_t cycles) { audper -= cycles; }

    // Period counter finished (value <= 1)
    bool perfin() { return audper <= 1; }

    // Reloads length counter from back-up latch
    void lencntrld() { audlen = audlenLatch; }

    // Reloads volume counter from back-up latch
    void volcntrld() { audvol = audvolLatch; }

    // Count length counter down one notch
    void lencount() { audlen--; }

    // Length countel' finished (value = 1)
    bool lenfin() { return audlen <= 1; }

public:

    // Move the machine to a specific state
    void setState(uint8_t state) { this->state = state; }

    /* Executes the state machine for a certain number of DMA cycles.
     * The return value is the current audio sample of this channel.
     */
    int16_t execute(DMACycle cycles);
};


#endif
