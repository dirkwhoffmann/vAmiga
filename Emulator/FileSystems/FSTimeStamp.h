// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_TIME_STAMP_H
#define _FS_TIME_STAMP_H

#include "Utils.h"

class FSTimeStamp {
        
    // Days since Jan 1, 1978
    u32 days;

    // Minutes since midnight
    u32 mins;

    // Ticks past minute @ 50Hz
    u32 ticks;
    
public:

    FSTimeStamp(time_t date);
    FSTimeStamp() { FSTimeStamp(time(NULL)); }
    
    void write(u8 *ptr);
};

#endif
