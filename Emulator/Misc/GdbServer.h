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
    
    
    //
    // Initializing
    //
    
public:
    
    GdbServer(Amiga& ref);
    ~GdbServer();

    
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
    // Processing packets
    //

public:
    
    string process(string cmd) throws;
    
private:
    
    string process(char cmd, string arg) throws;
    template <char cmd> string process(string arg) throws;
    template <GdbCmd cmd> string processCmd(string arg) throws;

    string checksum(const string &s);
    std::vector<string> split(const string &s, char delimiter);
    
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);

    // Reads a byte from memory
    string readMemory(isize addr);
};
