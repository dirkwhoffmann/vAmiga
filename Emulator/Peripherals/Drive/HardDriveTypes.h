// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"

#ifdef __cplusplus
#include "DriveDescriptors.h"
#include <vector>
#endif

//
// Enumerations
//

enum_long(HDR_TYPE)
{
    HDR_GENERIC
};
typedef HDR_TYPE HardDriveType;

#ifdef __cplusplus
struct HardDriveTypeEnum : util::Reflection<HardDriveTypeEnum, HardDriveType>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDR_GENERIC;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "HDR"; }
    static const char *key(HardDriveType value)
    {
        switch (value) {
                
            case HDR_GENERIC:   return "GENERIC";
        }
        return "???";
    }
};
#endif

enum_long(HDR_STATE)
{
    HDR_STATE_IDLE,
    HDR_STATE_READING,
    HDR_STATE_WRITING
};
typedef HDR_STATE HardDriveState;

#ifdef __cplusplus
struct HardDriveStateEnum : util::Reflection<HardDriveStateEnum, HardDriveState>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = HDR_STATE_WRITING;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }
    
    static const char *prefix() { return "HDR_STATE"; }
    static const char *key(HardDriveType value)
    {
        switch (value) {
                
            case HDR_STATE_IDLE:      return "IDLE";
            case HDR_STATE_READING:   return "READING";
            case HDR_STATE_WRITING:   return "WRITING";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    HardDriveType type;
    i16 pan;
    u8 stepVolume;
}
HardDriveConfig;

typedef struct
{
    DriveHead head;
    bool modified;
}
HardDriveInfo;
