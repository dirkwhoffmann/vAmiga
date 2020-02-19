// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _PAULA_INC
#define _PAULA_INC

#include "AudioUnit.h"
#include "DiskController.h"
#include "UART.h"

class Paula : public AmigaComponent {
    
private:

    // Information shown in the GUI inspector panel
    PaulaInfo info;
    
    
    //
    // Sub components
    //
    
public:

    // Sound chip
    AudioUnit audioUnit = AudioUnit(amiga);

    // Disk controller
    DiskController diskController = DiskController(amiga);

    // Universal Asynchronous Receiver Transmitter
    UART uart = UART(amiga);
    
    
    //
    // Counters
    //
    
    // Paula has been executed up to this clock cycle
    Cycle clock = 0;
    
    
    //
    // Interrupts
    //
    
    // The interrupt request register
    uint16_t intreq;
    
    // The interrupt enable register
    uint16_t intena;

    // Trigger cycle for setting a bit in INTREQ
    Cycle setIntreq[16];


    //
    // Control port registers
    //
    
    // The pot control register
    uint16_t potgo;

    // Potentiometer counters for the first and the second control port
    uint8_t potCntX0;
    uint8_t potCntY0;
    uint8_t potCntX1;
    uint8_t potCntY1;

    // Current capacitor charge on all four potentiometer lines
    double chargeX0;
    double chargeY0;
    double chargeX1;
    double chargeY1;

    // The Audio and Disk Control Register (ADKCON)
    uint16_t adkcon;

    
    //
    // Constructing and destructing
    //
    
public:
    
    Paula(Amiga& ref);


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
        & intreq
        & intena
        & setIntreq
        & potgo
        & potCntX0
        & potCntY0
        & potCntX1
        & potCntY1
        & chargeX0
        & chargeY0
        & chargeX1
        & chargeY1
        & adkcon;
    }

    //
    // Methods from HardwareComponent
    //
    
private:

    void _reset() override;
    void _inspect() override;
    void _dump() override;
    void _warpOn() override;
    void _warpOff() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing properties
    //
    
public:
    
    // Returns the result of the most recent call to inspect()
    PaulaInfo getInfo();
    
    
    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x010 (r)
    uint16_t peekADKCONR() { return adkcon; }
    bool UARTBRK() { return GET_BIT(adkcon, 11); }

    // OCS register 0x01E (r)
    uint16_t peekINTREQR();

    // OCS register 0x09C (w)
    void pokeINTREQ(uint16_t value);
    
    // OCS register 0x01C (r)
    uint16_t peekINTENAR() { return intena; }
    
    // OCS register 0x09A (w)
    void pokeINTENA(uint16_t value);
    
    // OCS register 0x09E (w)
    void pokeADKCON(uint16_t value);

    // OCS registers 0x012 and 0x014 (r)
    template <int x> uint16_t peekPOTxDAT();

    // OCS register 0x016 (r) (originally called POTINP)
    uint16_t peekPOTGOR();
    
    // OCS register 0x030 (w)
    void pokePOTGO(uint16_t value);


    //
    // Serving events
    //
    
public:

    void hsyncHandler();
    void servicePotEvent(EventID id);

    
    //
    // Managing interrupts
    //
    
public:
    
    // Changes the value of register INTREQ
    void setINTREQ(uint16_t value) { setINTREQ(value & 0x8000, value & 0x7FFF); }
    void setINTREQ(bool setclr, uint16_t value);

    // Changes the value of register INTENA
    void setINTENA(uint16_t value) { setINTENA(value & 0x8000, value & 0x7FFF); }
    void setINTENA(bool setclr, uint16_t value);

    // Schedules an interrupt
    void raiseIrq(IrqSource src);
    void scheduleIrqAbs(IrqSource src, Cycle trigger);
    void scheduleIrqRel(IrqSource src, Cycle trigger);

    // Triggers all pending interrupts
    void serviceIrqEvent();

    // Changes the CPU interrupt priority lines
    void serviceIplEvent();
    
private:
    
    // Computes the interrupt level of a pending interrupt.
    int interruptLevel();

public:
    
    // Checks intena and intreq and triggers an interrupt (if pending).
    void checkInterrupt();
};

#endif
