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

#include "AmigaComponent.h"

class UART : public AmigaComponent {

    friend class Amiga;
    
    // Result of the latest inspection
    UARTInfo info;

    // Collected statistical information
    UARTStats stats;

    // Port period and control register
    u16 serper;

    // Input registers
    u16 receiveBuffer;
    u16 receiveShiftReg;

    // Output registers
    u16 transmitBuffer;
    u16 transmitShiftReg;

    // Bit that is currently outputted on the TXD line
    bool outBit; 

    // Overrun bit
    bool ovrun;

    // Bit reception counter
    u8 recCnt;

    //
    // Constructing and destructing
    //

public:

    UART(Amiga& ref);

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

    void _reset() override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Reading the internal state
    //

public:

    // Returns the latest internal state recorded by inspect()
    UARTInfo getInfo();

    // Returns statistical information about the current activiy
    UARTStats getStats() { return stats; }

    // Resets the collected statistical information
    void clearStats() { memset(&stats, 0, sizeof(stats)); }


    //
    // Accessing registers
    //

public:
    
    // OCS register $018(r) (Serial port data and status read)
    u16 peekSERDATR();

    // OCS register $030(w) (Serial port data and stop bits write)
    void pokeSERDAT(u16 value);

    // OCS register $032(w) (Serial port period and control)
    void pokeSERPER(u16 value);

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
    void serviceTxdEvent(EventID id);

    // Process a bit reception event
    void serviceRxdEvent(EventID id);

};

#endif
