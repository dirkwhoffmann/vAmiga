// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

UART::UART()
{
    setDescription("UART");

    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {

        { &txd,        sizeof(txd),        0 },

        { &serdat,     sizeof(serdat),     0 },
        { &serper,     sizeof(serper),     0 },
        { &shiftReg,   sizeof(shiftReg),   0 },
        { &recCnt,     sizeof(recCnt),     0 },
    });
}

void
UART::_initialize()
{
    events = &amiga->agnus.events;
    paula = &amiga->paula;
    serialPort = &amiga->serialPort;
}

void
UART::_powerOn()
{
    // tbe = true;
}

void
UART::_powerOff()
{

}

void
UART::_reset()
{

}

void
UART::_dump()
{
    plainmsg("   serper: %X\n", serper);
}

uint16_t
UART::peekSERDATR()
{
    uint16_t result = 0; // = receiveBuffer

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

    if (shiftReg == 0) SET_BIT(result, 12);
    if (serdat == 0)   SET_BIT(result, 13);

    // Bits 14, 15: MISSING

    debug(SER_DEBUG, "peekSERDATR() = %X\n", result);

    return result;
}

void
UART::pokeSERDAT(uint16_t value)
{
    debug(SER_DEBUG, "pokeSERDAT(%X)\n", value);

    // Write value into SERDAT register
    serdat = value & 0x3FF;
    // tbe = false;

    // Start the transmission if the shift register is empty
    if (shiftReg == 0 && serdat != 0) fillShiftRegister();
}

void
UART::pokeSERPER(uint16_t value)
{
    debug(SER_DEBUG, "pokeSERPER(%X)\n", value);

    serper = value;
}

void
UART::fillShiftRegister()
{
    debug(SER_DEBUG, "fillShiftRegister(%X)\n", serdat);

    assert(shiftReg == 0);
    assert(serdat != 0);

    // Move the payload from serdat into the shift register
    shiftReg = serdat;
    serdat = 0;

    // Prefix the payload with a start bit (leading 0)
    shiftReg <<= 1;

    // Trigger the TBE interrupt
    debug(SER_DEBUG, "Triggering TBE interrupt\n");
    paula->pokeINTREQ(0x8001);

    // Schedule the transmission of the first bit
    events->scheduleSecRel(TXD_SLOT, 0, TXD_BIT);
}

void
UART::serveTxdEvent(EventID id)
{
    static int tmp = 0; // For debugging only

    switch (id) {

        case TXD_BIT:

            // This event should not occurr if the shift register is empty
            assert(!shiftRegEmpty());

            // Shift the right-most bit out
            txd = shiftReg & 1;
            shiftReg >>= 1;
            tmp = (tmp << 1) | txd;
            // debug("Bit: %X\n", txd);

            // Transmit the next bit if the shift register is not empty
            if (shiftReg) {
                events->scheduleSecRel(TXD_SLOT, rate(), TXD_BIT);
                return;
            }

            // Continue with the next data packet if serdat is not empty
            if (serdat) {
                debug(SER_DEBUG, "Transmission continues with packet %X\n", serper);
                fillShiftRegister();
                return;
            }

            // Terminate the transmission
            debug(SER_DEBUG, "End of transmission\n");
            events->cancelSec(TXD_SLOT);

            break;

        default:
            assert(false);
    }
}

void
UART::serveRxdEvent(EventID id)
{
    debug(SER_DEBUG, "serveRxdEvent(%d)\n", id);

    debug("Receiving bit: %d (recCnt = %d)\n", serialPort->getRXD(), recCnt);

    // Schedule next event or abort
    if (recCnt > 0) {
        events->scheduleSecRel(RXD_SLOT, rate(), RXD_BIT);
        recCnt--;
    } else {
        events->cancelSec(RXD_SLOT);
    }
}
