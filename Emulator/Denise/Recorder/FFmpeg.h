// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"

class FFmpeg {
    
    // Path to the FFmpeg backend
    static optional<string> execPath;
    
public:
    
#ifdef _MSC_VER

#else
    FILE *handle = nullptr;
#endif

    //
    // Locating FFmpeg
    //
    
    // Getter and setter for the FFmpeg executable path
    static const string getExecPath();
    static void setExecPath(const string &path);
    
    // Searches for FFmpeg at various default locations
    static std::vector<const string> findFFmpeg();
    static optional<string> findFFmpeg(isize nr);
    
    // Tries to locate FFmpeg at various default locations
    static bool findExec();
    static bool findExec(const string &path);

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
