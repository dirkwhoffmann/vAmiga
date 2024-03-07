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

namespace vamiga {

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
                    trace(SER_DEBUG, "Transmitting first packet %x\n", transmitBuffer);
                    copyToTransmitShiftRegister();

                } else {

                    // Abort the transmission
                    trace(SER_DEBUG, "All packets sent\n");
                    agnus.cancel<SLOT_TXD>();
                    break;
                }

            } else {

                // Run the shift register
                trace(SER_DEBUG, "Transmitting bit %d\n", transmitShiftReg & 1);
                transmitShiftReg >>= 1;

                if (!transmitShiftReg && transmitBuffer) {

                    // Copy next packet into shift register
                    trace(SER_DEBUG, "Transmitting next packet %x\n", transmitBuffer);
                    copyToTransmitShiftRegister();
                }
            }

            // Let the bit appear on the TXD line
            outBit = transmitShiftReg & 1;
            updateTXD();

            // Schedule next event
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

        if (!payload.empty()) {

            SYNCHRONIZED

            auto c = payload[0];

            // Overwrite the shift register contents
            receiveShiftReg = c;

            // Remove the character from the buffer
            payload.erase(0, 1);

            // Send a stop bit if necessary
            rxd = payload.empty();
        }

        // Copy shift register contents into the receive buffer
        copyFromReceiveShiftRegister();
        trace(SER_DEBUG, "Received packet %X (%c) (%ld)\n", receiveBuffer, (char)receiveBuffer, packetLength());

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

}
