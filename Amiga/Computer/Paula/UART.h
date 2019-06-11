// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _UART_INC
#define _UART_INC

#include "HardwareComponent.h"

class UART : public HardwareComponent {

    //
    // References (for quickly accessing other components)
    //

    class EventHandler *events;
    class Paula *paula;


    //
    // Variables
    //

    // Current value of the TXD pin
    bool txd;

    // Serial data register
    uint16_t serdat;

    // Port period and control register
    uint16_t serper;

    // The serial shift register
    uint16_t shiftReg;

    // Transmit-Buffer-Empty bit
    // bool tbe; 

    //
    // Constructing and destructing
    //

public:

    UART();

    //
    // Methods from HardwareComponent
    //

private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _dump() override;

    //
    // Accessing UART interface pins
    //

    /*
    bool getTXD();
    bool setRXD();

    bool getDTR();
    bool getRTS();
    bool getCD();
    bool getCTS();
    bool getDSR();
    */

    //
    // Accessing registers
    //

public:
    
    // OCS register $018(r) (Serial port data and status read)
    uint16_t peekSERDATR();

    // OCS register $030(w) (Serial port data and stop bits write)
    void pokeSERDAT(uint16_t value);

    // OCS register $032(w) (Serial port period and control)
    void pokeSERPER(uint16_t value);

private:

    // Returns the length of a received packet (8 or 9 bits)
    int packetLength() { return GET_BIT(serper, 15) ? 9 : 8; }

    // Returns the baud rate bits
    int rate() { return serper & 0x7FFF; }

    // Returns true if the shift register is empty
    bool shiftRegEmpty() { return shiftReg == 0; }

    // Fills the shift register and starts the transmission
    void fillShiftRegister();


    //
    // Serving events
    //

public:

    // Processes a event
    void serveEvent(EventID id);

};

#endif
