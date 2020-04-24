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

    // Result of the latest inspection
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
    u16 intreq;
    
    // The interrupt enable register
    u16 intena;

    // Trigger cycle for setting a bit in INTREQ
    Cycle setIntreq[16];

    // Value pipe for emulating the four cycle delay on the IPL pins
    u64 iplPipe;
    
    
    //
    // Control port registers
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

    // The Audio and Disk Control Register (ADKCON)
    u16 adkcon;

    
    //
    // Methods
    //
    
public:
    
    Paula(Amiga& ref);
    PaulaInfo getInfo() { return HardwareComponent::getInfo(info); }

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
        & iplPipe
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

private:

    void _reset() override;
    void _inspect() override;
    void _dump() override;
    void _warpOn() override;
    void _warpOff() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x010 (r)
    u16 peekADKCONR() { return adkcon; }
    bool UARTBRK() { return GET_BIT(adkcon, 11); }

    // OCS register 0x01E (r)
    u16 peekINTREQR();

    // OCS register 0x09C (w)
    void pokeINTREQ(u16 value);
    
    // OCS register 0x01C (r)
    u16 peekINTENAR() { return intena; }
    
    // OCS register 0x09A (w)
    void pokeINTENA(u16 value);
    
    // OCS register 0x09E (w)
    void pokeADKCON(u16 value);

    // OCS registers 0x012 and 0x014 (r)
    template <int x> u16 peekPOTxDAT();

    // OCS register 0x016 (r) (originally called POTINP)
    u16 peekPOTGOR();
    bool OUTRY() { return potgo & 0x8000; }
    bool DATRY() { return potgo & 0x4000; }
    bool OUTRX() { return potgo & 0x2000; }
    bool DATRX() { return potgo & 0x1000; }
    bool OUTLY() { return potgo & 0x0800; }
    bool DATLY() { return potgo & 0x0400; }
    bool OUTLX() { return potgo & 0x0200; }
    bool DATLX() { return potgo & 0x0100; }

    // OCS register 0x030 (w)
    void pokePOTGO(u16 value);


    //
    // Serving events
    //
    
public:

    void servicePotEvent(EventID id);

    
    //
    // Managing interrupts
    //
    
public:
    
    // Changes the value of register INTREQ
    void setINTREQ(u16 value) { setINTREQ(value & 0x8000, value & 0x7FFF); }
    void setINTREQ(bool setclr, u16 value);

    // Changes the value of register INTENA
    void setINTENA(u16 value) { setINTENA(value & 0x8000, value & 0x7FFF); }
    void setINTENA(bool setclr, u16 value);

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
