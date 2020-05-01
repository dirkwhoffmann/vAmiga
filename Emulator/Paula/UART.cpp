// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

UART::UART(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("UART");
}

void
UART::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS
    outBit = 1;
}

void
UART::_inspect()
{
    pthread_mutex_lock(&lock);

    info.receiveBuffer = receiveBuffer;
    info.receiveShiftReg = receiveShiftReg;
    info.transmitBuffer = transmitBuffer;
    info.transmitShiftReg = transmitShiftReg;

    pthread_mutex_unlock(&lock);
}

void
UART::_dump()
{
    msg("   serper: %X\n", serper);
}

u16
UART::peekSERDATR()
{
    // Get the RBF bit from INTREQ
    bool rbf = GET_BIT(paula.intreq, 11);

    // Clear the overrun bit if the interrupt has been acknowledged
    if (!rbf) ovrun = false;

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
    REPLACE_BIT(result, 14, rbf);
    REPLACE_BIT(result, 13, transmitBuffer == 0);
    REPLACE_BIT(result, 12, transmitShiftReg == 0);
    REPLACE_BIT(result, 11, serialPort.getRXD());

    // debug(SER_DEBUG, "peekSERDATR() = %X\n", result);

    return result;
}

void
UART::pokeSERDAT(u16 value)
{
    debug(SER_DEBUG, "pokeSERDAT(%X)\n", value);

    // Write value into the transmit buffer
    transmitBuffer = value & 0x3FF;

    // Start the transmission if the shift register is empty
    if (transmitShiftReg == 0 && transmitBuffer != 0) copyToTransmitShiftRegister();
}

void
UART::pokeSERPER(u16 value)
{
    debug(SER_DEBUG, "pokeSERPER(%X)\n", value);

    serper = value;
}

void
UART::copyToTransmitShiftRegister()
{
    debug(SER_DEBUG, "Copying %X into transmit shift register\n", transmitBuffer);

    assert(transmitShiftReg == 0);
    assert(transmitBuffer != 0);

    // Inform the GUI about the outgoing data
    amiga.putMessage(MSG_SER_OUT, transmitBuffer);
    debug(POT_DEBUG, "transmitBuffer: %X ('%c')\n", transmitBuffer & 0xFF, transmitBuffer & 0xFF);

    // Move the contents of the transmit buffer into the shift register
    transmitShiftReg = transmitBuffer;
    transmitBuffer = 0;

    // Prefix the data with a start bit (leading 0)
    transmitShiftReg <<= 1;

    // Trigger a TBE interrupt
    debug(SER_DEBUG, "Triggering TBE interrupt\n");
    paula.raiseIrq(INT_TBE);

    // Schedule the transmission of the first bit
    agnus.scheduleRel<TXD_SLOT>(0, TXD_BIT);
}

void
UART::copyFromReceiveShiftRegister()
{
    static int count = 0;

    debug(SER_DEBUG, "Copying %X into receive buffer\n", receiveShiftReg);
    
    receiveBuffer = receiveShiftReg;
    receiveShiftReg = 0;

    // Inform the GUI about the incoming data
    amiga.putMessage(MSG_SER_IN, receiveBuffer);

    // msg("receiveBuffer: %X ('%c')\n", receiveBuffer & 0xFF, receiveBuffer & 0xFF);

    count++;

    // Update the overrun bit
    // Bit will be 1 if the RBF interrupt hasn't been acknowledged yet
    ovrun = GET_BIT(paula.intreq, 11);
    if (ovrun) debug(SER_DEBUG, "OVERRUN BIT IS 1\n");

    // Trigger the RBF interrupt (Read Buffer Full)
    debug(SER_DEBUG, "Triggering RBF interrupt\n");
    paula.raiseIrq(INT_RBF);
}

void
UART::updateTXD()
{
    // If the UARTBRK bit is set, the TXD line is forced to 0
    serialPort.setTXD(outBit && !paula.UARTBRK());
}

void
UART::rxdHasChanged(bool value)
{
    // Schedule the first reception event if transmission has not yet started
    if (value == 0 && !agnus.hasEvent<RXD_SLOT>()) {

        // Reset the bit counter
        recCnt = 0;

        // Trigger the event in the middle of the first data bit
        Cycle delay = rate() * 3 / 2;

        // Schedule the event
        agnus.scheduleRel<RXD_SLOT>(delay, RXD_BIT);
    }
}

void
UART::serviceTxdEvent(EventID id)
{
    // debug(SER_DEBUG, "serveTxdEvent(%d)\n", id);

    switch (id) {

        case TXD_BIT:

            // This event should not occurr if the shift register is empty
            assert(!shiftRegEmpty());

            // Shift out bit and let it appear on the TXD line
            debug(SER_DEBUG, "Transmitting bit %d\n", transmitShiftReg & 1);
            outBit = transmitShiftReg & 1;
            transmitShiftReg >>= 1;
            updateTXD();

            // Check if the shift register is empty
            if (!transmitShiftReg) {

                // Check if there is a new data packet to send
                if (transmitBuffer) {

                    // Copy new packet into shift register
                    // debug("Transmission continues with packet %X '%c'\n", transmitBuffer, transmitBuffer & 0xFF);
                    copyToTransmitShiftRegister();

                } else {

                    // Abort the transmission
                    debug(SER_DEBUG, "End of transmission\n");
                    agnus.cancel<TXD_SLOT>();
                    break;
                }
            }

            // Schedule the next event
            agnus.scheduleRel<TXD_SLOT>(rate(), TXD_BIT);
            break;

        default:
            assert(false);
    }
}

void
UART::serviceRxdEvent(EventID id)
{
    // debug(SER_DEBUG, "serveRxdEvent(%d)\n", id);

    bool rxd = serialPort.getRXD();
    // debug(SER_DEBUG, "Receiving bit %d: %d\n", recCnt, rxd);

    // Shift in bit from RXD line
    REPLACE_BIT(receiveShiftReg, recCnt++, rxd);

    // Check if this was the last bit to receive
    if (recCnt >= packetLength() + 2) {

        // Copy shift register contents into the receive buffer
        copyFromReceiveShiftRegister();
        debug(SER_DEBUG, "Received packet %X\n", receiveBuffer);

        // Stop receiving if the last bit was a stop bit
        if (rxd) {
            agnus.cancel<RXD_SLOT>();
            return;
        } else {
            // Prepare for the next packet
            recCnt = 0;
        }
    }

    // Schedule the next reception event
    agnus.scheduleRel<RXD_SLOT>(rate(), RXD_BIT);
}
