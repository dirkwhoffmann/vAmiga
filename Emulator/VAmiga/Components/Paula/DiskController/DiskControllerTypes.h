// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Enumerations
//

enum class DriveDmaState
{
    OFF,     // Drive is idle
    
    WAIT,    // Drive is waiting for the sync word
    READ,    // Drive is reading
    
    WRITE,   // Drive is writing
    FLUSH,   // Drive is finishing up the write process
};

struct DriveStateEnum : Reflection<DriveStateEnum, DriveDmaState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(DriveDmaState::FLUSH);
        
    static const char *_key(DriveDmaState value)
    {
        switch (value) {
                
            case DriveDmaState::OFF:    return "OFF";
                
            case DriveDmaState::WAIT:   return "WAIT";
            case DriveDmaState::READ:   return "READ";
                
            case DriveDmaState::WRITE:  return "WRITE";
            case DriveDmaState::FLUSH:  return "FLUSH";
        }
        return "???";
    }
    static const char *help(DriveDmaState value)
    {
        switch (value) {
                
            case DriveDmaState::OFF:    return "Inactive";
                
            case DriveDmaState::WAIT:   return "Waiting";
            case DriveDmaState::READ:   return "Reading";
                
            case DriveDmaState::WRITE:  return "Writing";
            case DriveDmaState::FLUSH:  return "Flushing";
        }
        return "???";
    }
};


//
// Structures
//

typedef struct
{
    /* Acceleration factor. This value equals the number of words that get
     * transfered into memory during a single disk DMA cycle. This value must
     * be 1 to emulate a real Amiga. If it set to, e.g., 2, the drive loads
     * twice as fast. A value of -1 indicates a turbo drive. In this case,
     * the exact value of the acceleration factor has no meaning.
     */
    i32 speed;
    
    bool lockDskSync;
    bool autoDskSync;
}
DiskControllerConfig;

inline bool isValidDriveSpeed(isize speed)
{
    switch (speed) {
            
        case -1: case 1: case 2: case 4: case 8: return true;
    }
    return false;
}

typedef struct
{
    isize selectedDrive;
    DriveDmaState state;
    i32 fifo[6];
    u8 fifoCount;
    
    u16 dsklen;
    u16 dskbytr;
    u16 dsksync;
    u8 prb;
}
DiskControllerInfo;

}
