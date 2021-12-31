// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "NamedPipe.h"
#include "IOUtils.h"
#ifndef _MSC_VER
#include <unistd.h>
#endif

bool
NamedPipe::create(const string &name)
{
    this->name = name;
 
#ifdef WIN32

    return false;

#else
    
    ::unlink(name.c_str());
    return ::mkfifo(name.c_str(), 0666) != -1;
    
#endif
}

bool
NamedPipe::open()
{
#ifdef WIN32
    
    return false;
    
#else
    
    pipe = ::open(name.c_str(), O_WRONLY);
    return pipe != -1;
    
#endif
}

bool
NamedPipe::isOpen()
{
#ifdef WIN32

    return false;

#else
    
    return pipe != -1;

#endif
}

bool
NamedPipe::close()
{
#ifdef WIN32

    return false;

#else
    
    auto result = ::close(pipe);
    pipe = -1;
    return result == 0;
    
#endif
}

isize
NamedPipe::write(u8 *buffer, isize length)
{
#ifdef WIN32

    return 0;
    
#else

    assert(isOpen());
    return ::write(pipe, (void *)buffer, (size_t)length);
    
#endif
}
