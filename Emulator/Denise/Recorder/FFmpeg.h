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
    
public:
    
#ifdef _MSC_VER

#else
    
    FILE *handle = nullptr;

#endif

    // Path to the FFmpeg executable
    static string ffmpegPath() { return "/usr/local/bin/ffmpeg"; }

    // Checks whether FFmeg is available
    static bool available();
    
    // Launches the FFmpeg instance
    bool launch(const string &args);
    
    // Returns true if the FFmpeg instance is currently running
    bool isRunning();
    
    // Waits until the FFmpeg instance has terminated
    void join();
};
