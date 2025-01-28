// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "PaulaTypes.h"
#include "SubComponent.h"
#include "AudioFilter.h"
#include "AudioStream.h"
#include "ChangeRecorder.h"
#include "DiskController.h"
#include "AudioPort.h"
#include "StateMachine.h"
#include "UART.h"

namespace vamiga {

class Paula final : public SubComponent, public Inspectable<PaulaInfo> {

    Descriptions descriptions = {{

        .type           = Class::Paula,
        .name           = "Paula",
        .description    = "Audio, Interrupts, Disk Control",
        .shell          = "paula"
    }};

    ConfigOptions options = {

    };

    
    //
    // Subcomponents
    //
    
public:

    // Audio state machines
    StateMachine<0> channel0 = StateMachine<0>(amiga);
    StateMachine<1> channel1 = StateMachine<1>(amiga);
    StateMachine<2> channel2 = StateMachine<2>(amiga);
    StateMachine<3> channel3 = StateMachine<3>(amiga);

    // Disk controller
    DiskController diskController = DiskController(amiga);

    // Universal Asynchronous Receiver Transmitter
    UART uart = UART(amiga);
    
    
    //
    // Counters
    //
    
    // Paula's audio unit has been executed up to this clock cycle
    Cycle audioClock = 0;
    
    
    //
    // Interrupts
    //
    
    // The interrupt request register
    u16 intreq;
    
    // The interrupt enable register
    u16 intena;

    // Trigger cycle for setting a bit in INTREQ
    Cycle setIntreq[16];

    // Value pipe for emulating the delay on the IPL pins
    u64 iplPipe;

    
    //
    // Control ports
    //
    
    // The pot control register
    u16 potgo;

    // Potentiometer counters for the first and the second control port
    u8 potCntX0;
    u8 potCntY0;
    u8 potCntX1;
    u8 potCntY1;

    // Current capacitor charge on all four potentiometer lines
    double chargeX0;
    double chargeY0;
    double chargeX1;
    double chargeY1;

    // The Audio and Disk Control Register
    u16 adkcon;

    
    //
    // Initializing
    //
    
public:

    Paula(Amiga& ref);
    
    Paula& operator= (const Paula& other) {

        CLONE(channel0)
        CLONE(channel1)
        CLONE(channel2)
        CLONE(channel3)
        CLONE(diskController)
        CLONE(uart)

        CLONE(intreq)
        CLONE(intena)
        CLONE_ARRAY(setIntreq)
        CLONE(iplPipe)
        CLONE(potgo)
        CLONE(potCntX0)
        CLONE(potCntY0)
        CLONE(potCntX1)
        CLONE(potCntY1)
        CLONE(chargeX0)
        CLONE(chargeY0)
        CLONE(chargeX1)
        CLONE(chargeY1)
        CLONE(adkcon)
        CLONE(audioClock)

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

        << intreq
        << intena
        << setIntreq
        << iplPipe
        << potgo
        << potCntX0
        << potCntY0
        << potCntX1
        << potCntY1
        << chargeX0
        << chargeY0
        << chargeX1
        << chargeY1
        << adkcon;

        if (isSoftResetter(worker)) return;

        worker

        << audioClock;

    } SERIALIZERS(serialize, override);


    //
    // Methods from CoreObject
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;

    void _run() override;
    void _pause() override;
    void _warpOn() override;
    void _warpOff() override;

    void _didReset(bool hard) override;
    void _didLoad() override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Methods from Inspectable
    //
    
public:
    
    void cacheInfo(PaulaInfo &result) const override;


    //
    // Running the audio unit
    //
    
    void executeUntil(Cycle target);


    //
    // Managing interrupts
    //
    
public:
    
    // Signals an interrupt in INTREQ
    void raiseIrq(IrqSource src) { setINTREQ(true, u16(1 << isize(src))); }
    
    // Schedules an interrupt
    void scheduleIrqAbs(IrqSource src, Cycle trigger);
    void scheduleIrqRel(IrqSource src, Cycle trigger);

private:

    // Updates the IPL pipe
    void checkInterrupt();

    // Computes the interrupt level of a pending interrupt
    u8 interruptLevel();
    
    
    //
    // Accessing registers
    //
    
public:

    u16 peekADKCONR() const;
    void pokeADKCON(u16 value);

    u16 peekINTREQR() const;
    template <Accessor s> void pokeINTREQ(u16 value);
    void setINTREQ(bool setclr, u16 value);
    void setINTREQ(u16 value) { setINTREQ(value & 0x8000, value & 0x7FFF); }

    u16 peekINTENAR() const;
    template <Accessor s> void pokeINTENA(u16 value);
    void setINTENA(bool setclr, u16 value);
    void setINTENA(u16 value) { setINTENA(value & 0x8000, value & 0x7FFF); }

    template <isize x> u16 peekPOTxDAT() const;

    u16 peekPOTGOR() const;
    void pokePOTGO(u16 value);

    
    //
    // Serving events
    //
    
public:

    // Triggers all pending interrupts
    void serviceIrqEvent();

    // Changes the CPU interrupt priority lines
    void serviceIplEvent();

    // Charges or discharges a potentiometer capacitor
    void servicePotEvent(EventID id);

    // Finishes up the current frame
    void eofHandler();
};

}
