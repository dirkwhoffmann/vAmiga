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
#include "Paula.h"
#include "SerialPort.h"

void
UART::serviceTxdEvent(EventID id)
{
    trace(SER_DEBUG, "serveTxdEvent(%d)\n", id);

    switch (id) {

        case TXD_BIT:

            if (shiftRegEmpty()) {

                // Check if there is a new data packet to send
                if (transmitBuffer) {

                    // Copy new packet into shift register
                    trace(SER_DEBUG, "Transmitting packet %x\n", transmitBuffer);
                    copyToTransmitShiftRegister();

                } else {

                    // Abort the transmission
                    trace(SER_DEBUG, "All packets sent\n");
                    agnus.cancel<SLOT_TXD>();
                    break;
                }

            } else {

                // Shift out bit and let it appear on the TXD line
                trace(SER_DEBUG, "Transmitting bit %d\n", transmitShiftReg & 1);
                transmitShiftReg >>= 1;

                if (!transmitShiftReg && transmitBuffer) {

                    copyToTransmitShiftRegister();

                    // Trigger a TBE interrupt
                    /*
                    trace(SER_DEBUG, "Triggering TBE interrupt\n");
                    paula.raiseIrq(INT_TBE);
                    // paula.scheduleIrqRel(INT_TBE, DMA_CYCLES(2));
                    */
                }
            }

            // Send bit
            outBit = transmitShiftReg & 1;
            updateTXD();

            // Schedule the next event
            agnus.scheduleRel<SLOT_TXD>(pulseWidth(), TXD_BIT);
            break;

        default:
            fatalError;
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
        trace(SER_DEBUG, "Received packet %X\n", receiveBuffer);

        // Stop receiving if the last bit was a stop bit
        if (rxd) {
            agnus.cancel<SLOT_RXD>();
            return;
        } else {
            // Prepare for the next packet
            recCnt = 0;
        }
    }

    // Schedule the next reception event
    agnus.scheduleRel<SLOT_RXD>(pulseWidth(), RXD_BIT);
}
