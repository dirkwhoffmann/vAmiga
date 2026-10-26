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

struct DriveStateEnum : utl::Reflectable<DriveStateEnum, DriveDmaState>
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
    
    bool lockDskSync;                       ///< Ignore writes to DSKSYNC
    bool autoDskSync;                       ///< Always find a sync mark
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
    isize selectedDrive;                    ///< Number of the selected drive
    DriveDmaState state;                    ///< State of the disk DMA
    i32 fifo[6];                            ///< Contents of the disk FIFO
    u8 fifoCount;                           ///< Number of bytes in the FIFO
    
    u16 dsklen;                             ///< Disk DMA length register (DSKLEN)
    u16 dskbytr;                            ///< Disk data byte and status register (DSKBYTR)
    u16 dsksync;                            ///< Disk sync register (DSKSYNC)
    u8 prb;                                 ///< CIA B port register B (drive control lines)
}
DiskControllerInfo;

}
