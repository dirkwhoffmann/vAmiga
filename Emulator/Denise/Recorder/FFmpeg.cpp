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
    if (!execPath) setExecPath("");

    return *execPath;
}

void
FFmpeg::setExecPath(const string &path)
{
#ifdef _WIN32
    
    return false;
    
#else
    
    if (path == "") {

        // If an empty string is passed, assign the first default location
        execPath = findFFmpeg(0);
        
    } else {
        
        execPath = path;
    }
        
#endif
}

std::vector<const string>
FFmpeg::findFFmpeg()
{
    std::vector <const string> result;
 
    auto add = [&](const string &path) {
        if (util::getSizeOfFile(path) > 0 && !FORCE_NO_FFMPEG) {
            result.push_back(path);
        }
    };
        
    add("/usr/bin/ffmpeg");
    add("/usr/local/bin/ffmpeg");
    add("/opt/bin/ffmpeg");
    add("/opt/homebrew/bin/ffmpeg");
    
    return result;
}

optional<string>
FFmpeg::findFFmpeg(isize nr)
{
    auto paths = findFFmpeg();
    
    if (nr < (isize)paths.size()) return paths[nr];
    return { };
}

/*
bool
FFmpeg::findExec()
{
    
    return
    findExec("/usr/bin/ffmpeg") ||
    findExec("/usr/local/bin/ffmpeg") ||
    findExec("/opt/bin/ffmpeg") ||
    findExec("/opt/homebrew/bin/ffmpeg");
}

bool
FFmpeg::findExec(const string &path)
{
    assert(path != "");
    
    if (util::getSizeOfFile(path) > 0) {

        setExecPath(path);
        return true;
    }
    
    return false;
}
*/

bool
FFmpeg::available()
{
    return execPath ? util::getSizeOfFile(*execPath) > 0 : false;
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
