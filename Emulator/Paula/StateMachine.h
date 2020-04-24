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

#include "AmigaComponent.h"

template <int nr>
class StateMachine : public AmigaComponent {

    friend class AudioUnit;

    // Result of the latest inspection
    AudioChannelInfo info;

public:

    // The state machine has been executed up to this clock cycle
    Cycle clock;

    // The current state of this machine
    i8 state;

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
    // u16 auddat;

    // Audio location (AUDxLC)
    u32 audlcLatch;

    // Set to true if the next 011->010 transition should trigger an interrupt
    bool intreq2;

    // Ringbuffer storing the synthesized samples
    SortedRingBuffer<short, 256> samples;

    
    //
    // Constructing and serializing
    //

public:

    StateMachine(Amiga& ref);

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & clock
        & state
        & buffer
        & audlenLatch
        & audlen
        & audperLatch
        & audper
        & audvolLatch
        & audvol
        & auddat
        & audlcLatch
        & intreq2
        & samples;
    }


    //
    // Methods from HardwareComponent
    //

private:

    void _dump() override;
    void _reset() override;
    void _inspect() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing properties
    //

public:

    // Returns the latest internal state recorded by inspect()
    AudioChannelInfo getInfo() { return HardwareComponent::getInfo(info); }

    
    //
    // Accessing registers
    //

public:

    // OCS registers 0xA4, 0xB4, 0xC4, 0xD4 (w)
    void pokeAUDxLEN(u16 value);

    // OCS registers 0xA6, 0xB6, 0xB6, 0xD6 (w)
    void pokeAUDxPER(u16 value);

    // OCS registers 0xA8, 0xB8, 0xC8, 0xD8 (w)
    void pokeAUDxVOL(u16 value);

    // OCS registers 0xAA, 0xBA, 0xCA, 0xDA (w)
    void pokeAUDxDAT(u16 value);


    //
    // Synthesizing samples
    //

    template <SamplingMethod method> i16 interpolate(Cycle clock);

    
    //
    // Performing state machine actions
    //

public:

    // Returns true if the state machine is running in DMA mode
    bool AUDxON();

    // Returns true if the audio interrupt is pending
    bool AUDxIP();

    // Asks Paula to trigger the audio interrupt
    void AUDxIR();

    // Asks Agnus for one word of data
    void AUDxDR() { agnus.setAudxDR<nr>(); }

    // Tells Agnus to reset the DMA pointer to the block start
    void AUDxDSR() { agnus.reloadAUDxPT<nr>(); }

    // Reloads the period counter from its backup latch
    void percntrld();

    // Reloads the length counter from its backup latch
    void lencntrld() { audlen = audlenLatch; }

    // Counts length counter down one notch
    void lencount() { audlen--; }

    // Checks if the length counter has finished
    bool lenfin() { return audlen == 1; }

    // Reloads the volume register from its backup latch
    void volcntrld() { audvol = audvolLatch; }

    // Loads the output buffer from holding latch written to by AUDxDAT
    void pbufld1();

    // Like pbufld1, but only during 010->011 transition with attach period
    void pbufld2();

    // Returns true in attach volume mode
    bool AUDxAV();

    // Returns true in attach period mode
    bool AUDxAP();

    // Condition for normal DMA and interrupt requests
    bool napnav() { return !AUDxAP() || AUDxAV(); }

    // Enables the high byte of data to go to the digital-analog converter
    void penhi();

    // Enables the high byte of data to go to the digital-analog converter
    void penlo();


    //
    // Performing state machine transitions
    //

public:

    void enableDMA();
    void disableDMA();

private:

    void move_000_010();
    void move_000_001();
    void move_001_000();
    void move_001_101();
    void move_101_000();
    void move_101_010();
    void move_010_011();
    void move_011_000();
    void move_011_010();

    //
    // Servicing events
    //

public:

    void serviceEvent();
};

#endif
