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

    // References (for quickly accessing other components)
    class Agnus *agnus;
    class Paula *paula;
    class SerialPort *serialPort;

    // Information shown in the GUI inspector panel
    UARTInfo info;
    
    //
    // Variables
    //

    // Port period and control register
    uint16_t serper;

    // Input registers
    uint16_t receiveBuffer;
    uint16_t receiveShiftReg;

    // Output registers
    uint16_t transmitBuffer;
    uint16_t transmitShiftReg;

    // Bit that is currently outputted on the TXD line
    bool outBit; 

    // Overrun bit
    bool ovrun;

    // Bit reception counter
    uint8_t recCnt;

    //
    // Constructing and destructing
    //

public:

    UART();

    //
    // Iterating over snapshot items
    //

    template <class T>
    void forAllSnashotItems(T& worker)
    {
        worker

        & serper
        & receiveBuffer
        & receiveShiftReg
        & transmitBuffer
        & transmitShiftReg
        & outBit
        & ovrun
        & recCnt;
    }

    
    //
    // Methods from HardwareComponent
    //

private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _inspect() override;
    void _dump() override;
    size_t _load(uint8_t *buffer) override;
    size_t _save(uint8_t *buffer) override;


    //
    // Reading the internal state
    //

public:

    // Returns the latest internal state recorded by inspect()
    UARTInfo getInfo();


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

    // Returns the baud rate converted to DMA cycles
    int rate() { return DMA_CYCLES((serper & 0x7FFF) + 1); }

private:

    // Returns the length of a received packet (8 or 9 bits)
    int packetLength() { return GET_BIT(serper, 15) ? 9 : 8; }

    // Returns true if the shift register is empty
    bool shiftRegEmpty() { return transmitShiftReg == 0; }

    // Copies the contents of the transmit buffer to the transmit shift register
    void copyToTransmitShiftRegister();

    // Copies the contents of the receive shift register to the receive buffer
    void copyFromReceiveShiftRegister();


    //
    // Operationg this device
    //

public:

    // Updates the value on the UART's TXD line
    void updateTXD();

    // Called when the RXD port pin changes it's value
    void rxdHasChanged(bool value);


    //
    // Serving events
    //

public:

    // Processes a bit transmission event
    void serveTxdEvent(EventID id);

    // Process a bit reception event
    void serveRxdEvent(EventID id);

};

#endif
