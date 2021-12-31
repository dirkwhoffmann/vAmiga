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

bool
FFmpeg::available()
{
#ifdef _WIN32
    
    return false;
    
#else
    
    return util::getSizeOfFile(ffmpegPath()) > 0;
    
#endif
}

bool
FFmpeg::launch(const string &args)
{
#ifdef _WIN32
    
    return false;
    
#else
    
    auto cmd = ffmpegPath() + " " + args;
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
