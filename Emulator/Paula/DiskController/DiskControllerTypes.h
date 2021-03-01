// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskControllerPublicTypes.h"
#include "Reflection.h"

struct DriveStateEnum : Reflection<DriveStateEnum, DriveState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= DRIVE_DMA_FLUSH;
    }

    static const char *prefix() { return "DRIVE_DMA"; }
    static const char *key(DriveState value)
    {
        switch (value) {
                
            case DRIVE_DMA_OFF:    return "OFF";

            case DRIVE_DMA_WAIT:   return "WAIT";
            case DRIVE_DMA_READ:   return "READ";

            case DRIVE_DMA_WRITE:  return "WRITE";
            case DRIVE_DMA_FLUSH:  return "FLUSH";
        }
        return "???";
    }
};
