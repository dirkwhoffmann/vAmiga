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

class Paula : public HardwareComponent {
    
private:
    
    // Information shown in the GUI inspector panel
    PaulaInfo info;
    
    
    //
    // Sub components
    //
    
public:

    AudioUnit audioUnit;
    DiskController diskController;
    UART uart;
    
    
    //
    // Counters
    //
    
    // Paula has been executed up to this clock cycle.
    Cycle clock = 0;
    
    
    //
    // Interrupt registers
    //
    
    // The interrupt request register
    uint16_t intreq;
    
    // The interrupt enable register
    uint16_t intena;
    

    //
    // Control port registers
    //
    
    // The pot control register
    uint16_t potgo;
    
    // The Audio and Disk Control Register (ADKCON)
    uint16_t adkcon;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Paula();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dump() override;
    void _setWarp(bool value) override;
    
    
    //
    // Accessing properties
    //
    
public:
    
    // Returns the latest internal state recorded by inspect()
    PaulaInfo getInfo();
    
    
    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x010 (r)
    uint16_t peekADKCONR() { return adkcon; }
    bool UARTBRK() { return GET_BIT(adkcon, 11); }

    // OCS register 0x01E (r)
    uint16_t peekINTREQR() { return intreq; }
    
    // OCS register 0x09C (w)
    void pokeINTREQ(uint16_t value);
    
    // OCS register 0x01C (r)
    uint16_t peekINTENAR() { return intena; }
    
    // OCS register 0x09A (w)
    void pokeINTENA(uint16_t value);
    
    // OCS register 0x09E (w)
    void pokeADKCON(uint16_t value);

    // OCS register 0x016 (r) (orignally called POTINP)
    uint16_t peekPOTGOR();
    
    // OCS register 0x030 (w)
    void pokePOTGO(uint16_t value);
    
    
    //
    // Managing events
    //
    
public:
    
    
    //
    // Managing interrupts
    //
    
public:
    
    // Changes the value of INTREQ.
    void setINTREQ(uint16_t value);
    
    // Changes the value of INTENA.
    void setINTENA(uint16_t value);
    
private:
    
    // Computes the interrupt level of a pending interrupt.
    int interruptLevel();
    
    // Checks intena and intreq and triggers an interrupt (if pending).
    void checkInterrupt();


    //
    // Debugging the component
    //

public:

    // Called by the GUI to manually change the INTENA register
    void debugSetINTENA(unsigned bit, bool value);

    // Called by the GUI to manually change the INTREQ register
    void debugSetINTREQ(unsigned bit, bool value);
};

#endif
