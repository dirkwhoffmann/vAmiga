// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Amiga.h"
#include <map>

using std::map;
using std::vector;

struct SyntaxError : public std::runtime_error {
    using runtime_error::runtime_error;
};

void process(const void *listener, long type, u32 data1, u32 data2);

class Headless {

    // Command line options
    vector <string> inputs;
    
    // Indicates if additional output should be generated
    bool verbose = false;
    
    // The emulator instance
    Amiga amiga;

public:
        
    // Main entry point
    void main(int argc, char *argv[]);

    // Processes an incoming message
    void process(long type, u32 data1, u32 data2);

private:
    
    // Main exection function
    void run(map<string,string> &keys);
    
    // Helper methods for parsing command line arguments
    void parseArguments(int argc, char *argv[], map<string,string> &keys);
    void checkArguments(map<string,string> &keys);
};
