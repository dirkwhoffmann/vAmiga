// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "GdbServerTypes.h"
#include "SubComponent.h"

class GdbServer : public SubComponent {
    
    // The most recently processed command string
    string latestCmd;
    
    // Indicates whether received packets should be acknowledged
    bool ackMode = true;

    
    //
    // Initializing
    //
    
public:
    
    GdbServer(Amiga& ref);
    ~GdbServer();

    // Tells the server that a new session has been started
    void startSession();

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "GdbServer"; }
    void _dump(dump::Category category, std::ostream& os) const override { };
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override { }
    
    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }

    
    //
    // Managing checksums
    //

    // Computes a checksum for a given string
    string computeChecksum(const string &s);

    // Verifies the checksum for a given string
    bool verifyChecksum(const string &s, const string &chk);

        
    //
    // Processing packets (GdbServerCmds.cpp)
    //

public:
    
    // Returns true if the provided package is in GDB format
    bool isGdbPacket(const string &packet);
    
    // Processes a packet in the format used by GDB
    string process(string package) throws;

    // Processes a checksum-free packet with the first letter stripped off
    string process(char letter, string package) throws;

private:
    
    string processCtrlC();
    
    template <char letter> string process(string arg) throws;
    template <char letter, GdbCmd cmd> string process(string arg) throws;
    
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);

    // Reads a byte from memory
    string readMemory(isize addr);
};
