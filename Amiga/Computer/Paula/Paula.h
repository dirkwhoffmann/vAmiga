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

class Paula : public HardwareComponent {
    
    private:
    
    // Information shown in the GUI inspector panel
    PaulaInfo info;
    
    
    //
    // Sub components
    //
    
    public:
    
    // Audio unit
    AudioUnit audioUnit;
    
    // The disk controller
    DiskController diskController;
    
    
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
    // Serial port registers
    //
    
    // Serial port data and stop bits write
    uint16_t serdat;
    
    // Serial port period and control
    uint16_t serper;
    
    
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
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    PaulaInfo getInfo();
    
    
    //
    // Accessing registers
    //
    
    public:
    
    // OCS register 0x010 (r)
    uint16_t peekADKCONR() { debug("peekADKCONR()"); return adkcon; }
    
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
    
    // OCS register 0x018 (r)
    uint16_t peekSERDATR() { return serdat; }
    
    // OCS register 0x030 (w)
    void pokeSERDAT(uint16_t value) { serdat = value; }
    
    // OCS register 0x032 (w)
    void pokeSERPER(uint16_t value) { serper = value; }
    
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
    
};

#endif
