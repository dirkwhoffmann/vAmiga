// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FFmpeg.h"
#include "IOUtils.h"

optional<string> FFmpeg::execPath;
    
const string
FFmpeg::getExecPath()
{
    // Search for FFmpeg if path is uninitialized
    if (!execPath) findExec();

    return *execPath;
}

bool
FFmpeg::setExecPath(const string &path)
{
#ifdef _WIN32
    
    return false;
    
#else
    
    // If an empty string is given, scan some default locations
    if (path == "") return findExec();
            
    // Otherwise, check if the specified executable exists
    if (util::getSizeOfFile(path) > 0) {
        
        execPath = path;
        return true;
    
    } else {

        execPath = "";
        return false;
    }
    
#endif
}

bool
FFmpeg::findExec()
{
    return
    setExecPath("/usr/bin/ffmpeg") ||
    setExecPath("/usr/local/bin/ffmpeg") ||
    setExecPath("/opt/bin/ffmpeg") ||
    setExecPath("/opt/homebrew/bin/ffmpeg");
}

bool
FFmpeg::launch(const string &args)
{
#ifdef _WIN32
    
    return false;
    
#else
    
    auto cmd = getExecPath() + " " + args;
    handle = popen(cmd.c_str(), "w");
    return handle != nullptr;
    
#endif
}

bool
FFmpeg::isRunning()
{
#ifdef _WIN32
    
    return false;
    
#else
    
    return handle != nullptr;
    
#endif
}

void
FFmpeg::join()
{
#ifdef _WIN32
    
#else
    
    pclose(handle);
    handle = nullptr;
    
#endif
}
