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

class Paula : public HardwareComponent {
    
public:
    
    // Paula has been executed up to this clock cycle.
    Cycle clock = 0;
    
    // Audio unit
    AudioUnit audioUnit;
    
    
    //
    // Interrupt registers
    //
    
    // The interrupt request register
    uint16_t intreq;
    
    // The interrupt enable register
    uint16_t intena;

    
    //
    // Disk DMA registers
    //
    
    // Disk DMA block length
    uint16_t dsklen;
    
    // Disk write data (from RAM to disk)
    uint16_t dskdat;
    
    
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
    
    
    //
    // Audio registers
    //
    
    // Audio length (AUDxLEN)
    uint16_t audlen[4];

    // Audio period (AUDxPER)
    uint16_t audper[4];
    
    // Audio volume (AUDxVOL)
    uint16_t audvol[4];
    
    // Audio data (AUDxDAT)
    uint16_t auddat[4];

    
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
    /*
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    */
    void _setWarp(bool value) override;
    
    
    //
    // Collecting information
    //
    
public:
    
    // Collects the data shown in the GUI's debug panel.
    PaulaInfo getInfo();
    
    
    //
    // Register access
    //
    
public:
    
    uint16_t peekINTREQ();
    void pokeINTREQ(uint16_t value);
 
    uint16_t peekINTENA();
    void pokeINTENA(uint16_t value);
    
    uint16_t peekDSKLEN() { return dsklen; }
    void pokeDSKLEN(uint16_t value) { dsklen = value; }
    
    uint16_t peekDSKDAT() { return dskdat; }
    void pokeDSKDAT(uint16_t value) { dskdat = value; }
    
    uint16_t peekSERDAT() { return serdat; }
    void pokeSERDAT(uint16_t value) { serdat = value; }
    
    uint16_t peekSERPER() { return serper; }
    void pokeSERPER(uint16_t value) { serper = value; }

    uint16_t peekPOTGO();
    void pokePOTGO(uint16_t value);

    void pokeAUDxLEN(int x, uint16_t value);
    void pokeAUDxPER(int x, uint16_t value);
    void pokeAUDxVOL(int x, uint16_t value);
    void pokeAUDxDAT(int x, uint16_t value);

    
    
    //
    // Managing events
    //
    
public:
    

    
    
    //
    // Interrupts
    //
    
public:

    // Changes the value of INTREQ
    void setINTREQ(uint16_t value);

    // Changes the value of INTENA
    void setINTENA(uint16_t value);
    

private:

    // Computes the interrupt level of a pending interrupt.
    int interruptLevel();

    // Checks intena and intreq and triggers an interrupt (if pending)
    void checkInterrupt();
    
    // Sets a bit in register INTENA
    // DEPRECATED
    void setBitINTENA(unsigned nr) { intena |= (1 << nr); checkInterrupt(); }

    // Clears a bit in register INTENA
    // DEPRECATED
    void clearBitINTENA(unsigned nr) { intena &= ~(1 << nr); checkInterrupt(); }

    // Sets a bit in register INTREQ
    // DEPRECATED
    void setBitINTREQ(unsigned nr) { intreq |= (1 << nr); checkInterrupt(); }

    // Clears a bit in register INTREQ
    // DEPRECATED
    void clearBitINTREQ(unsigned nr) { intreq &= ~(1 << nr); checkInterrupt(); }


    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    /*
    uint32_t getVolume() { return 42; }
    long bufferUnderflows() { return 42; }
    long bufferOverflows() { return 42; }
    double fillLevel() { return .5; }
     */
};

#endif
