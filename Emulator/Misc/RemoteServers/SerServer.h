// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"
#include "RingBuffer.h"

class SerServer : public RemoteServer {
      
    // A ringbuffer for buffering incoming bytes
    util::SortedRingBuffer <u8, 8096> buffer;
    
    /* Indicates if we are currenty running in buffering mode. In this mode,
     * incoming bytes are collected in the ring buffer withous passing them
     * to the UART. When a certain number of bytes has been received, buffering
     * mode is left and the collected symbols are fed into the UART with proper
     * timing. Buffering mode is also left when no symbols were incoming for
     * a longer period of time to prevent symbols from starving in the buffer
     * at the end of a transmission.
     */
    bool buffering = true;
        
    // Used to determine when we need to leave buffering mode
    i64 skippedTransmissions = 0;
    
    // Byte counters
    isize receivedBytes = 0;
    isize transmittedBytes = 0;
    isize processedBytes = 0;
    isize lostBytes = 0;

    
public:
    
    SerServer(Amiga& ref);
    

    //
    // Methods from AmigaObject
    //
    
protected:
    
    const char *getDescription() const override { return "SerServer"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from RemoteServer
    //
    
public:
    
    ServerConfig getDefaultConfig() override;
    bool shouldRun() override;
    string doReceive() override;
    void doSend(const string &packet) override;
    void doProcess(const string &packet) override;
    void didSwitch(SrvState from, SrvState to) override;
    
    void processIncomingByte(u8 byte);

    
    //
    // Servicing events
    //
    
public:
    
    // Services an event in the SER slot
    void serviceSerEvent();
    
    // Schedules the next event in the SER slot
    void scheduleNextEvent();    
};
