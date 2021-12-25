// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "SerServer.h"
#include "Agnus.h"
#include "Scheduler.h"
#include "UART.h"

SerServer::SerServer(Amiga& ref) : RemoteServer(ref)
{
    port = 8080;
}

string
SerServer::doReceive()
{
    return connection.recv();
}

void
SerServer::doProcess(const string &packet)
{
    for (auto c : packet) { processIncomingByte((u8)c); }
}

void
SerServer::doSend(const string &packet)
{

}

void
SerServer::processIncomingByte(u8 byte)
{
    if (buffer.isFull()) {

        debug(SRV_DEBUG, "Buffer overflow\n");
        
    } else {

        buffer.write(byte);
        
        // When enough bytes have been received, leave buffering mode
        if (buffer.count() >= 8) buffering = false;
    }
}

void
SerServer::serviceSerEvent()
{
    assert(scheduler.id[SLOT_SER] == SER_RECEIVE);

    if (!buffer.isEmpty() && !buffering) {
        
        // Hand the oldest buffer element over to the UART
        uart.receiveShiftReg = buffer.read();
        uart.copyFromReceiveShiftRegister();
        skippedTransmissions = 0;
        
    } else {
        
        skippedTransmissions++;
    }
    
    // Leave buffering mode if there are too many skipped transmissions
    if (skippedTransmissions > 8) buffering = false;
        
    scheduleNextEvent();
}

void
SerServer::scheduleNextEvent()
{
    assert(scheduler.id[SLOT_SER] == SER_RECEIVE);
    
    if (!scheduler.isPending <SLOT_SER> ()) {

        // If no event is present, schedule it immediately
        scheduler.scheduleImm <SLOT_SER> (SER_RECEIVE);

    } else {
        
        // Otherwise, emulate proper timing based on the current baud rate
        auto pulseWidth = uart.pulseWidth();
        if (pulseWidth > 40) {
            
            agnus.scheduleRel<SLOT_SER>(8 * pulseWidth, SER_RECEIVE);
            
        } else {
            
            warn("Baud rate is too high: %ld\n", uart.baudRate());
            scheduler.cancel <SLOT_SER> ();
        }
    }
}
