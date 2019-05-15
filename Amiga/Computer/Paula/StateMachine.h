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
    
public:

    // The audio channel controlled by this state machine
    int8_t nr;

    // The current state of this machine
    int8_t state;

    // Audio length (AUDxLEN)
    uint16_t audlen;
    uint16_t audlenInternal;

    // Audio period (AUDxPER)
    uint16_t audper;
    int32_t audperInternal;

    // Audio volume (AUDxVOL)
    uint16_t audvol;
    uint16_t audvolInternal;

    // Audio data (AUDxDAT)
    uint16_t auddat;
    uint16_t auddatInternal;

    uint32_t audlcLatch;


    //
    // Constructing and destructing
    //

public:

    StateMachine();

    // Sets the channel number (for debugging output only)
    void setNr(uint8_t nr);


    //
    // Configuring the device
    //

public:


    //
    // Running the device
    //

    // Move the machine to a specific state
    void setState(uint8_t state) { this->state = state; }

    /* Executes the state machine for a certain number of DMA cycles.
     * The return value is the current audio sample of this channel.
     */
    int16_t execute(DMACycle cycles);

    // Returns the current sample from this audio channel
    // int16_t getSample() { return (int8_t)auddatInternal * audvol; }
};


#endif
