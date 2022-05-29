// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "UART.h"
#include "Agnus.h"
#include "IOUtils.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "RemoteManager.h"
#include "SerialPort.h"
#include <iostream>

void
UART::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    outBit = 1;
}

void
UART::_inspect() const
{
    SYNCHRONIZED
    
    info.serper = serper;
    info.baudRate = baudRate();
    info.receiveBuffer = receiveBuffer;
    info.receiveShiftReg = receiveShiftReg;
    info.transmitBuffer = transmitBuffer;
    info.transmitShiftReg = transmitShiftReg;
}

void
UART::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::State) {
        
        os << tab("Serper");
        os << hex(serper) << std::endl;
    }
}

u16
UART::peekSERDATR()
{
    // Get the RBF bit from INTREQ
    bool rbf = GET_BIT(paula.intreq, 11);

    // Clear the overrun bit if the interrupt has been acknowledged
    if (!rbf) ovrun = false;

    u16 result = spypeekSERDATR();
    trace(SER_DEBUG, "peekSERDATR() = %x\n", result);
    return result;
}

u16
UART::spypeekSERDATR() const
{
    /* 15      OVRUN      Serial port receiver overun
     * 14      RBF        Serial port receive buffer full
     * 13      TBE        Serial port transmit buffer empty
     * 12      TSRE       Serial port transmit shift register empty
     * 11      RXD        Input line
     * 10      -
     * 09      STP        Stop bit
     * 08      STP / DB8  Stop bit if LONG, data bit if not.
     * 07..00  DB7 - DB0  Data bits
     */
    u16 result = receiveBuffer & 0x3FF;
    
    REPLACE_BIT(result, 15, ovrun);
    REPLACE_BIT(result, 14, GET_BIT(paula.intreq, 11));
    REPLACE_BIT(result, 13, transmitBuffer == 0);
    REPLACE_BIT(result, 12, transmitShiftReg == 0);
    REPLACE_BIT(result, 11, serialPort.getRXD());

    return result;
}

void
UART::pokeSERDAT(u16 value)
{
    trace(SER_DEBUG, "pokeSERDAT(%04x)\n", value);

    // Experimental findings:
    // From here, the TSRE bit goes high in
    // DMA_CYCLES(1) + (bitcount(value) + 1) * pulseWidth() cycles

    // Schedule the write cycle
    agnus.recordRegisterChange(DMA_CYCLES(1), SET_SERDAT, value);
}

void
UART::setSERDAT(u16 value)
{
    trace(SER_DEBUG, "setSERDAT(%04x)\n", value);

    // Write value into the transmit buffer
    transmitBuffer = value;

    // Start the transmission if the shift register is empty
    if (transmitShiftReg == 0 && transmitBuffer != 0) {
        agnus.scheduleRel <SLOT_TXD> (DMA_CYCLES(0), TXD_BIT);
    }
}

void
UART::pokeSERPER(u16 value)
{
    trace(SPRREG_DEBUG, "pokeSERPER(%04x)\n", value);

    setSERPER(value);
}

void
UART::setSERPER(u16 value)
{
    trace(SER_DEBUG, "setSERPER(%04x)\n", value);
    serper = value;
    trace(SER_DEBUG, "New baud rate = %ld\n", baudRate());
}

void
UART::copyToTransmitShiftRegister()
{
    trace(SER_DEBUG, "Copying %04x into transmit shift register\n", transmitBuffer);

    assert(transmitShiftReg == 0);
    assert(transmitBuffer != 0);

    // Send the byte to the null modem cable
    auto byte = (char)(transmitBuffer & 0xFF);
    remoteManager.serServer << byte;
    
    // Inform the GUI about the outgoing data
    msgQueue.put(MSG_SER_OUT, transmitBuffer);
    trace(SER_DEBUG, "transmitBuffer: %X ('%c')\n", byte, byte);
    
    // Move the contents of the transmit buffer into the shift register
    transmitShiftReg = transmitBuffer;
    transmitBuffer = 0;

    // Prefix the data with a start bit (leading 0)
    transmitShiftReg <<= 1;

    // Trigger a TBE interrupt
    trace(SER_DEBUG, "Triggering TBE interrupt\n");
    paula.scheduleIrqRel(INT_TBE, DMA_CYCLES(2));
}

void
UART::copyFromReceiveShiftRegister()
{
    static int count = 0;

    trace(SER_DEBUG, "Copying %X into receive buffer\n", receiveShiftReg);
    
    receiveBuffer = receiveShiftReg;
    receiveShiftReg = 0;

    // Inform the GUI about the incoming data
    msgQueue.put(MSG_SER_IN, receiveBuffer);

    count++;

    // Update the overrun bit
    ovrun = GET_BIT(paula.intreq, 11);
    if (ovrun) { trace(SER_DEBUG, "OVERRUN BIT IS 1\n"); }

    // Trigger the RBF interrupt (Read Buffer Full)
    trace(SER_DEBUG, "Triggering RBF interrupt\n");
    paula.raiseIrq(INT_RBF);
}

void
UART::updateTXD()
{
    // Get the UARTBRK bit
    bool uartbrk = GET_BIT(paula.adkcon, 11);
    
    // If the bit is set, force the TXD line to 0
    serialPort.setTXD(outBit && !uartbrk);
}

void
UART::rxdHasChanged(bool value)
{
    // Schedule the first reception event if transmission has not yet started
    if (value == 0 && !agnus.hasEvent<SLOT_RXD>()) {

        // Reset the bit counter
        recCnt = 0;

        // Trigger the event in the middle of the first data bit
        Cycle delay = pulseWidth() * 3 / 2;

        // Schedule the event
        agnus.scheduleRel<SLOT_RXD>(delay, RXD_BIT);
    }
}
