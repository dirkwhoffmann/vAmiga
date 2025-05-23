// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BasicTypes.h"

namespace vamiga {

class FFmpeg {
    
public:
    
    // A list of available FFmpeg executables (setup in init() )
    static std::vector<fs::path> paths;

    // Path to the selected FFmpeg executable
    static fs::path exec;

#ifdef _MSC_VER

#else
    FILE *handle = nullptr;
#endif

    //
    // Locating FFmpeg
    //
    
    // Sets up the 'path' vector
    static void init();

    // Getter and setter for the FFmpeg executable path
    static const fs::path getExecPath();
    static void setExecPath(const fs::path &path);

    // Checks whether FFmeg is available
    static bool available();
    
    
    //
    // Running FFmpeg
    //
    
    // Launches the FFmpeg instance
    bool launch(const string &args);
    
    // Returns true if the FFmpeg instance is currently running
    bool isRunning();
    
    // Waits until the FFmpeg instance has terminated
    void join();
};

}
