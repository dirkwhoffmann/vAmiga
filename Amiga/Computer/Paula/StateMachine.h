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

    //
    // Bookkeeping
    //

    // Information shown in the GUI inspector panel
    AudioChannelInfo info;

public:

    // The state machine has been executed up to this clock cycle
    Cycle clock;

    // The current state of this machine
    int8_t state;

    // Audio length (AUDxLEN)
    uint16_t audlenLatch;
    uint16_t audlen;

    // Audio period (AUDxPER)
    uint16_t audperLatch;
    int32_t  audper;

    // Audio volume (AUDxVOL)
    uint16_t audvolLatch;
    uint16_t audvol;

    // Audio data (AUDxDAT)
    uint16_t auddatLatch;
    uint16_t auddat;

    // Audio location (AUDxLC)
    uint32_t audlcLatch;

    // EXPERIMENTAL (DEPRECATED)
    int samplecnt = 0;

    /* Sample pipeline
     * When the state machine outputs a sample, it is stored in this pipeline
     * together with a time stamp. The pipe is read in the hsync handler of
     * the audio engine to assemble the 44.1kHz audio stream.
     */
    int16_t sampleData[3];
    Cycle   sampleTime[3];


    //
    // Constructing and destructing
    //

public:

    StateMachine(Amiga& ref);


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

        & clock
        & state
        & audlenLatch
        & audlen
        & audperLatch
        & audper
        & audvolLatch
        & audvol
        & auddatLatch
        & auddat
        & audlcLatch
        & sampleData
        & sampleTime;
    }


    //
    // Methods from HardwareComponent
    //

private:

    void _reset() override { RESET_SNAPSHOT_ITEMS }
    void _inspect() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing properties
    //

public:

    // Returns the latest internal state recorded by inspect()
    AudioChannelInfo getInfo();


    //
    // Accessing registers
    //

public:

    // OCS registers 0xA4, 0xB4, 0xC4, 0xD4 (w)
    void pokeAUDxLEN(uint16_t value);

    // OCS registers 0xA6, 0xB6, 0xB6, 0xD6 (w)
    void pokeAUDxPER(uint16_t value);

    // OCS registers 0xA8, 0xB8, 0xC8, 0xD8 (w)
    void pokeAUDxVOL(uint16_t value);

    // OCS registers 0xAA, 0xBA, 0xCA, 0xDA (w)
    void pokeAUDxDAT(uint16_t value);

    // OCS registers 0xA0, 0xB0, 0xC0, 0xD0 (w)
    void pokeAUDxLCH(uint16_t value);

    // OCS registers 0xA0, 0xB0, 0xC0, 0xD0 (w)
    void pokeAUDxLCL(uint16_t value);


    //
    // Working with the sample pipeline
    //

    // Feeds a new sample into the pipeline
    void pushSample(int16_t data, Cycle clock);

    // Reads the proper sample for a given time stamp
    int16_t pickSample(Cycle clock);


    //
    // Performing state machine actions
    //

    void audxDR() { agnus.setAudxDR<nr>(); }

    //
    // Running the device
    //

private:

    // Returns true if the audio unit is operation in DMA mode
    bool dmaMode();

    // Returns true if the audio interrupt is pending
    void triggerIrq();

    // Returns true if the audio interrupt is pending
    bool irqIsPending();

    // Writes a byte of a 16-bit sample to the audio buffer
    void outputLo(uint16_t sample);
    void outputHi(uint16_t sample);

public:

    // Move the machine to a specific state
    void setState(uint8_t state) { this->state = state; }

    /* Executes the state machine for a certain number of DMA cycles.
     * The return value is the current audio sample of this channel.
     * DEPRECATED
     */
    int16_t execute(DMACycle cycles);

    /* Executes the state machine until a given cycle has been reached.
     */
    // int16_t executeUntil(Cycle target); 
};

#endif
