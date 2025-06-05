// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "StateMachineTypes.h"
#include "SubComponent.h"
#include "Sampler.h"
#include "Agnus.h"

namespace vamiga {

template <isize nr>
class StateMachine final : public SubComponent, public Inspectable<StateMachineInfo> {

    Descriptions descriptions = {
        {
            .type           = Class::StateMachine,
            .name           = "StateMachine1",
            .description    = "Audio State Machine 1",
            .shell          = ""
        },
        {
            .type           = Class::StateMachine,
            .name           = "StateMachine2",
            .description    = "Audio State Machine 2",
            .shell          = ""
        },
        {
            .type           = Class::StateMachine,
            .name           = "StateMachine3",
            .description    = "Audio State Machine 3",
            .shell          = ""
        },
        {
            .type           = Class::StateMachine,
            .name           = "StateMachine4",
            .description    = "Audio State Machine 4",
            .shell          = ""
        }
    };

    Options options = {

    };

public:

    // The state machine has been executed up to this clock cycle
    Cycle clock;

    // The current state of this machine
    isize state;

    // The 16 bit output buffer
    u16 buffer;

    // Audio length (AUDxLEN)
    u16 audlenLatch;
    u16 audlen;

    // Audio period (AUDxPER)
    u16 audperLatch;
    i32 audper;

    // Audio volume (AUDxVOL)
    u16 audvolLatch;
    u16 audvol;

    // Audio data (AUDxDAT)
    u16 auddat;

    // Audio location (AUDxLC)
    u32 audlcLatch;

    // Audio DMA request to Agnus for one word of data
    bool audDR;

    // Set to true if the next 011->010 transition should trigger an interrupt
    bool intreq2;

    /* Two locks regulate the access to the sample buffer.
     *
     * "The minimum period is 124 color clocks. This means that the smallest
     *  number that should be placed in this register [AUDxPER] is 124 decimal.
     *  This corresponds to a maximum sample frequency of 28.86 khz." [HRM]
     *
     * Many games initialize AUDxPER with a value of 1 (e.g., James Pond 2 and
     * Ghosts'n Goblins). As a result, the sample buffer is flooded with
     * identical samples. To prevent this, these two variables hinder penlo()
     * and penhi() to write into the sample buffer. The locks are released
     * whenever a new sample is written into the AUDxDAT register.
     *
     * This feature is experimental (and might be well disabled).
     */
    bool enablePenlo = false;
    bool enablePenhi = false;

    
    //
    // Initializing
    //

public:

    StateMachine(Amiga& ref);

    StateMachine& operator= (const StateMachine& other) {

        CLONE(state)
        CLONE(buffer)
        CLONE(audlenLatch)
        CLONE(audlen)
        CLONE(audperLatch)
        CLONE(audper)
        CLONE(audvolLatch)
        CLONE(audvol)
        CLONE(auddat)
        CLONE(audlcLatch)
        CLONE(audDR)
        CLONE(intreq2)
        CLONE(enablePenlo)
        CLONE(enablePenhi)
        CLONE(clock)

        return *this;
    }


    //
    // Methods from Serializable
    //
    
private:
        
    template <class T>
    void serialize(T& worker)
    {
        worker

        << state
        << buffer
        << audlenLatch
        << audlen
        << audperLatch
        << audper
        << audvolLatch
        << audvol
        << auddat
        << audlcLatch
        << audDR
        << intreq2
        << enablePenlo
        << enablePenhi;

        if (isSoftResetter(worker)) return;

        worker

        << clock;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(StateMachineInfo &result) const override;


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Performing state machine actions
    //

public:

    // Called when DMA mode changes
    void enableDMA();
    void disableDMA();

    // Returns true if the state machine is running in DMA mode
    bool AUDxON() const { return agnus.auddma<nr>(); }

    // Returns true if the audio interrupt is pending
    bool AUDxIP() const;

    // Asks Paula to trigger the audio interrupt
    void AUDxIR() const;

    // Asks Agnus for one word of data
    void AUDxDR() { audDR = true; }

    // Tells Agnus to reset the DMA pointer to the block start
    void AUDxDSR() { agnus.reloadAUDxPT<nr>(); }

    // Reloads the period counter from its backup latch
    void percntrld();

    // Reloads the length counter from its backup latch
    void lencntrld() { audlen = audlenLatch; }

    // Counts length counter down one notch
    void lencount() { U16_DEC(audlen, 1); }

    // Checks if the length counter has finished
    bool lenfin() { return audlen == 1; }

    // Reloads the volume register from its backup latch
    void volcntrld() { audvol = audvolLatch; }

    // Loads the output buffer from holding latch written to by AUDxDAT
    void pbufld1();

    // Like pbufld1, but only during 010->011 transition with attach period
    void pbufld2();

    // Returns true in attach volume mode
    bool AUDxAV() const;

    // Returns true in attach period mode
    bool AUDxAP() const;

    // Condition for normal DMA and interrupt requests
    bool napnav() const { return !AUDxAP() || AUDxAV(); }

    // Enables the high byte of data to go to the digital-analog converter
    void penhi();

    // Enables the high byte of data to go to the digital-analog converter
    void penlo();

    // Transfers a DMA request to Agnus (done in the first refresh cycle)
    void requestDMA() { if (audDR) { agnus.setAudxDR<nr>(); audDR = 0; } }
    
    
    //
    // Performing state machine transitions
    //

private:

    void move_000_010();
    void move_000_001();
    void move_001_000();
    void move_001_101();
    void move_101_000();
    void move_101_010();
    void move_010_011();
    void move_011_000();
    void move_010_000();
    void move_011_010();

    
    //
    // Accessing registers
    //

public:

    // Writes a value into an audio register
    void pokeAUDxLEN(u16 value);
    void pokeAUDxPER(u16 value);
    void pokeAUDxVOL(u16 value);
    void pokeAUDxDAT(u16 value);

    
    //
    // Servicing events
    //

public:

    void serviceEvent();
};

}
