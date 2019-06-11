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
    // Registers
    //

    // Serial data register
    uint16_t serdat;

    // Port period and control register
    uint16_t serper;


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
    // Accessing registers
    //

public:
    
    // OCS register $018(r) (Serial port data and status read)
    uint16_t peekSERDATR();

    // OCS register $030(w) (Serial port data and stop bits write)
    void pokeSERDAT(uint16_t value);

    // OCS register $032(w) (Serial port period and control)
    void pokeSERPER(uint16_t value);

}; 


#endif

