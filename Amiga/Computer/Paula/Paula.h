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

    // Quick-access references
    class CPU *cpu;
    class Agnus *agnus;

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

    // Potentiometer counter for the first control port
    uint8_t potCntX0;
    uint8_t potCntY0;

    // Potentiometer counter for the second control port
    uint8_t potCntX1;
    uint8_t potCntY1;

    // The Audio and Disk Control Register (ADKCON)
    uint16_t adkcon;

    
    //
    // Constructing and destructing
    //
    
public:
    
    Paula();


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
        & potgo
        & potCntX0
        & potCntY0
        & potCntX1
        & potCntY1
        & adkcon;
    }
    
    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
    void _reset() override { RESET_SNAPSHOT_ITEMS }
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

    // OCS registers 0x012 and 0x014 (r)
    uint16_t peekPOTxDAT(int x);

    // OCS register 0x016 (r) (originally called POTINP)
    uint16_t peekPOTGOR();
    
    // OCS register 0x030 (w)
    void pokePOTGO(uint16_t value);


    //
    // Serving events
    //
    
public:

    void servePotEvent(EventID id);

    
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
