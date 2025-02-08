// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga.h"
#include "Wakeable.h"
#include <map>

namespace vamiga {

struct SyntaxError : public std::runtime_error {
    using runtime_error::runtime_error;
};

// The message listener
void process(const void *listener, Message msg);

class Headless : Wakeable {

    // Parsed command line arguments
    std::map<string,string> keys;

    // Return code
    int returnCode = 0;

    
    //
    // Launching
    //
    
public:

    // Main entry point
    int main(int argc, char *argv[]);

private:

    // Parses the command line arguments
    void parseArguments(int argc, char *argv[]);

    // Checks all command line arguments for conistency
    void checkArguments();

    // Runs a RetroShell script
    void runScript(const char **script);
    void runScript(const std::filesystem::path &path);

    
    //
    // Running
    //

public:
    
    // Reports size information
    void reportSize();

    // Processes an incoming message
    void process(Message msg);
};


}
